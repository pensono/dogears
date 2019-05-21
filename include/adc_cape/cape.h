#pragma once
#include <functional>
#include "adc_cape/buffer.h"

namespace adc {
  
// In samples, per channel
static constexpr int pru_buffer_capacity = 256;
static constexpr int channels = 4;
static constexpr int buffers = 2; // double buffered
static constexpr int sample_rate = 105469;
  
/**
   Only one data aquisition method should be used at a time.
 */
class Cape {
  public:
    Cape();
    virtual ~Cape();
    
    /**
      Begins asynchronously streaming data to a callback
     */
    template<typename T>
    void beginStream(std::function<void(Buffer<T>)> callback);
    
    /**
      Synchronously captures data for the given number of samples.
     */
    Buffer<int> capture(unsigned int samples);
    
    /**
      Samples a single channel. This is intended to measure DC signals. Calling
      this function quickly may return identical results.
    */
    template<typename T>
    T sample(unsigned int channel);
    
    /**
      Samples all channels simultaniously. This is intended to measure DC 
      signals.  Calling this function quickly may return identical results.
    */
    template<typename T>
    std::array<T, channels> sampleAll();
    
  private: 
    int memory_fd;
    volatile void* buffer_base;
    volatile void* buffer_number_base;
};

}