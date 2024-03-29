#include <unistd.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/stat.h>
#include <sys/mman.h>
#include <prussdrv.h>
#include "dogears/dogears.h"
#include "dogears/format.h"

namespace dogears {

static constexpr int PRU_NUM = 0;
static constexpr int pru0_memory_start = 0x4a300000;
static constexpr int pru1_memory_start = 0x4a302000; // Unused, but included for completeness
static constexpr int pru_shared_memory_start = 0x4a310000;

// Each pair of elements are the pins for a single channel
static const std::array<std::string, channels * 2> gain_pins = {
  // ---CH0---  ---CH1----  ---CH2----  ----CH3-----
    "86", "87", "32", "36", "49", "48", "115", "117"
};

// Map refers to both (all) buffers, since the data is double buffered
static constexpr int map_size = pru_buffer_capacity_samples * channels * buffers * sizeof(uint32_t);

void writeFile(const std::string filename, const std::string contents) {
    std::ofstream out(filename);
    out << contents;
    out.close();
}

DogEars::DogEars() : gains(channels, dB_0) {
    // Setup GPIO pins/gains
    // All this business with configuring the pins over the FS can probably be replaced with a device tree overlay
    for (unsigned int i = 0; i < channels * 2; i++) {
        struct stat s;
        if (stat(("/sys/class/gpio/gpio" + gain_pins[i]).c_str(), &s) != 0) {
            writeFile("/sys/class/gpio/export", gain_pins[i]);
        }
        writeFile("/sys/class/gpio/gpio" + gain_pins[i] + "/direction", "out");
    }

    for (unsigned int i = 0; i < channels; i++) {
        writeGain(i, dB_0);
    }

    // Setup memory
    if ((memory_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        throw std::runtime_error("Failed to open memory! Are you root?");
    }

    buffer_base = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pru0_memory_start);
    buffer_number_base = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pru_shared_memory_start);
    if (buffer_base == (void *) -1 || buffer_number_base == (void *) -1) {
        throw std::runtime_error("Failed to map base address");
    }

    // Set up PRU interrupts
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    if (prussdrv_init()) {
        throw std::runtime_error("Failed to init prussdrv");
    }

    if (prussdrv_open(PRU_EVTOUT_0)){
        throw std::runtime_error("Failed to open the PRU-ICSS, have you loaded the overlay?");
    }

    if (prussdrv_pruintc_init(&pruss_intc_initdata)) {
        throw std::runtime_error("Failed to init pru interrupts");
    }

    if (int err = prussdrv_exec_program(PRU_NUM, "./bin/pru/code.bin")) {
        throw std::runtime_error("Failed to load pru firmware. Code: " + std::to_string(err));
    }
}

DogEars::~DogEars() {
    // This is a no-op if the stream has already ended
    endStream();
    
    munmap((void *) buffer_number_base, 4);
    munmap((void *) buffer_base, map_size);
    close(memory_fd);

    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit();
}

void DogEars::endStream() {
    continueStreaming = false;
    
    // streamThread may not be started if a stream operation was never called
    if (streamThread.joinable()) {
        streamThread.join();
    }
}

void DogEars::setGain(unsigned int channel, Gain gain) {
    assert(channel < channels);

    if (gains[channel] == gain) {
        return;
    }

    writeGain(channel, gain);
    gains[channel] = gain;
}

Gain DogEars::getGain(unsigned int channel) {
    assert(channel < channels);
    return gains[channel];
}

void DogEars::writeGain(unsigned int channel, Gain gain) {
    const std::array<std::string, 2> values = { "0", "1" };
    writeFile("/sys/class/gpio/gpio" + gain_pins[channel * 2] + "/value", values[gain & 1]);
    writeFile("/sys/class/gpio/gpio" + gain_pins[channel * 2 + 1] + "/value", values[(gain & 2) >> 1]);
}

}