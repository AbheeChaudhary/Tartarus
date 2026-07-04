# Tartarus: Ultra-Low-Latency Limit Order Book Ecosystem

![C++20](https://img.shields.io/badge/Standard-C%2B%2B20-blue.svg)
![Build](https://img.shields.io/badge/Build-CMake-brightgreen.svg)
![Status](https://img.shields.io/badge/Status-V1_Baseline_Complete_V2_In_Development-orange.svg)

## Overview

Tartarus is an enterprise-grade, multi-version limit order book (LOB) ecosystem designed to systematically eliminate latency bottlenecks through documented architectural evolution.

**V1.0** (current baseline) establishes a correct, measurable foundation: a single-process matching engine with custom arena allocation and lock-free SPSC ingestion, achieving sub-10-microsecond matching latency on ARM64.

**V2.0** (in development) replaces V1's allocate-only arena with a fixed-size pool allocator enabling order cancellation/modification, introduces measurable per-order latency instrumentation, and hardenes CPU core isolation.

Versions 3–6 extend into kernel-bypass networking, distributed persistence, cross-process IPC, and adversarial ML testing — each version explicitly addressing bottlenecks discovered in the prior release.

---

## V1.0 Architecture — The Deterministic Core

### Layer 1: SPSC Lock-Free Ring Buffer (Cross-Thread Synchronization)
Orders arrive from a network ingestion thread into a Single-Producer/Single-Consumer ring buffer.

- **Cache-Line Alignment:** Head and tail pointers are padded and aligned to `alignas(64)` to eliminate false sharing between producer and consumer threads.
- **Memory Ordering:** Acquire/release semantics (`std::memory_order_acquire`, `std::memory_order_release`) enforce thread synchronization without OS-level mutexes, avoiding scheduler context-switches on the critical path.
- **Ring Wraparound (V1 Implementation):** Capacity is set to a non-power-of-two value (8193); wraparound uses modulo division (`% capacity`), incurring integer division on the hot path.

### Layer 2: Arena Allocator (Memory Management)
All Order nodes are allocated from a pre-reserved 1 GB contiguous memory pool using a pointer-bump allocator.

- **Allocation Latency:** O(1) pointer arithmetic yields ~6 nanoseconds per allocation, eliminating OS heap lock contention.
- **Node Layout:** `std::map` RB-tree nodes are allocated from the arena via `std::allocator_traits` specialization.
- **Allocation Model:** Allocate-only (no individual object reclamation); memory is returned to the OS only when the arena is destroyed.

### Layer 3: Price-Time Priority Matching (Order Execution)
The matching loop processes orders price-by-price, executing partial and full fills in strict time priority.

- **Price Level Container:** `std::map<price, std::vector<Order>>` — each vector maintains FIFO order at a price level.
- **Fill Execution:** Orders are filled from the front of the vector (oldest first). On partial fill, the vector is not modified; on full fill, `vector::erase()` is called.
- **Iterator Safety:** Partial-fill logic uses the returned iterator from `erase()` to continue iteration without double-incrementing, preventing iterator invalidation bugs.

### Layer 4: AI Code Review Pipeline
A Python/Gemini agent performs automated architectural analysis before each deployment:

- Compiles the engine and runs a 5M-order benchmark.
- Streams core C++ logic to a generative AI model role-prompted as a Principal Systems Engineer.
- Focuses analysis on pointer safety, iterator corruption, and algorithmic complexity.

---

## Evolutionary Architecture: V1 → V6 Roadmap

### V1.0 — The Deterministic Core
**Status:** ✅ Complete (baseline).

**Architecture:** Single-process execution. Arena allocator. SPSC ring buffer with acquire/release semantics.

**Identified Bottlenecks:**
1. Arena is allocate-only; cancelled/filled orders never reclaim memory, leading to unbounded logical capacity leakage under sustained order cancellation.
2. No `order_id → location` index exists; future cancel/modify operations would require O(n) scans across all price-level containers.
3. `std::vector<Order>` price-level containers invalidate iterators/indices on mid-container erase, precluding stable order handles.
4. Ring buffer uses modulo division against non-power-of-two capacity instead of bitwise masks.
5. CPU affinity is unset; threads are subject to OS-scheduler migration, causing cache-line eviction on core switches.
6. Producer spin-loop lacks `_mm_pause()` hints, wasting CPU cycles under contention.
7. No execution/fill logging exists; fills are unrecoverable post-hoc.
8. No per-order latency instrumentation; only aggregate throughput (orders/sec) is measured.

---

### V2.0 — Multicore Execution & Intrusive Memory Pools
**Status:** 🟡 In Development (Days 1–10 of 10-day sprint).

**Architecture:**
- Fixed-size pool allocator with intrusive free-list, replacing the arena.
- Explicit CPU thread-pinning (`pthread_setaffinity_np`) for matching and ingestion threads.
- Intrusive doubly-linked list for price-level containers, replacing `std::vector`.
- `order_id → PoolNode*` hash map index enabling O(1) cancel/modify operations.
- Execution/fill log with per-order latency instrumentation.
- Ring buffer with power-of-two capacity and bitwise mask wraparound.
- `_mm_pause()` instructions in producer/consumer spin-loops.

**Bottleneck Resolutions:**
- Pool allocator O(1) alloc/dealloc reclaims memory immediately on cancel/fill, closing the V1 exhaustion path.
- Stable, non-moving node addresses enable O(1) cancel/modify via direct index lookup, resolving the O(n) scan and iterator-invalidation constraints simultaneously.
- Bitwise masks eliminate integer division from the hot path.
- CPU pinning removes scheduler-driven cache eviction.
- Per-order latency instrumentation provides p50/p99/p99.9 histograms, making the "sub-10-microsecond" claim measurable rather than asserted.

**Open Constraints:**
- Pool capacity must be sized against a bounded working-set assumption; undersizing reintroduces exhaustion failure modes.
- Cross-core MESI cache-coherency traffic between pinned ingestion and matching threads is not yet profiled.
- Kernel-level core isolation (`isolcpus`, `nohz_full`, IRQ affinity steering) is documented as the next hardening step if CPU-isolated benchmarks show improvement.

**Expected V2 Improvements (measured):**
- Matching latency: p99 < 10 µs (measured via rdtsc per-order instrumentation).
- Throughput under cancel-heavy workload: sustained >1M orders/sec without memory exhaustion.
- Cancel latency: O(1), <1 µs (index lookup + unlink + free-list push).

---

### V3.0 — Custom User-Space Network Stack & Automated Feed Simulation
**Status:** 🔵 Planned (post-V2).

**Architecture:**
- User-space C++ TCP/IP stack intercepting raw Layer 2 Ethernet frames via a virtual Linux TUN/TAP interface.
- Parser for ARP and IPv4 headers, delivering frames directly into user-space memory without kernel traversal.
- Python/`scapy` packet-injection harness for reproducible synthetic order-flow simulation.
- Integration of the network stack binary into the matching core as a single executable.

**Bottleneck Resolutions:**
- Eliminates kernel network-stack traversal (interrupt handling, `sk_buff` allocation, socket-layer copies).
- Removes system call context-switch overhead from the order-ingestion path.
- Collapses redundant buffer copying (kernel socket → user app) into a single zero-copy parse step.

**Open Constraints:**
- Hand-rolled TCP state machine must be hardened against synthetic packet loss, reordering, and congestion via the `scapy` harness before it can replace the kernel's TCP implementation.

---

### V3.5 — Public-Facing Deployment & Real-Time Visualization
**Status:** 🔵 Planned (mid-V2 or post-V2, depending on interview schedule).

**Architecture:**
- Web-deployed trading simulation interface with Vite/TypeScript frontend.
- C++ `uWebSockets` server exposing the matching engine as a backend service.
- Browser-based live order-book visualization using HTML5 Canvas (not DOM reflow-heavy) and binary WebSocket frames.
- **Conflation layer:** A separate process maintains the latest-known order-book state (not a queue of all events) and flushes diffs to the frontend on a fixed tick (33–100 ms), decoupling render-update frequency from engine event frequency.
- Interactive dashboard allowing users to adjust market volatility, trigger synthetic "fat-finger" attacks (e.g., ₹1 to ₹1M order floods), and observe LOB behavior in real time.

**Bottleneck Resolutions:**
- Addresses the observability bottleneck absent from V1–V3: system correctness and behavior are now demonstrable via a human-readable, interactive interface.
- A deployed application converts an internal systems project into a stateful, clickable system suitable for demonstrating to non-systems audiences (e.g., business interviewers).

**Open Constraints:**
- Serialization contract between C++ engine and web frontend must later be superseded (not duplicated) by the `mmap`-based IPC introduced in V5.
- Dashboard request load must not perturb core matching-engine latency characteristics.

---

### V4.0 — Distributed Persistence & Backpressure
**Status:** 🔵 Planned (post-V3).

**Architecture:**
- Asynchronous, non-blocking write-ahead logging (WAL) consumed by a dedicated persistence thread.
- Trade executions flushed to a distributed PostgreSQL cluster for ACID-compliant ledger integrity.
- Periodic state snapshots written to MongoDB for rapid crash-recovery rehydration without full WAL replay.

**Bottleneck Resolutions:**
- Solves in-memory-only state loss during catastrophic failure.
- Removes blocking disk I/O latency from the matching thread's critical path by fully decoupling persistence into an independent consumer thread.

**Open Constraints:**
- A strict backpressure mechanism must be defined if WAL consumer throughput falls behind matching-engine trade-generation rate, to prevent unbounded queue growth.
- A reconciliation and idempotency strategy must be established for dual-write consistency between PostgreSQL and MongoDB under partial failure.

---

### V5.0 — Decoupled IPC & Adversarial ML Agent
**Status:** 🔵 Planned (post-V4).

**Architecture:**
- An independent `uWebSockets` edge gateway process communicating with the matching engine via POSIX shared memory (`mmap`), superseding the provisional serialization contract from V3.5.
- An adversarial Python/`libtorch` ML agent attaches to the same shared-memory region to generate real-time predictive order flow for non-linear market-stress testing.
- **Neuro-Fuzzy (ANFIS) market simulation:** Fuzzy-logic rules map order-book imbalance metrics to probability distributions over next-tick price movement, driving adaptive order-generation rates into the matching engine.

**Bottleneck Resolutions:**
- Eliminates serialization overhead at the network boundary by replacing socket-based marshaling with direct shared-memory access between gateway and engine processes.
- Extends test coverage beyond manually enumerated edge cases via generative adversarial order-flow, exercising the matching engine against stress patterns not captured by V3's synthetic feed simulator.

**Open Constraints:**
- Explicit cross-process synchronization (semaphore or atomic-flag protocol) must be implemented across the `mmap` boundary — a coordination problem structurally identical to V1's thread synchronization, now recurring at process scale.
- The ML agent's inference latency must be isolated via non-blocking reads on the engine side, ensuring a stalled or slow Python process cannot backpressure the core C++ matching engine.

---

### V6.0 — Hardware Acceleration & CPU Core Customization
**Status:** 🔵 Planned (post-V5, long-term).

**Architecture:**
- Integration of a custom RISC-V CPU core (Spartan-7 FPGA) via PCIe or UART simulation.
- Exploration of hardware-accelerated order-matching or risk-gateway logic.
- Possible instruction-set extension for market-microstructure-specific operations (e.g., price-time priority comparison, RB-tree node allocation).

---

## Bottleneck Ledger (Unresolved & In-Progress)

This ledger documents the specific architectural failures and hardware constraints discovered during each version, along with their resolution strategy.

### V1 → V2 (Resolving in V2 Development Sprint)
| ID | Bottleneck | Failure Mode | V2 Resolution | Status |
|:----|:-----------|:-------------|:--------------|:-------|
| B1 | Arena allocate-only | Cancelled/filled orders never reclaim memory; unbounded logical-capacity leakage under sustained cancellation. | Fixed-size pool allocator with intrusive free-list; O(1) dealloc. | In Progress (Day 2–3) |
| B2 | No order_id index | O(n) scan across all price-level containers required for future cancel/modify ops. | Hash map index `order_id → PoolNode*`; O(1) lookup. | In Progress (Day 5) |
| B3 | vector<Order> containers | Iterators/indices invalidate on mid-container erase; precludes stable order handles. | Intrusive doubly-linked list; O(1) removal from middle without shifting. | In Progress (Day 4) |
| B4 | Non-power-of-two ring buffer | Modulo division on hot path (~20–40 CPU cycles per wraparound). | Power-of-two capacity; bitwise mask `& (capacity - 1)` (~1 cycle). | In Progress (Day 7) |
| B5 | No CPU affinity | OS-scheduler migration causes L1/L2 cache eviction between core switches. | `pthread_setaffinity_np` for matching & ingestion threads. | In Progress (Day 7) |
| B6 | Busy-wait spin loops | Producer spin-loop burns 100% core, wastes power; hyperthread contention on sibling. | `_mm_pause()` hints; optional yield under extreme contention. | In Progress (Day 7) |
| B7 | No execution log | Fills are unrecoverable; no audit trail for post-hoc analysis. | Append-only, arena-backed fill log with `{order_id, qty, price, timestamp}` records. | In Progress (Day 8) |
| B8 | No per-order instrumentation | Only aggregate throughput measured; "sub-10-µs" claim unsubstantiated. | `rdtsc` or `std::chrono` per-order timing; p50/p99/p99.9 histogram at shutdown. | In Progress (Day 8–9) |

### V2 → V3 (Unresolved at End of V2)
| ID | Bottleneck | Failure Mode | V3 Resolution | Status |
|:----|:-----------|:-------------|:--------------|:-------|
| B9 | Pool undersizing | Fixed pool capacity assumes bounded working-set; undersizing reintroduces exhaustion. | Dynamic working-set profiling; re-sizing strategy under sustained load. | Planned |
| B10 | Cache-coherency cost unmeasured | MESI invalidation traffic between pinned threads is unbounded; may dominate latency. | CPU-isolated benchmarking (perfstat, perf); measure L1/L2 miss rates. | Planned |
| B11 | Hand-rolled TCP correctness | Custom TCP state machine lacks kernel's hardened retransmission/congestion-control logic. | Hardenening via `scapy` synthetic packet-loss/reordering injection. | Planned |

### V3 → V4 (Unresolved at End of V3)
| ID | Bottleneck | Failure Mode | V4 Resolution | Status |
|:----|:-----------|:-------------|:--------------|:-------|
| B12 | WAL consumer throughput unbounded | If persistence falls behind trade generation, queue grows without bound; memory OOM. | Strict backpressure mechanism: block matching thread if WAL queue exceeds threshold. | Planned |
| B13 | Dual-write consistency | PostgreSQL ledger and MongoDB snapshot can diverge under partial failure. | Reconciliation & idempotency strategy; eventual consistency model or distributed transaction. | Planned |

### V5+ (Long-term Hardening)
| ID | Bottleneck | Failure Mode | V5+ Resolution | Status |
|:----|:-----------|:-------------|:--------------|:-------|
| B14 | Cross-process synchronization | `mmap` IPC between gateway and engine requires explicit semaphore/atomic protocols; unrefined. | Implement & document semaphore-based signaling; add inter-process benchmarks. | Planned |
| B15 | ML agent backpressure | Slow Python inference can stall shared-memory read, impacting engine latency. | Non-blocking read pattern on engine side; separate inference & decision threads in agent. | Planned |

---

## 📊 Benchmarks

### V1.0 Baseline (ARM64, 5M mixed orders, 50% crosses)

| Component | Metric | Performance |
|:----------|:-------|:------------|
| Arena Allocator | Allocation Latency | 6 ns |
| SPSC Ring Buffer | Cross-Thread Throughput | 125M msgs/sec |
| Matching Engine | End-to-End Throughput | 121,748 trades/sec |
| Matching Engine | Per-Order Latency (aggregate) | ~8.2 µs |

**Note:** V1 aggregate latency is wall-clock time over all 5M orders. Per-order latency histogram (p50/p99/p99.9) is added in V2.

### V2.0 Expected (In Development)

| Component | Metric | Expected Performance |
|:----------|:-------|:-----|
| Pool Allocator | Allocation Latency | <6 ns (same as arena) |
| Pool Allocator | Deallocation Latency | <1 ns (list push) |
| Cancel Operation | Hash-Map Lookup + Unlink | <1 µs |
| Matching Latency (p99) | Per-Order | <10 µs (measured) |
| Throughput (cancel-heavy) | Sustained (no exhaustion) | >1M orders/sec |

---

## 🛠 Build Instructions

### Prerequisites
- C++17-compliant compiler (GCC 7+, Clang 6+, MSVC 2017+)
- CMake 3.10+
- Python 3.8+ (for AI code review pipeline)

### Build V1.0 (Baseline)

```bash
git clone https://github.com/AbheeChaudhary/Tartarus.git
cd Tartarus
mkdir build && cd build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS="-O3 -march=native" ..
make
./Tartarus
```

### Build V2.0 (In Development)

```bash
git clone https://github.com/AbheeChaudhary/Tartarus.git
cd Tartarus
git checkout v2-pool-allocator  # Switch to V2 development branch
mkdir build && cd build
cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_FLAGS="-O3 -march=native" -DENABLE_LATENCY_INSTRUMENTATION=ON ..
make
./Tartarus-v2
```

---

## 🤖 Running the AI Code Review Pipeline (V1.0)

### Setup
1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Set your Gemini API key:
   ```bash
   export GEMINI_API_KEY=your_api_key_here
   ```

### Execution
```bash
python3 automate_tests.py
```

The script will:
1. Compile the engine with `-O3` optimization.
2. Run the 5M-order benchmark.
3. Stream core C++ logic to Gemini for architectural review (pointer safety, iterator correctness, complexity analysis).
4. Print the review output and save a timestamped report.

---

## 📚 Required Reading (Canonicalized Knowledge)

### Memory & Cache Fundamentals
- **Book:** *What Every Programmer Should Know About Memory* by Ulrich Drepper  
  Read Sections 3 (CPU Caches) and 6 (Memory-Related Instructions).  
  Reference for cache-line behavior, false sharing, and the rationale behind `alignas(64)` padding.

- **Paper:** *The LMAX Disruptor: High Performance Alternative to Bounded Queues* by Martin Fowler & LMAX Architecture Team  
  Reference for mechanical sympathy, lock-free queue design, and the SPSC pattern used in Tartarus.

### C++ Concurrency & Memory Model
- **Book:** *C++ Concurrency in Action (2nd Edition)* by Anthony Williams  
  Read Chapter 5 (The C++ memory model and operations on atomic types) and Chapter 7 (Designing lock-free concurrent data structures).  
  Essential for understanding `memory_order_acquire`, `memory_order_release`, and lock-free synchronization.

### Memory Allocators & Pool Management
- **Paper:** *A Scalable Concurrent malloc(3) Implementation for FreeBSD* (jemalloc) by Jason Evans  
  Read the size-class and slab design sections.  
  Reference architecture for the fixed-size pool allocator being implemented in V2.

- **Documentation:** Boost.Intrusive — "Intrusive vs Non-Intrusive Containers"  
  Essential before implementing intrusive data structures in V2.  
  Understanding intrusive singly-linked lists and the placement of next-pointers within node structures.

### Systems Programming & Kernel Interfaces
- **Book:** *Operating System Concepts (10th Edition)* by Silberschatz, Galvin, and Gagne  
  Read Chapter 3 (Processes and IPC) and Chapter 9 (Virtual Memory).  
  Foundation for POSIX `shm_open`, `mmap`, and IPC patterns used in V5.

- **Guide:** Red Hat — *Low Latency Performance Tuning for RHEL*  
  Reference for CPU core isolation (`isolcpus`, `nohz_full`), IRQ affinity steering, and hyperthread-sibling considerations.

- **Documentation:** Linux Kernel — `isolcpus` and `nohz_full` boot parameters  
  Exact mechanisms for moving a core out of the general scheduler's pool and disabling the timer tick.

### Advanced C++ Templates & Type Traits
- **Book:** *Effective Modern C++* by Scott Meyers  
  Read Items 1 (Template type deduction), 5 (Prefer `auto`), and 10 (Prefer scoped enums).

- **Book:** *C++ Templates: The Complete Guide (2nd Edition)* by Vandevoorde, Josuttis, and Gregor  
  Read Chapter 19 (Traits and Policy Classes) and Chapter 21 (Templates and Memory Management).  
  Foundation for `std::allocator_traits` specialization and SFINAE patterns.

### Networking & Protocols
- **Book:** *High Performance Browser Networking* by Ilya Grigorik  
  Read Chapter 17 (WebSocket) for binary payload delivery semantics and subprotocols.  
  Reference for V3.5's WebSocket gateway design.

### Market Microstructure & Order-Flow Simulation
- **Concept:** Market-data "conflation" (no single canonical text)  
  Study real L2/L3 feed handlers (ITCH, OUCH protocols) and their delta-encoding mechanisms.  
  Reference: Exchange protocol documentation (NASDAQ ITCH 5.0, CME MDR).  
  Understand why a slow consumer (web dashboard) receives conflated snapshots, not a queue of every event.

- **Book:** *Pattern Recognition and Machine Learning* by Christopher M. Bishop  
  Read Chapter 1 (Probability & Decision Theory) and Chapter 3 (Linear Models for Regression).  
  Foundation for the ANFIS fuzzy-logic simulation in V5.

---

## 📋 Development Roadmap & Timeline

### V2.0 Development Sprint (10 Days)
- **Days 1–3:** Pool allocator design & implementation; intrusive free-list validation under ASan/TSan.
- **Days 4–5:** Intrusive linked-list price-level containers; order cancellation & modification; benchmark cancel latency.
- **Days 6–7:** Ring buffer power-of-two optimization, `_mm_pause()` integration, CPU affinity configuration.
- **Days 8–9:** Execution logging, per-order latency instrumentation (p50/p99/p99.9).
- **Day 10:** Integration benchmark, README update, interview narrative rehearsal.

### V3.0–V6.0 (Post-Interview, Long-Term)
- V3: Custom network stack, automated feed simulation.
- V3.5: Web-deployed simulation interface, conflation layer, interactive dashboard.
- V4: Distributed persistence (PostgreSQL + MongoDB WAL).
- V5: Cross-process IPC (`mmap`), adversarial ML agent.
- V6: Hardware acceleration (RISC-V FPGA core).

---

## 🔬 Testing & Validation

### Unit Tests (V1)
```bash
cd build
ctest --verbose
```

### Benchmark with Instrumentation (V2)
```bash
./Tartarus-v2 --benchmark --histogram --output=v2_latency_report.txt
```

### Memory Safety
```bash
# Run under AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ./Tartarus-v2

# Run under ThreadSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" ..
make && ./Tartarus-v2
```

---

## 📝 License

Tartarus is released under the MIT License. See `LICENSE` for details.

---

## 👤 Author

**Abhee Chaudhary** — IIIT Kota, ECE  
GitHub: [@AbheeChaudhary](https://github.com/AbheeChaudhary)  
Focus: Low-latency systems, high-frequency trading infrastructure, custom memory management.

---

## 🤝 Contributing

This is an active research/portfolio project. Contributions, bug reports, and architectural discussions are welcome. Please open an issue or a pull request.

---

## 📞 Contact

For technical discussions on the architecture, bottleneck analysis, or potential collaboration, reach out via GitHub Issues or email.