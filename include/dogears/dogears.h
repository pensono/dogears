#pragma once

#include <algorithm>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <pthread.h>
#include "dogears/buffer.h"
#include "dogears/format.h"

namespace dogears {
  
// In samples, per channel
static constexpr unsigned int pru_buffer_capacity_samples = 512;
static constexpr unsigned int channels = 4;
static constexpr unsigned int buffers = 2; // double buffered
static constexpr unsigned int sample_rate_fsync = 144000;
static constexpr unsigned int sample_rate_spi = 105469;
static constexpr unsigned int sample_rate = sample_rate_spi;

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

      This should not be called from the callback itself.
     */
    void endStream();

    /**
      Begins synchronously streaming data to a callback. This function does
      not return.
     */
    template<typename format>
    void stream(std::function<void(Buffer<format>)> callback);
    
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

    std::thread streamThread;
    volatile bool continueStreaming;
};

// TODO remove duplicated logic between capture/beginStream/stream
// TODO make the templates compile faster by calling into precompiled versions

// Not to be called by users
template <typename format>
void dispatchBuffers(
    std::function<void(Buffer<format>)> callback, 
    std::shared_ptr<std::queue<Buffer<format>>> queue,    
    std::shared_ptr<std::mutex> queue_mutex,
    std::shared_ptr<std::condition_variable> ready_signal, 
    volatile bool* continueStreaming);

template<typename format>
void DogEars::beginStream(std::function<void(Buffer<format>)> callback) {
    continueStreaming = true;
    streamThread = std::thread([&] () {
        auto processing_queue = std::make_shared<std::queue<Buffer<format>>>();
        auto queue_mutex = std::make_shared<std::mutex>();
        auto ready_signal = std::make_shared<std::condition_variable>();
        std::thread processing_thread(dispatchBuffers<format>, callback, processing_queue, queue_mutex, ready_signal, &continueStreaming);

        // Make this thread realtime
        struct sched_param param = { 2 };
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

        int last_buffer_number = -1;

        volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;

        while (continueStreaming) {
            // Heap allocate the data
            auto data = std::make_shared<std::vector<std::vector<typename format::backing_type>>>(
                    channels,
                    std::vector<typename format::backing_type>(pru_buffer_capacity_samples));

            // We expect a new buffer every 3ms, so we'll wait  for at most 10ms before checking the continueStreaming variable again
            if (prussdrv_pru_wait_event_timeout(PRU_EVTOUT_0, 10 * 1000) == 0) {
                continue;
            }

            int buffer_number = *buffer_number_pru;

            readInto<format>(*data, buffer_number, 0, pru_buffer_capacity_samples);
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

        processing_thread.join();
    });
}

template<typename format>
void DogEars::stream(std::function<void(Buffer<format>)> callback) {
    auto processing_queue = std::make_shared<std::queue<Buffer<format>>>();
    auto queue_mutex = std::make_shared<std::mutex>();
    auto ready_signal = std::make_shared<std::condition_variable>();
    continueStreaming = true;
    
    streamThread = std::thread([=] () {
        // Make this thread realtime
        struct sched_param param = { 2 };
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

        int last_buffer_number = -1;

        volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;

        while (continueStreaming) {
            // Heap allocate the data
            auto data = std::make_shared<std::vector<std::vector<typename format::backing_type>>>(
                    channels,
                    std::vector<typename format::backing_type>(pru_buffer_capacity_samples));

            // We expect a new buffer every 3ms, so we'll wait  for at most 10ms before checking the continueStreaming variable again
            if (prussdrv_pru_wait_event_timeout(PRU_EVTOUT_0, 10 * 1000) == 0) {
                continue;
            }
            int buffer_number = *buffer_number_pru;

            readInto<format>(*data, buffer_number, 0, pru_buffer_capacity_samples);
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
    });

    dispatchBuffers(callback, processing_queue, queue_mutex, ready_signal, &continueStreaming);
}

template<typename format>
Buffer<format> DogEars::capture(unsigned int samples) {
    auto data = std::make_shared<std::vector<std::vector<typename format::backing_type>>>(
            channels,
            std::vector<typename format::backing_type>(samples));
    unsigned int samples_captured = 0;
    int last_buffer_number = -1;

    volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;

    // Make this thread realtime
    struct sched_param param = { 2 };
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    while (samples_captured < samples) {
        // We expect a new buffer every 3ms, so we'll wait  for at most 10ms before checking the continueStreaming variable again
        if (prussdrv_pru_wait_event_timeout(PRU_EVTOUT_0, 10 * 1000) == 0) {
            continue;
        }
        int buffer_number = *buffer_number_pru;

        int read_size = std::min(pru_buffer_capacity_samples, samples - samples_captured);
        readInto<format>(*data, buffer_number, samples_captured, read_size);
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
    int buffer_offset = (buffer_number & 1) ? pru_buffer_capacity_samples * channels : 0;
    uint32_t* buffer_start = &((uint32_t*) buffer_base)[buffer_offset];

    for (unsigned int i = 0; i < channels; i++) {
        for (unsigned int j = 0; j < samples; j++) {
            // Convert from 24bit signed
            data[i][startSample+j] = format::convert(buffer_start[j + pru_buffer_capacity_samples * i]);
        }
    }
}

template <typename format>
void dispatchBuffers(
        std::function<void(Buffer<format>)> callback,
        std::shared_ptr<std::queue<Buffer<format>>> queue,
        std::shared_ptr<std::mutex> queue_mutex,
        std::shared_ptr<std::condition_variable> ready_signal,
        volatile bool* continueStreaming) {
    std::queue<Buffer<format>> callback_queue;
    while (*continueStreaming) {
        {
            std::unique_lock<std::mutex> lock(*queue_mutex);
            if (ready_signal->wait_for(lock, std::chrono::milliseconds(10)) == std::cv_status::timeout) {
                // Timed out. check that we're still streaming and try again
                continue;
            }

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