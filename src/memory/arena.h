//
// Created by Abhee Chaudhary on 23/04/26.
//
#pragma once
#include <cstdlib>

class Arena {
private :
    void* ptr{nullptr};
    std::size_t size{};
    std::size_t offset{};
public :
    explicit Arena(std::size_t size);
    ~Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    void* alloc(std::size_t bytes);


};




