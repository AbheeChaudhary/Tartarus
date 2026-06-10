cmake_minimum_required(VERSION 4.2)
project(Tartarus)

set(CMAKE_CXX_STANDARD 26)

add_executable(Tartarus src/main.cpp
        src/memory/arena.cpp
        src/memory/arena.h
        src/concurrency/RingBuffer.h
        src/engine/OrderBook.h
        src/engine/OrderBook.cpp
)
