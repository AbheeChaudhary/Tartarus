//
// Created by Abhee Chaudhary on 23/04/26.
//

#include "arena.h"
#include <iostream>

using namespace std;

Arena::Arena(const std::size_t size) : size(size) , offset(0), ptr(std::malloc(size)){

    if (!ptr) {
        throw std::bad_alloc();
    }
}

Arena::~Arena() {
    free(ptr);
}

void* Arena::alloc(const std::size_t bytes, const std::size_t alignment) {

    if (offset + bytes > size) return nullptr;
    void* result = static_cast<char*>(ptr) + offset;
    auto remainder = reinterpret_cast<std::uintptr_t>(result) % alignment;
    if (remainder != 0) {
        auto padding = alignment - remainder;
        offset +=padding;
        result = static_cast<char*>(ptr) + offset;
    }
    if (offset + bytes > size) return nullptr;
    offset += bytes;
    return result;
}


