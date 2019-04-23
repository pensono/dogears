#pragma once

#include <vector>
#include <assert.h>

namespace adc {


// TODO Format
// Raw
// uV
// mV
// Float (-1 to 1)
  
// A datastructure which allows data from all channels to be grouped together 
// easily.
template<class T>
class Buffer {
  public:
    Buffer(std::vector<std::vector<T>> data) : data(data) {
      assert(data.size() >= 1);
      // Assert each element in data is the same size
    }
    
    /**
       Get all the data associated with a particular channel
     */
    const std::vector<T> channelData(unsigned int channel) const {
      assert(channel < channels());
      return data[channel];
    };

    /**
       Get all the data associated with a particular sample in time
     */
    const std::vector<T> sampleData(unsigned int sample) const {
      assert(sample < samples());
      
      std::vector<T> result(channels());
      for (unsigned int i = 0; i < channels(); i++) {
        // This is potentailly very slow. It might be useful to be able to say
        // if you want the data in sample or channel major layout
        result[i] = data[i][sample];
      }
      
      return result;
    };
    
    /**
       Gives the number of channels in the buffer
     */
    unsigned int channels() const {
      return data.size();
    };
    
    /**
       Gives the number of samples in each channel of the buffer
     */
    unsigned int samples() const {
      return data[0].size();
    };
    
    // Iterates over the channels
    typename std::vector<std::vector<T>>::const_iterator begin() const { return data.begin(); }
    typename std::vector<std::vector<T>>::const_iterator end() const { return data.end(); }
    
  private:
    const std::vector<std::vector<T>> data;
};

}