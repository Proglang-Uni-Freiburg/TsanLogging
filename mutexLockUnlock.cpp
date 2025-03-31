#include <iostream>
#include <thread>
#include <mutex>

// Global mutex for synchronization
std::mutex mtx;
int shared_value = 0; // Shared resource

// Function that increments the shared resource
void increment(int times) {
    for (int i = 0; i < times; ++i) {
        // Lock the mutex
        mtx.lock();
        // Critical section
        ++shared_value;
        // Unlock the mutex
        mtx.unlock();
    }
}

int main() {
    // Create two threads without using containers
    std::thread t1(increment, 1);
    std::thread t2(increment, 2);

    // Wait for both threads to finish
    t1.join();
    t2.join();

    // Output the final shared value
    std::cout << "Final shared_value = " << shared_value << std::endl;
    return 0;
}
