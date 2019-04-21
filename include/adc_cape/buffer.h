#pragma once

#include <array>
#include <assert.h>

namespace adc {
  
// In samples, per channel
static constexpr int buffer_capacity = 1024;
static constexpr int channels = 4;
  
// A datastructure which allows data from all channels to be grouped together 
// easily. The user can modify the contents of a buffer directly if they wish 
// to perform in-place operations.
template<class T>
class Buffer {
  public:
    Buffer(std::array<std::array<T, buffer_capacity>, channels> data) : data(data) {}
    const std::array<T, buffer_capacity>& channelData(unsigned int channel) const {
      assert(channel < channels);
      return data[channel];
    };
    
    // Iterates over the channels
    typename std::array<std::array<T, buffer_capacity>, channels>::const_iterator begin() const { return data.begin(); }
    typename std::array<std::array<T, buffer_capacity>, channels>::const_iterator end() const { return data.end(); }
    
  private:
    const std::array<std::array<T, buffer_capacity>, channels> data;
};

}