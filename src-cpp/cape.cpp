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


template<>
float Cape::sample(unsigned int channel) {
    // Stub
    return 0.0;
}

}