//
// Created by Abhee Chaudhary on 13/05/26.
//
#pragma once

#include <array>
#include <atomic>
#include <cstddef>




template <typename T> class RingBuffer {

private:
    T* buffer;
    std::size_t capacity;
    alignas(64) std::atomic<std::size_t> head;
    alignas(64) std::atomic<std::size_t> tail;

public:
    explicit RingBuffer(const std::size_t size) {
        capacity = size+1; // 1 empty slot to diff between full buffer and empty
        head.store(0);
        tail.store(0);
        buffer = new T[capacity];
    }

    ~RingBuffer() {
        delete[] buffer;
    }

    bool push(const T& item) {

        std::size_t current_head = head.load(std::memory_order_relaxed);
        const std::size_t next_head = (current_head  + 1) % capacity; // wrap around
        if (const std::size_t current_tail = tail.load(std::memory_order_acquire); next_head == current_tail) return false; // buffer is full
        buffer[current_head] = item;
        head.store(next_head,std::memory_order_release);
        return true;

    }
    bool pop(T& item) {
        std::size_t current_tail = tail.load(std::memory_order_relaxed);
        if (const std::size_t current_head = head.load(std::memory_order_acquire); current_tail == current_head) return false;
        item = buffer[current_tail];
        const std::size_t next_tail = (current_tail + 1) % capacity;
        tail.store(next_tail,std::memory_order_release);
        return true;
    }

};