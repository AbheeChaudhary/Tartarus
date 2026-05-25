//
// Created by Abhee Chaudhary on 13/05/26.
//
#pragma once

#include <array>
#include <atomic>
#include <cstddef>




template <typename T> class RingBuffer {
    std::Array<T> buffer;
private :
    std::size_t capacity;
    std::atomic<std::size_t> head;
    std::atomic<std::size_t> tail;
};