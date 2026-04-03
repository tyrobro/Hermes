#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <memory>
#include <iomanip>
#include "SPSCOmptimisedQueue.hpp"
#include "os_utils.hpp"

using namespace hermes;
using namespace std::chrono;

const int WARMUP_ITERATIONS = 100000;
const int BENCH_ITERATIONS = 1000000;

void pong_thread(SPSCOptimisedQueue<uint64_t, 1048576> &q_in, SPSCOptimisedQueue<uint64_t, 1048576> &q_out)
{
    pin_current_thread(4);
    uint64_t msg;
    for (int i = 0; i < WARMUP_ITERATIONS + BENCH_ITERATIONS; ++i)
    {
        while (!q_in.pop(msg))
        {
        }
        while (!q_out.push(msg))
        {
        }
    }
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "  Hermes Nanosecond Latency Benchmark   \n";
    std::cout << "========================================\n";

    auto q_ping = std::make_unique<SPSCOptimisedQueue<uint64_t, 1048576>>();
    auto q_pong = std::make_unique<SPSCOptimisedQueue<uint64_t, 1048576>>();

    std::vector<double> latencies;
    latencies.reserve(BENCH_ITERATIONS);

    std::thread pong(pong_thread, std::ref(*q_ping), std::ref(*q_pong));

    pin_current_thread(2);

    std::cout << "[Bench] Warming up CPU caches (" << WARMUP_ITERATIONS << " iterations)...\n";
    uint64_t msg_recv;
    for (int i = 0; i < WARMUP_ITERATIONS; ++i)
    {
        while (!q_ping->push(1))
        {
        }
        while (!q_pong->pop(msg_recv))
        {
        }
    }

    std::cout << "[Bench] Measuring " << BENCH_ITERATIONS << " round-trips...\n";

    for (int i = 0; i < BENCH_ITERATIONS; ++i)
    {
        auto start = high_resolution_clock::now();

        while (!q_ping->push(i))
        {
        }
        while (!q_pong->pop(msg_recv))
        {
        }

        auto end = high_resolution_clock::now();

        double one_way_ns = duration_cast<nanoseconds>(end - start).count() / 2.0;
        latencies.push_back(one_way_ns);
    }

    pong.join();

    std::sort(latencies.begin(), latencies.end());

    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double mean = sum / BENCH_ITERATIONS;
    double p50 = latencies[BENCH_ITERATIONS * 0.50];
    double p90 = latencies[BENCH_ITERATIONS * 0.90];
    double p99 = latencies[BENCH_ITERATIONS * 0.99];
    double p99_9 = latencies[BENCH_ITERATIONS * 0.999];
    double max_lat = latencies.back();

    std::cout << "\n--- Latency Percentiles (One-Way) ---\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mean   : " << mean << " ns\n";
    std::cout << "p50    : " << p50 << " ns (Median)\n";
    std::cout << "p90    : " << p90 << " ns\n";
    std::cout << "p99    : " << p99 << " ns\n";
    std::cout << "p99.9  : " << p99_9 << " ns\n";
    std::cout << "Max    : " << max_lat << " ns\n";
    std::cout << "-------------------------------------\n";

    if (mean > 150.0)
    {
        std::cerr << "\n[-] WARNING: Latency regression detected! Mean exceeded 150ns limit.\n";
        return 1;
    }
    else
    {
        std::cout << "\n[PASS] Latencies are within acceptable micro-architecture bounds.\n";
    }

    std::cout << "========================================\n";
    return 0;
}