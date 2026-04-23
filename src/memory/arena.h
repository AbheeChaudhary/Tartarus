//
// Created by Abhee Chaudhary on 23/04/26.
//
#pragma once
#include <cstdlib>

class Arena {
private :
    std::size_t size{};
    std::size_t offset{};
    void* ptr{nullptr};
public :
    explicit Arena(std::size_t size);
    ~Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    void* alloc(std::size_t bytes, std::size_t alignment);


};




