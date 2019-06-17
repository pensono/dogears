#include <string>
#include <iostream>
#include <limits>
#include "adc_cape/format.h"

using namespace adc;

template<typename T>
bool assert_eq(T observed, T expected, std::string errorMessage) {
    bool value = observed == expected;

    if (!value) {
        std::cout << errorMessage << std::endl;
        std::cout << "  Got: " << observed << std::endl;
        std::cout << "  Expected: " << expected << std::endl;
    }

    return value;
}

template <typename format>
int doTest(const std::string& name) {
    bool passed = true;
    passed &= assert_eq(format::convert(Raw::min), format::min, "Incorrect min for " + name);
    passed &= assert_eq(format::convert(Raw::zero), format::zero, "Incorrect zero for " + name);
    passed &= assert_eq(format::convert(Raw::max), format::max, "Incorrect max for " + name);

    return passed;
}

int main(int argc, char* argv[]) {
    bool passed = true;

    // Make sure if we print out float values that we use enough precision so
    // important details don't get rounded off
    std::cout.precision(std::numeric_limits<float>::max_digits10);

    passed &= doTest<Raw>("raw");
    passed &= doTest<Normalized>("normalized");
    passed &= doTest<SignedInt>("signed int");
    passed &= doTest<UnsignedInt>("unsigned int");

    if (passed) {
        std::cout << "  [PASSED]" << std::endl;
    } else {
        std::cout << "[FAILED]" << std::endl;
    }
}