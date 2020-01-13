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

// Can't be const or else it won't be included in the final binary
unsigned int py_buffer_size_samples = pru_buffer_capacity_samples * 16;
float py_sample_rate = sample_rate;

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
        std::memcpy(&buffer[i*samples_per_channel], data.channel(i).data(), samples_per_channel * sizeof(float));
    }
}

void dogears_stream(DogEars* dogears, void (*callback)(float*)) {
    // Calling into python code is expensive. To avoid this, we will group several buffers together and
    // call the callback once
    // 16 is an arbitrarily chosen value
    float python_buffer[py_buffer_size_samples * channels];
    unsigned int sample_count = 0;

    auto wrappedCallback = [&](Buffer<Normalized> buffer) {
        for (unsigned int i = 0; i < buffer.channels(); i++) {
            std::memcpy(&python_buffer[i * py_buffer_size_samples + sample_count], buffer.channel(i).data(), buffer.samples() * sizeof(float));
        }

        sample_count += buffer.samples();

        // Assume that the buffer's size is always evenly divisible by samples_per_python_buffer
        if (sample_count == py_buffer_size_samples) {
            callback(python_buffer);
            sample_count = 0;
        }
    };

    // If the callback is called on a different thread than python is running, ctypes
    // will create a new python thread for the callback. To avoid this, always
    // use stream which performs the streaming and the callback on the current thread.
    dogears->stream<Normalized>(wrappedCallback);
}

void dogears_endStream(DogEars* dogears, float* buffer, uint32_t samples_per_channel) {
    dogears->endStream();
}

Gain dogears_getGain(DogEars* dogears, unsigned int channel) {
    return dogears->getGain(channel);
}

void dogears_setGain(DogEars* dogears, unsigned int channel, Gain newGain) {
    dogears->setGain(channel, newGain);
}

}
}