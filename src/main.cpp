#include <functional>
#include <iostream>
#include "memory/arena.h"
#include "concurrency/RingBuffer.h"
#include <thread>
#include <vector>
#include <atomic>

std::atomic<std::size_t> counter{0};
RingBuffer<int> queue(1024);

void producer() {
    for (int i = 0; i < 1000000; i++) {
        while (!queue.push(i)) {
            // busy wait
        }
    }
};

void consumer() {
    for (int i = 0; i < 1000000; i++) {
        int item;// the item we will be popping out and the pop function still returns true or false if the value was popped or not
        while (!queue.pop(item)) {
            // busy wait;
        }
        ++counter;
    }
}

int main() {
    const auto timer = std::chrono::high_resolution_clock::now();

    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);
    producer_thread.join();
    consumer_thread.join();
    std::cout << counter.load() << std::endl;

    const auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - timer).count() << " ms" << std::endl;



}
