#pragma once
#include <functional>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "dogears/buffer.h"
#include "dogears/format.h"

namespace dogears {
  
// In samples, per channel
static constexpr unsigned int pru_buffer_capacity = 256;
static constexpr unsigned int channels = 4;
static constexpr unsigned int buffers = 2; // double buffered
static constexpr unsigned int sample_rate = 144000;

enum Gain {
    // Integer values correspond to the pin settings we feed into the analog switch
    dB_0 = 0,
    dB_10 = 1,
    dB_20 = 2,
    dB_30 = 3,
};
  
/**
   Only one data acquisition method should be used at a time.
 */
class DogEars {
  public:
    DogEars();
    virtual ~DogEars();
    
    /**
      Begins asynchronously streaming data to a callback. Only one stream is
      supported at a time.
     */
    template<typename format>
    void beginStream(std::function<void(Buffer<format>)> callback);

    /**
      Ends asynchronously streaming data to a callback.
     */
    template<typename format>
    void endStream();
    
    /**
      Synchronously captures data for the given number of samples.
     */
    template<typename format>
    Buffer<format> capture(unsigned int samples);

    /**
      Asynchronously captures data for the given number of samples.
     */
    template<typename format>
    void capture(unsigned int samples, std::function<void(Buffer<format>)> callback);
    
    /**
      Samples a single channel. This is intended to measure DC signals. Calling
      this function in rapid succession may return identical results.
    */
    template<typename format>
    typename format::backing_type sample(unsigned int channel);
    
    /**
      Samples all channels simultaneously. This is intended to measure DC
      signals.  Calling this function in rapid succession may return identical results.
    */
    template<typename format>
    std::vector<typename format::backing_type> sampleAll();

    /**
     * Calibrates each channel individually. This sets the zero point of each channel
     * to the current value being read by the ADC, averaged over some period of time.
     */
    void calibrate(); // TODO implement

    /**
     * Sets the gain of a particular channel
     */
    void setGain(unsigned int channel, Gain gain);

    /**
     * Gets the gain of a particular channel
     */
    Gain getGain(unsigned int channel);
    
  private:
    template <typename format>
    void readInto(std::vector<std::vector<typename format::backing_type>>& data,
            uint32_t buffer_number,
            unsigned int startSample,
            unsigned int samples);

    void writeGain(unsigned int channel, Gain gain);

    int memory_fd;
    volatile void* buffer_base;
    volatile void* buffer_number_base;

    std::vector<Gain> gains;
};

// Not to be called by users
template <typename format>
void dispatchBuffers(std::function<void(Buffer<format>)> callback, std::shared_ptr<std::queue<Buffer<format>>> queue, std::shared_ptr<std::mutex> queue_mutex, std::shared_ptr<std::condition_variable> ready_signal);

template<typename format>
void DogEars::beginStream(std::function<void(Buffer<format>)> callback) {
    auto processing_queue = std::make_shared<std::queue<Buffer<format>>>();
    auto queue_mutex = std::make_shared<std::mutex>();
    auto ready_signal = std::make_shared<std::condition_variable>();
    std::thread processing_thread(dispatchBuffers<format>, callback, processing_queue, queue_mutex, ready_signal);

    int last_buffer_number = -1;

    while (true) {
        // Heap allocate the data
        auto data = std::make_shared<std::vector<std::vector<typename format::backing_type>>>(
                channels,
                std::vector<typename format::backing_type>(pru_buffer_capacity));

        int buffer_number = prussdrv_pru_wait_event(PRU_EVTOUT_0);

        readInto<format>(*data, buffer_number, 0, pru_buffer_capacity);
        prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

        {
            std::lock_guard<std::mutex> lock(*queue_mutex);
            processing_queue->emplace(std::move(data));
            ready_signal->notify_one();
        }

        // Get ready for next time
#ifdef DEBUG
        if (buffer_number - last_buffer_number > 1 && last_buffer_number != -1) {
            std::cout << "Buffer dropped. Dropped buffers: " << buffer_number - last_buffer_number - 1 << std::endl;
        }
#endif
        last_buffer_number = buffer_number;
    }
}

template<typename format>
Buffer<format> DogEars::capture(unsigned int samples) {
    std::vector<std::vector<typename format::backing_type>>
        data(channels, std::vector<typename format::backing_type>(samples));
    unsigned int samples_captured = 0;
    int last_buffer_number = -1;

    volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;

    while (samples_captured < samples) {
        int buffer_number = prussdrv_pru_wait_event(PRU_EVTOUT_0);

        int read_size = std::min(pru_buffer_capacity, samples - samples_captured);
        readInto<format>(data, buffer_number, samples_captured, read_size);
        prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

        // Get ready for next time
#ifdef DEBUG
        if (buffer_number - last_buffer_number > 1 && last_buffer_number != -1) {
            std::cout << "Buffer dropped. Dropped buffers: " << buffer_number - last_buffer_number - 1 << std::endl;
        }
#endif
        last_buffer_number = buffer_number;
        samples_captured += read_size;
    }

    return Buffer<format>{ data };
}

// Not 100% sure about this abstraction...
template <typename format>
void DogEars::readInto(
                std::vector<std::vector<typename format::backing_type>>& data,
                uint32_t buffer_number,
                unsigned int startSample,
                unsigned int samples) { // Not 100% sure about this abstraction...
    // Assume that data has `channels` elements

    int buffer_offset = (buffer_number & 1) ? pru_buffer_capacity * channels : 0;
    uint32_t* buffer_start = &((uint32_t*) buffer_base)[buffer_offset];

    for (unsigned int i = 0; i < channels; i++) {
        for (unsigned int j = 0; j < samples; j++) {
            // Convert from 24bit signed
            data[i][startSample+j] = format::convert(buffer_start[j + pru_buffer_capacity * i]);
        }
    }
}

template <typename format>
void dispatchBuffers(std::function<void(Buffer<format>)> callback, std::shared_ptr<std::queue<Buffer<format>>> queue, std::shared_ptr<std::mutex> queue_mutex, std::shared_ptr<std::condition_variable> ready_signal) {
    // Thread destroyed by destructor
    std::queue<Buffer<format>> callback_queue;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(*queue_mutex);
            ready_signal->wait(lock);

            // We've got the queue. Take all items off and send them to the user
            queue->swap(callback_queue);
        }

        // Dispatch
        while (!callback_queue.empty()) {
            callback(callback_queue.front());
            callback_queue.pop();
        }
    }
}

}