#include <cmath> // Temporary- for stubs
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <iostream>
#include "adc_cape/cape.h"

namespace adc {
    
static constexpr int pru0_memory_start = 0x4a300000;
static constexpr int pru1_memory_start = 0x4a302000; // Unused, but included for completeness
static constexpr int pru_shared_memory_start = 0x4a310000;

// Map refers to both (all) buffers, since the data is double buffered
static constexpr int map_size = pru_buffer_capacity * channels * buffers;

Cape::Cape() {
    if((memory_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
    	throw std::runtime_error("Failed to open memory! Are you root?");
    }
    
    buffer_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pru0_memory_start);
    buffer_number_base = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pru_shared_memory_start);
    if (buffer_base == (void *) -1 || buffer_number_base == (void *) -1) {
    	throw std::runtime_error("Failed to map base address");
    }
}

Cape::~Cape() {
    munmap((void*)buffer_number_base, map_size);
    munmap((void*)buffer_base, 4);
    close(memory_fd);
}

template<>
void Cape::beginStream<float>(std::function<void(adc::Buffer<float>)> callback) {
    uint32_t last_buffer = *((uint32_t*)buffer_number_base);
    
    std::cout << "Waiting... " << last_buffer << std::endl;
    while (true) {
        if (last_buffer != *((uint32_t*)buffer_number_base)) {
            last_buffer = *((uint32_t*)buffer_number_base);
            // perfrom the read
            std::cout << "New buffer! " << last_buffer << std::endl;
        }
    }
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