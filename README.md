# Tartarus: LLOME (Low Latency Order Matching Engine)

![C++20](https://img.shields.io/badge/Standard-C%2B%2B20-blue.svg)
![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)
![Status](https://img.shields.io/badge/Status-Active_Development-orange.svg)

## Overview
Tartarus is a bare-metal, ultra-low latency order matching engine engineered entirely from scratch in C++. It is designed to bypass standard operating system bottlenecks (context switches, heap allocation overhead, mutex locks) to achieve deterministic, nanosecond-level execution required in high-frequency trading (HFT) environments.

## Core Architecture

### 1. Memory Management: Custom Arena Allocator (Completed)
Standard heap allocations (`new`, `malloc`) introduce non-deterministic OS overhead and risk memory fragmentation. Tartarus utilizes a pre-allocated memory arena to achieve zero-latency allocations during the critical matching path.
* **Deterministic Execution:** Allocates memory via O(1) pointer arithmetic.
* **Hardware Alignment:** Enforces strict byte alignment dynamically to ensure compatibility with hardware-level atomic instructions and prevent cache-line penalties.

### 2. Concurrency: Lock-Free SPSC Ring Buffer (In Development)
Traditional synchronization primitives (`std::mutex`) force threads into OS sleep states, costing thousands of nanoseconds. Tartarus implements a zero-lock concurrency model.
* **Atomic Synchronization:** Utilizes `std::atomic` with precise memory ordering semantics (`memory_order_acquire` / `memory_order_release`) for thread-safe data passing.
* **Cache-Line Optimization:** Explicit `alignas(64)` padding prevents false sharing between the producer and consumer indices across CPU cores.

### 3. Network Stack (Planned)
* Epoll-based asynchronous event loop for high-throughput connection handling.
* Custom network packet framing to bypass standard OS network stack latency where possible.

## Build Instructions
This project utilizes CMake for cross-platform compilation.

```bash
git clone [https://github.com/yourusername/Tartarus.git](https://github.com/yourusername/Tartarus.git)
cd Tartarus
mkdir build && cd build
cmake ..
make