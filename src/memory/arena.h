//
// Created by Abhee Chaudhary on 23/04/26.
//
#pragma once
#include <cstdlib>
#include <new>

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


template <typename T>
class ArenaAllocator {
public :
    using value_type = T;
    Arena* arena;
    ArenaAllocator() : arena(nullptr){}
    ArenaAllocator(Arena* a) : arena(a){}

    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena(other.arena){}

    T* allocate (std::size_t n) {
        void* p =  arena->alloc(n*sizeof(T),alignof(T));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        // doesn't do as when the arena is destroyed everything is destroyed
    }

    template <typename U>
    struct rebind{ using other = ArenaAllocator<U>; };

};


// Defining / overloading operators

template <typename T, typename U>
bool operator == (const ArenaAllocator<T>& a, const ArenaAllocator<U>& b) noexcept { return a.arena == b.arena; }
template <typename T, typename U>
bool operator != (const ArenaAllocator<T>& a, const ArenaAllocator<U>& b) noexcept { return !(a == b); }




