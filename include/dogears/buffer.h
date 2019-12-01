#pragma once

#include <memory>
#include <vector>
#include <assert.h>
#include "dogears/format.h"

namespace dogears {

// A data structure which allows data from all channels to be grouped together
// easily.
template<typename format>
class Buffer {
  public:
    Buffer(std::shared_ptr<std::vector<std::vector<typename format::backing_type>>> data) : data(data) {
      static_assert(std::is_base_of<Format, format>::value, "Invalid format specified. Use something from format.h");
      assert(data->size() >= 1);
      // Assert each element in data is the same size
    }

    Buffer(const Buffer& other) : data(other.data) { }
    
    /**
       Get all the data associated with a particular channel
     */
    const std::vector<typename format::backing_type> channel(unsigned int channel) const {
      assert(channel < channels());
      return (*data)[channel];
    };

    /**
       Get all the data associated with a particular sample in time
     */
    const std::vector<typename format::backing_type> sample(unsigned int sample) const {
      assert(sample < samples());
      
      std::vector<typename format::backing_type> result(channels());
      for (unsigned int i = 0; i < channels(); i++) {
        // This is potentially very slow. It might be useful to be able to say
        // if you want the data in sample or channel major layout
        result[i] = (*data)[i][sample];
      }
      
      return result;
    };
    
    /**
       Gives the number of channels in the buffer
     */
    unsigned int channels() const {
      return data->size();
    };
    
    /**
       Gives the number of samples in each channel of the buffer
     */
    unsigned int samples() const {
      return (*data)[0].size();
    };
    
    // Iterates over the channels
    typename std::vector<std::vector<typename format::backing_type>>::const_iterator begin() const { return data->begin(); }
    typename std::vector<std::vector<typename format::backing_type>>::const_iterator end() const { return data->end(); }
    
  private:
    // TODO use unique_ptr over shared_ptr
    const std::shared_ptr<std::vector<std::vector<typename format::backing_type>>> data;
};

}