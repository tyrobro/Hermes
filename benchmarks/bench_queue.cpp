#include <benchmark/benchmark.h>
#include <thread>
#include "MutexQueue.hpp"
#include "SPSCQueue.hpp"
#include "LockFreeStack.hpp"
#include "MPMCQueue.hpp"
#include "SPSCOmptimisedQueue.hpp"

static void BM_MutexQueue_Contention(benchmark::State &state)
{
    static MutexQueue<int> queue;
    for (auto _ : state)
    {
        if (state.thread_index() % 2 == 0)
        {
            queue.push(42);
        }
        else
        {
            int item;
            queue.try_pop(item);
        }
    }
}
BENCHMARK(BM_MutexQueue_Contention)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(14)->Threads(16);

static void BM_SPSCQueue(benchmark::State &state)
{
    static SPSCQueue<int> spsc_queue(1024);
    for (auto _ : state)
    {
        if (state.thread_index() == 0)
        {
            spsc_queue.push(42);
        }
        else
        {
            int item;
            spsc_queue.try_pop(item);
        }
    }
}
BENCHMARK(BM_SPSCQueue)->Threads(2);

static void BM_LockFreeStack_Contention(benchmark::State &state)
{
    static LockFreeStack<int> lf_stack;

    for (auto _ : state)
    {
        if (state.thread_index() % 2 == 0)
        {
            lf_stack.push(42);
        }
        else
        {
            int item;
            lf_stack.try_pop(item);
        }
    }
}
BENCHMARK(BM_LockFreeStack_Contention)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(14)->Threads(16);

static void BM_MPMCQueue_Contention(benchmark::State &state)
{
    static MPMCQueue<int> mpmc_queue(1024);

    for (auto _ : state)
    {
        if (state.thread_index() % 2 == 0)
        {
            mpmc_queue.push(42);
        }
        else
        {
            int item;
            if (mpmc_queue.try_pop(item))
            {
                benchmark::DoNotOptimize(item);
            }
        }
    }
}
BENCHMARK(BM_MPMCQueue_Contention)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(14)->Threads(16);

static hermes::SPSCOptimisedQueue<int, 1048576> opt_spsc_queue;
static void BM_SPSCOptimisedQueue_Contention(benchmark::State &state)
{
    for (auto _ : state)
    {
        if (state.thread_index() == 0)
        {
            while (!opt_spsc_queue.push(42))
            {
                benchmark::DoNotOptimize(42);
            }
        }
        else
        {
            int item;
            while (!opt_spsc_queue.pop(item))
            {
                benchmark::DoNotOptimize(item);
            }
        }
    }
}
BENCHMARK(BM_SPSCOptimisedQueue_Contention)->Threads(2);