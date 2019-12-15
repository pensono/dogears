#include <string>
#include "dogears/dogears.h"
#include "test_utilites.h"

int main(int argc, char* argv[]) {
    dogears::DogEars cape;

    auto passed = true;
    auto bufferNumber = 0;
    cape.beginStream<dogears::Normalized>([&](auto buffer) {
        passed &= checkBuffer(buffer, "Buffer " + bufferNumber);
        passed &= assertEqual(buffer.channels(), 4u, "Incorrect number of channels in buffer " + bufferNumber);

        if (bufferNumber == 10) {
            cape.endStream();
        }
        bufferNumber++;
    });

    printTestResults(passed);
}