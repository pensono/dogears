#include <iostream>
#include <numeric>
#include <vector>
#include <iomanip>      // std::setprecision
#include <unistd.h>
#include "dogears/dogears.h"
#include "dogears/buffer.h"

void averageAndPrint(dogears::Buffer<dogears::Normalized> buffer);
void averageAndPrintIndexes(dogears::Buffer<dogears::Normalized> buffer);

int main(int argc, char* argv[]) {
    dogears::DogEars cape;
    
    std::cout << "Beginning stream..." << std::endl;
    std::cout << std::fixed;
    std::cout << std::showpos << std::setprecision(6); // Consistently sized outputs

    cape.beginStream<dogears::Normalized>(averageAndPrint);
    
    // Streaming happens asynchronously
    std::cout << "Press Ctrl-C to exit" << std::endl;
    pause();

    cape.endStream();
    
    exit(0);
}

void averageAndPrint(dogears::Buffer<dogears::Normalized> buffer) {
    for (auto channelData : buffer) {
        float sum = std::accumulate(channelData.begin(), channelData.end(), 0.0f, std::plus<float>());
        std::cout << sum / channelData.size() << "\t";
    }
    std::cout << "\r"; // Writing this will cause us to overwrite the previous line in the console
}


// Same thing as averageAndPrint, but using fewer modern C++ features
void averageAndPrintIndexes(dogears::Buffer<dogears::Normalized> buffer) {
    for (unsigned int channelNum = 0; channelNum < buffer.channels(); channelNum++) {
        float sum = 0.0f;
        std::vector<float> channelData = buffer.channel(channelNum);

        for (unsigned int i = 0; i < channelData.size(); i++) {
            sum += channelData[i];
        }
        
        std::cout << sum / buffer.samples() << " ";
    }
    std::cout << "\r"; // Writing this will cause us to overwrite the previous line in the console
}