#pragma once

#include <atomic>
#include <vector>
#include <cstddef>

using namespace std;

template <typename T>

class SPSCQueue
{
private:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    alignas(CACHE_LINE_SIZE) atomic<size_t> write_index_{0};
    alignas(CACHE_LINE_SIZE) atomic<size_t> read_index_{0};
    size_t capcacity_;
    vector<T> buffer_;

public:
    explicit SPSCQueue(size_t capacity) : capcacity_(capacity + 1), buffer_(capacity + 1) {}

    bool push(const T &item)
    {
        const size_t current_write = write_index_.load(memory_order_relaxed);
        const size_t next_write = (current_write + 1) % capcacity_;

        if (next_write == read_index_.load(memory_order_acquire))
        {
            return false;
        }
        buffer_[current_write] = item;
        write_index_.store(next_write, memory_order_release);
        return true;
    }

    bool try_pop(T &item)
    {
        const size_t current_read = read_index_.load(memory_order_relaxed);

        if (current_read == write_index_.load(memory_order_acquire))
        {
            return false;
        }

        item = move(buffer_[current_read]);

        read_index_.store((current_read + 1) % capcacity_, memory_order_release);
        return true;
    }
};