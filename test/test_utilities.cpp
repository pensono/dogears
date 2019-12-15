#include "test_utilites.h"

#include <iostream>

void printTestResults(bool passed) {
    if (passed) {
        std::cout << GREEN << "  [PASSED]" << RESET << std::endl;
    } else {
        std::cout << RED << "  [FAILED]" << RESET << std::endl;
    }
}