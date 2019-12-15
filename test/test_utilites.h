#include <algorithm>
#include <iostream>
#include <string>
#include "dogears/buffer.h"

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string RESET = "\033[0m";

void printTestResults(bool success);

template<typename T>
bool assertEqual(T observed, T expected, std::string errorMessage) {
    bool value = observed == expected;

    if (!value) {
        std::cout << RED << errorMessage << RESET << std::endl;
        std::cout << "  Got: " << observed << std::endl;
        std::cout << "  Expected: " << expected << std::endl;
    }

    return value;
}

/**
 * Ensure that the given buffer is sane.
 * 1. Not all entries are zero. There should be some noise
 * 2. All entries are within the acceptable range (ex: -1 to 1)
 */
template<typename format>
bool checkBuffer(const dogears::Buffer<format>& buffer, std::string errorMessagePrefix) {
    auto check_passed = true;

    for (unsigned int i = 0; i < buffer.channels(); i++) {
        auto channel_data = buffer.channel(i);

        auto all_zeros = std::all_of(channel_data.begin(), channel_data.end(), [](auto element){ 
            return element == format::zero; 
        });
        auto within_range = std::all_of(channel_data.begin(), channel_data.end(), [](auto element){
            return element >= format::min && element <= format::max;
        });

        if (all_zeros) {
            std::cout << RED << errorMessagePrefix << " Channel " << i << ": Buffer is entirely zeroes" << RESET << std::endl;
            check_passed = false;
        }

        if (!within_range) {
            std::cout << RED << errorMessagePrefix << " Channel " << i << ": Buffer contains items which are not within the format's range" << RESET << std::endl;
            std::cout << "Item values:" << std::endl;
            for (auto sample : channel_data) {
                if (sample < format::min || sample > format::max) {
                    std::cout << "  " << sample << std::endl;
                }
            }
            check_passed = false;
        }
    }

    return check_passed;
}