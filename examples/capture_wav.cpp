#include <iostream>
#include <fstream>
#include <numeric>
#include <cstdint>
#include <arpa/inet.h>
#include "adc_cape/cape.h"
#include "adc_cape/buffer.h"

void write_wav(std::ofstream& f, const adc::Buffer<adc::SignedInt>& data);
void write_int16(std::ofstream& f, uint16_t data);
void write_int24(std::ofstream& f, uint32_t data);
void write_int32(std::ofstream& f, uint32_t data);

int main(int argc, char* argv[]) {
    adc::Cape cape;
    
    if (argc != 3) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " samples output_file" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  " << argv[0] << " 144000 output.wav" << std::endl;
        exit(1);
    }
    
    unsigned int samples = std::stoi(argv[1]);
    
    std::cout << "Capturing..." << std::endl;
    adc::Buffer<adc::SignedInt> data = cape.capture<adc::SignedInt>(samples);
    std::ofstream output (argv[2], std::ios::binary);
    
    std::cout << "Writing to " << argv[2] << std::endl;
    write_wav(output, data);
    
    exit(0);
}

void write_wav(std::ofstream& f, const adc::Buffer<adc::SignedInt>& data) {
    // http://www.cplusplus.com/forum/beginner/166954/
    // http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html (PCM Data section)
    
    unsigned int wave_header_size = 12;
    unsigned int fmt_chunk_size = 16;
    unsigned int sample_size_bytes = 24 / 8;
    unsigned int payload_size = sample_size_bytes * data.channels() * data.samples();
    unsigned int riff_size = payload_size + fmt_chunk_size + wave_header_size;
    
    f << "RIFF";
    write_int32(f, riff_size);
    f << "WAVE";
    f << "fmt ";
    write_int32(f, fmt_chunk_size); // fmt chunk size
    write_int16(f, 0x0001); // Integer PCM
    write_int16(f, data.channels());
    write_int32(f, adc::sample_rate);
    write_int32(f, adc::sample_rate * sample_size_bytes * data.channels()); // Data rate
    write_int16(f, sample_size_bytes * data.channels()); // Block size, one per timestep
    write_int16(f, sample_size_bytes * 8); // Sample size
    
    f << "data";
    write_int32(f, sample_size_bytes * data.channels() * data.samples());
    
    for (unsigned int sample = 0; sample < data.samples(); sample++) {
        std::vector<int32_t> sampleData = data.sample(sample);
        for (int s : sampleData) {
            write_int24(f, s);
        }
    }
}


void write_int16(std::ofstream& f, uint16_t data) {
    f.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

void write_int24(std::ofstream& f, uint32_t data) {
    f.write(reinterpret_cast<const char*>(&data), 3);
}

void write_int32(std::ofstream& f, uint32_t data) {
    f.write(reinterpret_cast<const char*>(&data), sizeof(data));
}