//
// Created by Abhee Chaudhary on 10/06/26.
//

#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <functional>



struct Order {
    uint64_t order_id;
    uint32_t price;
    uint32_t quantity;
    bool is_buy;




};

class LimitOrderBook {
private :
    std::map<uint32_t, std::vector<Order>> ask_book; // orderprice, actual order
    std::map<uint32_t, std::vector<Order>, std::greater<uint32_t>> bid_book; // orderprice, actual order lists at the price
public:
    void add_order(Order order);

};

