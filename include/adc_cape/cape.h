#pragma once
#include <functional>
#include "buffer.h"

// In samples
#define BUFFER_SIZE 1024

namespace adc {
  
/**
   Only one data aquisition method should be used at a time.
 */
class Cape {
  public:
    Cape();
    virtual ~Cape();
    
    /**
      Begins streaming data to a callback
     */
    template<typename T>
    void beginStream(std::function<void(Buffer<T>)> callback);
    
    /**
      Begins streaming data to a callback
     */
    template<typename T>
    Buffer<T> capture(unsigned int samples);
    
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