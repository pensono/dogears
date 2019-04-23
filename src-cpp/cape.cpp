#include <cmath> // Temporary- for stubs
#include "adc_cape/cape.h"

namespace adc {

Cape::Cape() {
    // Stub
}

Cape::~Cape() {
    // Stub
}

template<>
void Cape::beginStream<float>(std::function<void(adc::Buffer<float>)> callback) {
    // Stub
}

template<typename T>
T Cape::sample(unsigned int samples) {
    // Stub
}


Buffer<int> Cape::capture(unsigned int samples) {
    // Temporary stub
    std::vector<std::vector<int>> data(4, std::vector<int>(samples));
    
    float frequency = 1000.0f;
    
    for (unsigned int channel = 0; channel < 4; channel++) {
        for (unsigned int sample = 0; sample < samples; sample++) {
            data[channel][sample] = 0x4FFFFF * sin(channel + 3.41459 * 2 * sample * frequency / sample_rate);
        }
    }
    
    return data;
}

}