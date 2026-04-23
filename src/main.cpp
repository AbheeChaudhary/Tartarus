#include <iostream>
#include "memory/arena.h"


int main() {

    Arena my_arena(1024); // on stack for fast access and auto destruction

    std::cout << my_arena.alloc(10) << std::endl;
    std::cout << my_arena.alloc(10) << std::endl;
    std::cout << my_arena.alloc(10) << std::endl;

}
