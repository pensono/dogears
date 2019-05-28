#pragma once
#include <functional>
#include <algorithm>
#include "adc_cape/buffer.h"
#include "adc_cape/format.h"

namespace adc {
  
// In samples, per channel
static constexpr unsigned int pru_buffer_capacity = 256;
static constexpr unsigned int channels = 4;
static constexpr unsigned int buffers = 2; // double buffered
static constexpr unsigned int sample_rate = 105469;

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
class Cape {
  public:
    Cape();
    virtual ~Cape();
    
    /**
      Begins asynchronously streaming data to a callback
     */
    template<typename format>
    void beginStream(std::function<void(Buffer<format>)> callback);
    
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
    void read_into(std::vector<std::vector<typename format::backing_type>>& data,
            uint32_t buffer_number,
            unsigned int startSample,
            unsigned int samples);

    void writeGain(unsigned int channel, Gain gain);

    int memory_fd;
    volatile void* buffer_base;
    volatile void* buffer_number_base;

    std::vector<Gain> gains;
};

template<typename format>
void Cape::beginStream(std::function<void(Buffer<format>)> callback) {
    volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;
    uint32_t last_buffer = 0;

    while (true) {
        uint32_t buffer_number = *buffer_number_pru;
        if (last_buffer != buffer_number) {
#ifdef DEBUG
            if (buffer_number != last_buffer + 1 && last_buffer != 0) {
                std::cout << "Buffer dropped. Amt: " << buffer_number - last_buffer - 1 << std::endl;
            }
#endif
            last_buffer = buffer_number;

            std::vector<std::vector<typename format::backing_type>>
                    data(channels, std::vector<typename format::backing_type>(pru_buffer_capacity));
            read_into<format>(data, buffer_number, 0, pru_buffer_capacity);

            Buffer<format> buffer { data };
            callback(buffer);
        }
    }
}

template<typename format>
Buffer<format> Cape::capture(unsigned int samples) {
    std::vector<std::vector<typename format::backing_type>>
        data(channels, std::vector<typename format::backing_type>(samples));
    unsigned int samples_captured = 0;

    volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;
    uint32_t last_buffer = 0;

    while (samples_captured < samples) {
        uint32_t buffer_number = *buffer_number_pru;
        if (last_buffer != buffer_number) {
#ifdef DEBUG
            if (buffer_number != last_buffer + 1 && last_buffer != 0) {
                std::cout << "Buffer dropped. Amt: " << buffer_number - last_buffer - 1 << std::endl;
            }
#endif
            last_buffer = buffer_number;

            int read_size = std::min(pru_buffer_capacity, samples - samples_captured);
            read_into<format>(data, buffer_number, samples_captured, read_size);

            samples_captured += read_size;
        }
    }

    return Buffer<format>{ data };
}

// Not 100% sure about this abstraction...
template <typename format>
void Cape::read_into(std::vector<std::vector<typename format::backing_type>>& data,
               uint32_t buffer_number,
               unsigned int startSample,
               unsigned int samples){ // Not 100% sure about this abstraction...
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

}