#pragma once

#include <atomic>
#include <cstddef>
#include <array>
#include <stdexcept>

namespace hermes
{

    template <typename T, size_t Capacity>
    class SPSCOptimisedQueue
    {
        static_assert(Capacity && ((Capacity & (Capacity - 1)) == 0), "Capacity must be a power of 2");

    private:
        std::array<T, Capacity> buffer;

        alignas(64) std::atomic<size_t> write_idx{0};
        alignas(64) std::atomic<size_t> read_idx{0};

        alignas(64) size_t cached_read_idx{0};
        alignas(64) size_t cached_write_idx{0};

        static constexpr size_t MASK = Capacity - 1;

    public:
        SPSCOptimisedQueue() = default;

        bool push(const T &item)
        {
            const size_t current_write = write_idx.load(std::memory_order_relaxed);

            if (current_write - cached_read_idx == Capacity)
            {
                cached_read_idx = read_idx.load(std::memory_order_acquire);
                if (current_write - cached_read_idx == Capacity)
                {
                    return false;
                }
            }

            buffer[current_write & MASK] = item;

            write_idx.store(current_write + 1, std::memory_order_release);
            return true;
        }

        bool pop(T &item)
        {
            const size_t current_read = read_idx.load(std::memory_order_relaxed);

            if (current_read == cached_write_idx)
            {
                cached_write_idx = write_idx.load(std::memory_order_acquire);
                if (current_read == cached_write_idx)
                {
                    return false;
                }
            }

            item = buffer[current_read & MASK];

            read_idx.store(current_read + 1, std::memory_order_release);
            return true;
        }

    public:
        static void verify_cache_alignment()
        {
            SPSCOptimisedQueue<T, Capacity> dummy;
            size_t write_addr = reinterpret_cast<size_t>(&dummy.write_idx);
            size_t read_addr = reinterpret_cast<size_t>(&dummy.read_idx);

            size_t diff = (write_addr > read_addr) ? (write_addr - read_addr) : (read_addr - write_addr);

            if (diff < 64)
            {
                throw std::runtime_error("FATAL: False Sharing detected! Atomics share a cache line.");
            }
        }
    };

}