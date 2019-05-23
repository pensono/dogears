#include <cmath> // Temporary- for stubs
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdexcept>
#include <iostream>
#include "adc_cape/cape.h"
#include "adc_cape/format.h"

namespace adc {
    
static constexpr int pru0_memory_start = 0x4a300000;
static constexpr int pru1_memory_start = 0x4a302000; // Unused, but included for completeness
static constexpr int pru_shared_memory_start = 0x4a310000;

// Map refers to both (all) buffers, since the data is double buffered
static constexpr int map_size = pru_buffer_capacity * channels * buffers * sizeof(uint32_t);

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
    munmap((void*)buffer_number_base, 4);
    munmap((void*)buffer_base, map_size);
    close(memory_fd);
}

}