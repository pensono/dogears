#include <string>
#include "dogears/dogears.h"
#include "test_utilites.h"

int captureTest(dogears::DogEars& cape, unsigned int bufferSize, const std::string& name) {
    auto data = cape.capture<dogears::Normalized>(bufferSize);

    auto passed = true;
    passed &= assertEqual(data.channels(), 4u, name + ": Incorrect channel count");
    passed &= assertEqual(data.samples(), bufferSize, name + ": Incorrect sample count");

    passed &= checkBuffer(data, name);

    return passed;
}

int main(int argc, char* argv[]) {
    dogears::DogEars cape;

    auto passed = true;
    // Internalaly, the buffer size is some power of two
    passed &= captureTest(cape, 2048u, "Several complete buffers");
    passed &= captureTest(cape, 3000u, "Partial buffer");
    passed &= captureTest(cape, 10u, "Small capture size");

    printTestResults(passed);
}