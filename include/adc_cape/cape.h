#pragma once
#include <functional>
#include "adc_cape/buffer.h"
#include "adc_cape/format.h"

namespace adc {
  
// In samples, per channel
static constexpr int pru_buffer_capacity = 256;
static constexpr int channels = 4;
static constexpr int buffers = 2; // double buffered
static constexpr int sample_rate = 105469;
  
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
      this function quickly may return identical results.
    */
    template<typename format>
    typename format::backing_type sample(unsigned int channel);
    
    /**
      Samples all channels simultaniously. This is intended to measure DC 
      signals.  Calling this function quickly may return identical results.
    */
    template<typename format>
    std::vector<typename format::backing_type> sampleAll();
    
  private: 
    int memory_fd;
    volatile void* buffer_base;
    volatile void* buffer_number_base;
};

template<typename format>
void Cape::beginStream(std::function<void(Buffer<format>)> callback) {
    volatile uint32_t* buffer_number_pru = (uint32_t*)buffer_number_base;
    uint32_t last_buffer = *buffer_number_pru;

    while (true) {
        uint32_t buffer_number = *buffer_number_pru;
        if (last_buffer != buffer_number) {
#ifdef DEBUG
            if (buffer_number != last_buffer + 1) {
            std::cout << "Buffer dropped. Amt: " << buffer_number - last_buffer - 1 << std::endl;
        }
#endif
            last_buffer = buffer_number;

            // perform the read
            std::vector<std::vector<typename format::backing_type>> data { channels };
            int buffer_offset = (buffer_number & 1) ? pru_buffer_capacity * channels : 0;
            uint32_t* buffer_start = &((uint32_t*) buffer_base)[buffer_offset];

            for (int i = 0; i < channels; i++) {
                data[i] = std::vector<typename format::backing_type>(pru_buffer_capacity);
                for (int j = 0; j < pru_buffer_capacity; j++) {
                    // Convert from 24bit signed
                    data[i][j] = format::convert(buffer_start[j + pru_buffer_capacity * i]);
                }
            }

            Buffer<format> buffer { data };
            callback(buffer);
        }
    }
}

}