cmake_minimum_required(VERSION 4.2)
project(Tartarus)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force Maximum Optimization (-O3)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Include the memory arena implementation
add_executable(Tartarus
        src/main.cpp
        engine/OrderBook.cpp
        src/memory/arena.cpp
)