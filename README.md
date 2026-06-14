# Tartarus: LLOME (Low Latency Order Matching Engine)

![C++20](https://img.shields.io/badge/Standard-C%2B%2B20-blue.svg)
![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)
![Status](https://img.shields.io/badge/Status-Active_Development-orange.svg)

## Overview# Tartarus: Low-Latency Limit Order Book (LOB)

Tartarus is a high-performance, multi-threaded Limit Order Book built in standard C++17. Engineered to minimize operating system interference, the system leverages a custom bump-allocator and a cache-aligned lock-free queue to achieve sub-10-microsecond end-to-end matching latency.

## ⚡ Core Architecture

The system is designed around the principle of isolated verification and zero-allocation critical paths. It consists of three foundational layers:

### 1. Concurrency Model: SPSC Ring Buffer
To isolate the matching engine from network I/O latency, Tartarus uses a Single-Producer/Single-Consumer (SPSC) Lock-Free Queue.
* **Cache Alignment:** Pointers are padded and aligned (`alignas(64)`) to the CPU cache line size, eliminating false sharing between threads.
* **Memory Barriers:** strict `std::memory_order_acquire` and `std::memory_order_release` semantics are used to bypass OS-level mutex locks.

### 2. Memory Management: Custom Arena Allocator
Dynamic heap allocations (`new`/`delete`) are fatal to high-frequency software. Tartarus overrides standard library allocators using a pre-allocated `ArenaAllocator`.
* A 1-Gigabyte contiguous memory pool is instantiated at startup.
* The `std::map` internal Red-Black tree nodes are routed through an adapter template, utilizing $O(1)$ pointer-bump arithmetic instead of querying the OS.
* Reduces allocation time to ~6 nanoseconds per allocation.

### 3. The Matching Engine
The core LOB executes strict Price-Time priority matching.
* Guaranteed safe iterator invalidation during partial fills and aggressive memory pruning when price levels are exhausted.

## 🤖 AI-Augmented CI/CD Pipeline

To ensure memory safety during iterative development, Tartarus integrates an automated AI Code Review agent via the `google-genai` Python SDK.
* Upon initiating a deployment build, a Python script compiles the engine, runs the 5-million order benchmark, and streams the core C++ logic to a Generative AI model.
* The AI is role-prompted as a Principal Systems Engineer to perform an architectural review, focusing exclusively on pointer safety, iterator corruption, and algorithmic time complexity before the build is finalized.

## 📊 Benchmarks

Tests conducted on an ARM64 architecture with a 5,000,000 order payload (mixed buys/sells crossing the spread).

| Component | Metric | Performance |
| :--- | :--- | :--- |
| **Arena Allocator** | Memory Allocation | 6 ns / allocation |
| **SPSC Queue** | Cross-Thread Throughput | 125,000,000 msgs / sec |
| **Matching Engine** | End-to-End Execution | 121,748 trades / sec (~8.2 µs / order)|

## 🛠 Build Instructions

```bash
git clone [https://github.com/yourusername/Tartarus.git](https://github.com/yourusername/Tartarus.git)
cd Tartarus
mkdir build && cd build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS="-O3" ..
make
./Tartarus
```

## 🤖 Running AI Code Review

To run the automated AI architectural review pipeline:

1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Create a `.env` file in the repository root with your Gemini API key:
   ```
   GEMINI_API_KEY=your_api_key_here
   ```

3. Execute the review:
   ```bash
   python3 automate_tests.py
   ```
   
   The script will stream the core C++ logic to an AI model for architectural analysis focusing on pointer safety, iterator invalidation, and memory efficiency.
