#include <iostream>
#include <thread>

void print_message(int id) {
    std::cout << "Hello from thread " << id << std::endl;
}

int main() {
    std::thread t1(print_message, 1);
    std::thread t2(print_message, 2);

    t1.join();
    t2.join();

    return 0;
}

