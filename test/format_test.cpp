#include <string>
#include <iostream>
#include <limits>
#include "dogears/format.h"
#include "test_utilites.h"

using namespace dogears;

template <typename format>
int doTest(const std::string& name) {
    bool passed = true;
    passed &= assertEqual(format::convert(Raw::min), format::min, "Incorrect min for " + name);
    passed &= assertEqual(format::convert(Raw::zero), format::zero, "Incorrect zero for " + name);
    passed &= assertEqual(format::convert(Raw::max), format::max, "Incorrect max for " + name);

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

    printTestResults(passed);
}