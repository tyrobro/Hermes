#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>

using namespace std;
namespace hermes
{
    template <typename T, size_t Capacity>
    class SPSCOptimisedQueue
    {
        static_assert(Capacity && ((Capacity & (Capacity - 1)) == 0), "Capacity must be a power of 2");

        static constexpr size_t Mask = Capacity - 1;

        static constexpr size_t CacheLineSize = 128;

        struct alignas(CacheLineSize) ProducerState
        {
            atomic<size_t> tail{0};
            size_t head_cache{0};
        };

        struct alignas(CacheLineSize) ConsumerState
        {
            atomic<size_t> head{0};
            size_t tail_cache{0};
        };

        alignas(CacheLineSize) ProducerState producer_;
        alignas(CacheLineSize) ConsumerState consumer_;
        alignas(CacheLineSize) T buffer_[Capacity];

    public:
        SPSCOptimisedQueue() = default;
        ~SPSCOptimisedQueue() = default;

        SPSCOptimisedQueue(const SPSCOptimisedQueue &) = delete;
        SPSCOptimisedQueue &operator=(const SPSCOptimisedQueue &) = delete;
        SPSCOptimisedQueue(SPSCOptimisedQueue &&) = delete;
        SPSCOptimisedQueue &operator=(SPSCOptimisedQueue &&) = delete;

        template <typename... Args>
        bool emplace(Args &&...args)
        {
            const size_t current_tail = producer_.tail.load(memory_order_relaxed);

            if (current_tail - producer_.head_cache == Capacity)
            {
                producer_.head_cache = consumer_.head.load(memory_order_acquire);

                if (current_tail - producer_.head_cache == Capacity)
                {
                    return false;
                }
            }

            new (&buffer_[current_tail & Mask]) T(forward<Args>(args)...);
            producer_.tail.store(current_tail + 1, memory_order_release);
            return true;
        }

        bool push(const T &item)
        {
            return emplace(item);
        }

        bool pop(T &item)
        {
            const size_t current_head = consumer_.head.load(memory_order_relaxed);

            if (current_head == consumer_.tail_cache)
            {
                consumer_.tail_cache = producer_.tail.load(memory_order_acquire);
                if (current_head == consumer_.tail_cache)
                {
                    return false;
                }
            }

            item = move(buffer_[current_head & Mask]);
            consumer_.head.store(current_head + 1, memory_order_release);
            return true;
        }
    };

}