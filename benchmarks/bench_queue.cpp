#include <benchmark/benchmark.h>
#include <thread>
#include "MutexQueue.hpp"
#include "SPSCQueue.hpp"

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
BENCHMARK(BM_MutexQueue_Contention)->Threads(1)->Threads(2)->Threads(4)->Threads(6)->Threads(8)->Threads(10)->Threads(12)->Threads(14)->Threads(16);

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