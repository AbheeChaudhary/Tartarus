//
//src/main.cpp
//
// created by : Abhee Chaudhary (Gyan Veer Singh)
// Final Integration Date : 14/06/26
//
//


#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <cstdint>
#include <functional>
#include <algorithm>


#include "../engine/OrderBook.h"
#include "memory/arena.h"
#include "concurrency/RingBuffer.h"


RingBuffer<Order> message_queue(8192);
std::atomic<bool> network_active{true};

void network_thread(int num_orders) {
    for (int i = 0 ; i < num_orders;++i) {

        bool is_buy = (i%2 == 0);// every power of 2 is a buy order i.e aggressive queue
        uint32_t price = 100 + (i%10);
        uint32_t qty = 10 + (i%5);

        Order new_order{static_cast<uint64_t>(i), price, qty,is_buy};

        while (!message_queue.push(new_order)){} // spinlock wait until we can push or pop

    }

    network_active.store(false,std::memory_order_release); // this way for atomic variables


}

void engine_thread(LimitOrderBook& lob,int& processed_count) {
    Order incoming_order;

    while (network_active.load(std::memory_order_acquire)) {
        while (message_queue.pop(incoming_order)){
            lob.add_order(incoming_order);
            ++processed_count;

        }
    }

    // network down

    while (message_queue.pop(incoming_order)) {
        lob.add_order(incoming_order);
        ++processed_count;
    }

}



int main() {

    std::cout << "========================================\n";
    std::cout << "    TARTARUS: FULL SYSTEM BENCHMARK     \n" ;
    std::cout << "========================================\n" ;
    std::cout << "[*] Booting threads...\n";

    constexpr int ORDER_VOLUME = 5000000; // 5million
    LimitOrderBook lob;
    int processed_count = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::thread network(network_thread, ORDER_VOLUME);
    std::thread engine(engine_thread, std::ref(lob), std::ref(processed_count));


    network.join();
    engine.join();

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration_ms = end_time - start_time;
    double elapsed_ms = duration_ms.count();
    double elapsed_secs = elapsed_ms / 1000.0;
    double throughput = static_cast<double>(processed_count) / elapsed_secs;

    std::cout << "\n========================================" << std::endl;

    std::cout << "Total Orders Processed : " << processed_count << "\n";
    std::cout << "System Wall-Clock Time : " << elapsed_ms << " ms\n";
    std::cout << "End-to-End Throughput  : " << static_cast<uint64_t>(throughput) << " orders/sec\n";
    std::cout << "========================================" << std::endl;

    return 0;


}