#include <iostream>
#include <numeric>
#include <vector>
#include "adc_cape/cape.h"
#include "adc_cape/buffer.h"

void averageAndPrint(adc::Buffer<adc::Normalized> buffer);
void averageAndPrintIndexes(adc::Buffer<adc::Normalized> buffer);

int main(int argc, char* argv[]) {
    adc::Cape cape;
    
    std::cout << "Beginning stream..." << std::endl;

    cape.beginStream<adc::Normalized>(averageAndPrint);
    
    // Streaming happens asynchronously
    system("pause");
    exit(0);
}

void averageAndPrint(adc::Buffer<adc::Normalized> buffer) {
    for (auto channelData : buffer) {
        float sum = std::accumulate(channelData.begin(), channelData.end(), 0.0f, std::plus<float>());
        std::cout << sum / channelData.size() << "\t";
    }
    std::cout << std::endl;
}


// Same thing as averageAndPrint, but using fewer modern C++ features
void averageAndPrintIndexes(adc::Buffer<adc::Normalized> buffer) {
    for (unsigned int channelNum = 0; channelNum < buffer.channels(); channelNum++) {
        float sum = 0.0f;
        std::vector<float> channelData = buffer.channel(channelNum);

        for (unsigned int i = 0; i < channelData.size(); i++) {
            sum += channelData[i];
        }
        
        std::cout << sum / buffer.samples() << " ";
    }
    std::cout << std::endl;
}