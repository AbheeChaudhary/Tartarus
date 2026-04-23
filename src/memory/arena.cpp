//
// Created by Abhee Chaudhary on 23/04/26.
//

#include "arena.h"
#include <iostream>

using namespace std;

Arena::Arena(const std::size_t size) : ptr(std::malloc(size)) , size(size), offset(0){

    if (!ptr) {
        throw std::bad_alloc();
    }
}

Arena::~Arena() {
    free(ptr);
}

void* Arena::alloc(const std::size_t bytes) {
    if (offset + bytes > size) return nullptr;
    void* result = static_cast<char*>(ptr) + offset;
    offset += bytes;
    return result;
}


