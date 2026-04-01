# Hermes Engine Benchmarks

## Baseline: MutexQueue (Release Mode)
* **Specs:** 16 Cores, 3686 MHz CPU

| Threads | Time (ns) | CPU Time (ns) | Notes |
|---------|-----------|---------------|-------|
| 1       | 17.3      | 17.2          | Baseline limit of uncontended std::mutex. |
| 2       | 35.0      | 54.9          | Immediate contention overhead observed. |
| 4       | 26.7      | 37.5          | Likely due to 2 pairs of threads sharing L1/L2 caches efficiently. |
| 8       | 48.9      | 174.0         | Heavy context switching begins. |
| 14      | 56.6      | 376.0         | Severe OS intervention. |
| 16      | 57.3      | 562.0         | Max hardware concurrency; 32x slower than baseline. |

## Optimization 1: Lock-Free SPSC Ring Buffer
* **Architecture:** Fixed-size `std::vector` with `std::atomic<size_t>` indices. Used `acquire/release` memory semantics and `alignas(64)` cache-line padding to prevent false sharing.

| Benchmark | Threads | Time (ns) | CPU Time (ns) | Iterations | Notes |
|-----------|---------|-----------|---------------|------------|-------|
| MutexQueue| 2       | 33.0      | 47.6          | 12.8M      | Baseline contention. |
| SPSCQueue | 2       | 3.61      | 6.98          | 134.2M     | Lock-free SPSC. ~6.8x faster CPU time, 10x throughput. |