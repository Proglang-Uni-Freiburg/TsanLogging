#include <iostream>
#include <thread>

// Each thread will have its own instance of the variable
thread_local int thread_local_variable = 0;

void increment(int id) {
    thread_local_variable += id; // No race condition here
    std::cout << "Thread " << id << ": " << thread_local_variable << std::endl;
}

int main() {
    std::thread t1(increment, 1);
    std::thread t2(increment, 2);

    t1.join();
    t2.join();

    return 0;
}

