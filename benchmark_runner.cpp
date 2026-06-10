
    #include <iostream>
    #include <chrono>
    #include <vector>
    #include "engine/OrderBook.h"

    int main() {
        LimitOrderBook lob;
        const int NUM_ORDERS = 1000000;
        
        std::cout << "[*] Pre-allocating " << NUM_ORDERS << " orders in memory..." << std::endl;
        std::vector<Order> test_payload;
        test_payload.reserve(NUM_ORDERS);
        
        // Generate a chaotic mix of buys and sells across 20 price levels
        for (int i = 0; i < NUM_ORDERS; ++i) {
            bool is_buy = (i % 2 == 0);
            uint32_t price = 100 + (i % 20); 
            uint32_t qty = 10 + (i % 5);
            test_payload.push_back({static_cast<uint64_t>(i), price, qty, is_buy});
        }
        
        std::cout << "[*] Firing payload into Tartarus Matching Engine..." << std::endl;
        
        // --- START CRITICAL PATH ---
        auto start = std::chrono::high_resolution_clock::now();
        
        for (const auto& order : test_payload) {
            lob.add_order(order);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        // --- END CRITICAL PATH ---
        
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "          TARTARUS LOB BENCHMARK        " << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Total Orders : " << NUM_ORDERS << std::endl;
        std::cout << "Total Time   : " << duration_ms << " ms" << std::endl;
        if (duration_ms > 0) {
            std::cout << "Throughput   : " << (NUM_ORDERS / duration_ms) * 1000 << " ops/sec" << std::endl;
        }
        std::cout << "Avg Latency  : " << duration_ns / NUM_ORDERS << " nanoseconds/order" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        return 0;
    }
    