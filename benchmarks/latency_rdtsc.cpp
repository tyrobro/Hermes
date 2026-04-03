#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <intrin.h>
#include "SPSCOmptimisedQueue.hpp"
#include "os_utils.hpp"

using namespace hermes;

const int WARMUP_ITERATIONS = 100000;
const int BENCH_ITERATIONS = 1000000;

double get_cpu_ghz()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    uint64_t r1 = __rdtsc();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint64_t r2 = __rdtsc();
    auto t2 = std::chrono::high_resolution_clock::now();

    double elapsed_s = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.0;
    return (r2 - r1) / (elapsed_s * 1e9);
}

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
    std::cout << "   Hermes Cycle-Accurate RDTSC Bench    \n";
    std::cout << "========================================\n";

    double ghz = get_cpu_ghz();
    std::cout << "[System] Estimated CPU Frequency: " << std::fixed << std::setprecision(2) << ghz << " GHz\n";

    auto q_ping = std::make_unique<SPSCOptimisedQueue<uint64_t, 1048576>>();
    auto q_pong = std::make_unique<SPSCOptimisedQueue<uint64_t, 1048576>>();

    std::vector<uint64_t> cycle_latencies;
    cycle_latencies.reserve(BENCH_ITERATIONS);

    std::thread pong(pong_thread, std::ref(*q_ping), std::ref(*q_pong));

    pin_current_thread(2);

    std::cout << "[Bench] Warming up...\n";
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

    std::cout << "[Bench] Measuring cycles...\n";

    for (int i = 0; i < BENCH_ITERATIONS; ++i)
    {
        uint64_t start = __rdtsc();

        while (!q_ping->push(i))
        {
        }
        while (!q_pong->pop(msg_recv))
        {
        }

        uint64_t end = __rdtsc();

        cycle_latencies.push_back((end - start) / 2);
    }

    pong.join();

    std::sort(cycle_latencies.begin(), cycle_latencies.end());

    auto to_ns = [&](uint64_t cycles)
    { return (double)cycles / ghz; };

    std::cout << "\n--- Latency Percentiles (One-Way) ---\n";
    std::cout << "p50    : " << std::setw(6) << cycle_latencies[BENCH_ITERATIONS * 0.50] << " cycles (" << to_ns(cycle_latencies[BENCH_ITERATIONS * 0.50]) << " ns)\n";
    std::cout << "p99    : " << std::setw(6) << cycle_latencies[BENCH_ITERATIONS * 0.99] << " cycles (" << to_ns(cycle_latencies[BENCH_ITERATIONS * 0.99]) << " ns)\n";
    std::cout << "p99.9  : " << std::setw(6) << cycle_latencies[BENCH_ITERATIONS * 0.999] << " cycles (" << to_ns(cycle_latencies[BENCH_ITERATIONS * 0.999]) << " ns)\n";
    std::cout << "Max    : " << std::setw(6) << cycle_latencies.back() << " cycles (" << to_ns(cycle_latencies.back()) << " ns)\n";
    std::cout << "-------------------------------------\n";

    return 0;
}