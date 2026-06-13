//
// Created by Abhee Chaudhary on 10/06/26.
//

#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <functional>
#include "../src/memory/arena.h"


struct Order {
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    bool is_buy;




};

using ArenaOrderVector = std::vector<Order,ArenaAllocator<Order>>;

class LimitOrderBook {
private :
    Arena memory_pool{1024*1024*1024};

    std::map<uint32_t, ArenaOrderVector, std::less<uint32_t>, ArenaAllocator<std::pair<const uint32_t, ArenaOrderVector>>> ask_book; // orderprice, actual order
    std::map<uint32_t, ArenaOrderVector, std::greater<uint32_t>,ArenaAllocator<std::pair<const uint32_t,ArenaOrderVector>>> bid_book; // orderprice, actual order lists at the price
public:
    LimitOrderBook() : ask_book(std::less<uint32_t>(), ArenaAllocator<std::pair<const uint32_t, ArenaOrderVector>>(&memory_pool)),
                       bid_book(std::greater<uint32_t>(), ArenaAllocator<std::pair<const uint32_t, ArenaOrderVector>>(&memory_pool)){}

    void add_order(Order order);
};

