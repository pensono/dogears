// For debugging purposes only
#include <iostream>

#include <cstring>

#include "dogears/dogears.h"
#include "dogears/buffer.h"

// The dogears C++ library uses lots of C++ features, and therefore is unable
// to easily communicate with the ctypes library. This wrapper will allow
// each function to be called with C linkage that ctypes understands.

extern "C" {
namespace dogears {

DogEars* dogears_init() {
    auto ptr = new DogEars;
    return ptr;
}

void dogears_free(DogEars* dogears) {
    delete dogears;
}

void dogears_capture(DogEars* dogears, float* buffer, uint32_t samples_per_channel) {
    // Let python control the buffer since the python program could hold onto it indefinately
    auto data = dogears->capture<Normalized>(samples_per_channel);
    for (unsigned int i = 0; i < data.channels(); i++) {
        std::memcpy(&buffer[i*samples_per_channel], data.channel(i).data(), samples_per_channel);
    }
}

void dogears_stream(DogEars* dogears, void (*callback)(float*)) {
    auto wrappedCallback = [&](Buffer<Normalized> buffer) {
        float pythonBuffer[buffer.channels() * buffer.samples()];
        for (unsigned int i = 0; i < buffer.channels(); i++) {
            std::memcpy(&pythonBuffer[i*buffer.samples()], buffer.channel(i).data(), buffer.samples());
        }
        callback(pythonBuffer);
    };

    dogears->stream<Normalized>(wrappedCallback);
}

}
}