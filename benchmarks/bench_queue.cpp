#include <benchmark/benchmark.h>
#include <thread>
#include "MutexQueue.hpp"
#include "SPSCQueue.hpp"
#include "LockFreeStack.hpp"
#include "MPMCQueue.hpp"

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