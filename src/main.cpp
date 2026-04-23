#include <iostream>
#include "memory/arena.h"


int main() {

    Arena my_arena(1024); // on stack for fast access and auto destruction

    std::cout << my_arena.alloc(1,1) << std::endl;
    std::cout << my_arena.alloc(4,4) << std::endl;
    std::cout << my_arena.alloc(8,8) << std::endl;


    //
    // int x {5};// on stack
    // int* y = (int*) malloc( sizeof(int)); // on heap
    // *y = 10;


// ARENA ALLOCATOR



}
