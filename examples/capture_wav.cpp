#include <iostream>
#include <fstream>
#include <numeric>
#include <cstdint>
#include <climits>
#include <arpa/inet.h>
#include "dogears/dogears.h"
#include "dogears/buffer.h"

void write_wav(std::ofstream& f, const dogears::Buffer<dogears::Raw>& data);
void write_int16(std::ofstream& f, uint16_t data);
void write_int24(std::ofstream& f, uint32_t data);
void write_int32(std::ofstream& f, uint32_t data);
void write_float32(std::ofstream& f, float data);

int main(int argc, char* argv[]) {
    dogears::DogEars cape;
    
    if (argc != 3 && argc != 3 + dogears::channels) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " samples output_file [gain gain gain gain]" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  " << argv[0] << " 105469 output.wav" << std::endl;
        std::cout << "or" << std::endl;
        std::cout << "  " << argv[0] << " 105469 output.wav 0 10 20 30" << std::endl;
        exit(1);
    }

    if (argc == 3 + dogears::channels) {
        for (unsigned int i = 0; i < dogears::channels; i++) {
            int value = std::stoi(argv[3 + i]);
            switch (value) {
                case 0: cape.setGain(i, dogears::Gain::dB_0); break;
                case 10: cape.setGain(i, dogears::Gain::dB_10); break;
                case 20: cape.setGain(i, dogears::Gain::dB_20); break;
                case 30: cape.setGain(i, dogears::Gain::dB_30); break;
                default:
                    std::cout << "Bad value for gain: " << argv[3 + i] << std::endl;
                    std::cout << "Must be 0, 10, 20, or 30" << std::endl;
                    exit(1);
            }
        };
    }
    
    unsigned int samples = std::stoi(argv[1]);
    
    std::cout << "Capturing..." << std::endl;
    dogears::Buffer<dogears::Raw> data = cape.capture<dogears::Raw>(samples);
    std::ofstream output (argv[2], std::ios::binary);

    std::cout << "Writing to " << argv[2] << std::endl;
    write_wav(output, data);

    std::cout << "Done" << std::endl;
    exit(0);
}

void write_wav(std::ofstream& f, const dogears::Buffer<dogears::Raw>& data) {
    // http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html (PCM Data section)

#if CHAR_BIT != 8
#error "Bytes are not 8 bits"
#endif

    unsigned int wave_header_size = 12;
    unsigned int fmt_chunk_size = 16;
    unsigned int sample_size_bytes = 24 / CHAR_BIT;
    unsigned int payload_size = sample_size_bytes * data.channels() * data.samples();
    unsigned int riff_size = payload_size + fmt_chunk_size + wave_header_size;
    
    f << "RIFF";
    write_int32(f, riff_size);
    f << "WAVE";
    f << "fmt ";
    write_int32(f, fmt_chunk_size); // fmt chunk size
    write_int16(f, 0x0001); // Integer PCM
    write_int16(f, data.channels());
    write_int32(f, dogears::sample_rate);
    write_int32(f, dogears::sample_rate * sample_size_bytes * data.channels()); // Data rate
    write_int16(f, sample_size_bytes * data.channels()); // Block size, one per timestep
    write_int16(f, sample_size_bytes * CHAR_BIT); // Sample size
    
    f << "data";
    write_int32(f, sample_size_bytes * data.channels() * data.samples());
    
    for (unsigned int sample = 0; sample < data.samples(); sample++) {
        std::vector<uint32_t> sampleData = data.sample(sample);
        for (uint32_t s : sampleData) {
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