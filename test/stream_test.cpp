#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include "dogears/dogears.h"
#include "test_utilites.h"

std::mutex mutex;
std::condition_variable cv;

int main(int argc, char* argv[]) {
    dogears::DogEars cape;

    auto passed = true;
    auto bufferNumber = 0;
    cape.beginStream<dogears::Normalized>([&](auto buffer) {
        passed &= checkBuffer(buffer, "Buffer " + std::to_string(bufferNumber));
        passed &= assertEqual(buffer.channels(), 4u, "Incorrect number of channels in buffer " + std::to_string(bufferNumber));

        if (bufferNumber == 10) {
            std::lock_guard<std::mutex> lock(mutex);
            cv.notify_all();
        }
        bufferNumber++;
    });

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock);
    cape.endStream();

    printTestResults(passed);
}