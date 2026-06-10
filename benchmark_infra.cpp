//
// Created by Abhee Chaudhary on 10/06/26.
//


#include <iostream>
#include <chrono>
#include <vector>
#include "engine/OrderBook.h"
#include "src/concurrency/RingBuffer.h"
#include "src/memory/arena.h"
#include <thread>


void benchmark_memory() {
    const int ALLOCATIONS = 1000000;
    std::cout << " --- MEMORY BENCHMARK (" << ALLOCATIONS << " allocations) ---\n";
    auto start_heap = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ALLOCATIONS; i++) {
        Order* o = new Order{static_cast<uint64_t>(i), 100, 50, true};
        delete o;
    }

    auto end_heap = std::chrono::high_resolution_clock::now();
    std::cout << "Heap Allocation Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_heap - start_heap).count() << " ms\n";

    Arena arena(ALLOCATIONS * sizeof(Order) * 2);
    // Pre-allocate a large arena

    auto start_arena = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ALLOCATIONS; i++) {
        void * o = arena.alloc(sizeof(Order), 64);
    }

    auto end_arena = std::chrono::high_resolution_clock::now();
    std::cout << "Arena Allocation Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_arena - start_arena).count() << " ms\n";
    std::cout << '\n';

}




void benchmark_concurrency() {

    const int MESSAGES = 5000000;
    std::cout << " --- CONCURRENCY BENCHMARK (" << MESSAGES << " messages) ---\n";

    RingBuffer<int> queue(1024); // spin around buffer

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer([&]()) {
        for (int i = 0 ; i < MESSAGES;++i) {

        }

    }

}








