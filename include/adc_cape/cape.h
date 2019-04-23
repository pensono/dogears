#pragma once
#include <functional>
#include "adc_cape/buffer.h"

namespace adc {
  
// In samples, per channel
static constexpr int pru_buffer_capacity = 1024;
static constexpr int channels = 4;
static constexpr int sample_rate = 144000; // I think this is the right sample rate, just a guess for now.
  
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
};

}