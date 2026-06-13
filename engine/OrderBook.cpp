//
// Created by Abhee Chaudhary on 10/06/26.
//


#include "OrderBook.h"
#include <iostream>


void LimitOrderBook::add_order(Order order) {

    if (order.is_buy) {
        while (!ask_book.empty() && order.quantity > 0) {
            const auto best_ask_itr = ask_book.begin();
            const uint32_t best_ask_price = best_ask_itr->first;
            auto& sellers = best_ask_itr->second;
            if (best_ask_price > order.price) break; // if the prices match or that the resting sell order is higher price then we push and break out of the loop

            auto seller_itr = sellers.begin();

            while (seller_itr != sellers.end() && order.quantity > 0) {

                if (seller_itr->quantity > order.quantity) {
                    seller_itr->quantity -= order.quantity;
                    order.quantity = 0;
                }
                else {
                    order.quantity -= seller_itr->quantity;
                    seller_itr = sellers.erase(seller_itr); // update the iterator after destroying the order
                    continue;
                }
                ++seller_itr;


            }
            if (sellers.empty()) {
                ask_book.erase(best_ask_itr);
            }
        }
        if (order.quantity>0) {
            auto it = bid_book.find(order.price);
            if (it == bid_book.end()) {
                ArenaAllocator<Order> alloc = bid_book.get_allocator();
                it = bid_book.emplace(order.price, ArenaOrderVector(alloc)).first;
            }

            it->second.push_back(order);

        }
    }
    else {
        while (!bid_book.empty() && order.quantity > 0) {
            const auto best_bid_itr = bid_book.begin();
            const uint32_t best_bid_price = best_bid_itr->first;
            auto& buyers = best_bid_itr->second;
            if (best_bid_price < order.price) break; // if the prices match or that the resting sell order is higher price then we push and break out of the loop

            auto buyer_itr = buyers.begin();

            while (buyer_itr != buyers.end() && order.quantity > 0) {

                if (buyer_itr->quantity > order.quantity) {
                    buyer_itr->quantity -= order.quantity;
                    order.quantity = 0;
                }
                else {
                    order.quantity -= buyer_itr->quantity;
                    buyer_itr = buyers.erase(buyer_itr);
                    continue;
                }
                ++buyer_itr;


            }
            if (buyers.empty()) {
                bid_book.erase(best_bid_itr);
            }
        }
        if (order.quantity > 0) {
            auto it = ask_book.find(order.price);
            if (it == ask_book.end()) {
                ArenaAllocator<Order> alloc = ask_book.get_allocator();
                it = ask_book.emplace(order.price, ArenaOrderVector(alloc)).first;
            }
            it->second.push_back(order);
        }
    }

}