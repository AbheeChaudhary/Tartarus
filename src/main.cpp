#include <functional>
#include <iostream>
#include "memory/arena.h"
#include <thread>
#include <vector>

int shared_counter = 0;

void increment(){
    for (int i = 0; i < 10000000;i++) shared_counter++;
}


int main() {
    const auto timer = std::chrono::high_resolution_clock::now();
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << shared_counter << std::endl;
    const auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - timer).count() << " ms" << std::endl;



}
