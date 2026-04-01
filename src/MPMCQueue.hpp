#pragma once

#include <atomic>
#include <vector>
#include <cstddef>

using namespace std;

template <typename T>
class MPMCQueue
{
private:
    struct Cell
    {
        atomic<size_t> sequence;
        T data;
    };

    static constexpr size_t CACHE_LINE_SIZE = 64;

    const size_t buffer_mask_;
    Cell *const buffer_;

    alignas(CACHE_LINE_SIZE) atomic<size_t> enqueue_pos_;
    alignas(CACHE_LINE_SIZE) atomic<size_t> dequeue_pos_;

public:
    MPMCQueue(size_t buffer_size)
        : buffer_mask_(buffer_size - 1), buffer_(new Cell[buffer_size]), enqueue_pos_(0), dequeue_pos_(0)
    {
        for (size_t i = 0; i < buffer_size; i++)
        {
            buffer_[i].sequence.store(i, memory_order_relaxed);
        }
    }

    ~MPMCQueue()
    {
        delete[] buffer_;
    }

    bool push(const T &data)
    {
        Cell *cell;
        size_t pos = enqueue_pos_.load(memory_order_relaxed);

        for (;;)
        {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;

            if (dif == 0)
            {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, memory_order_relaxed))
                {
                    break;
                }
            }
            else if (dif < 0)
            {
                return false;
            }
            else
            {
                pos = enqueue_pos_.load(memory_order_relaxed);
            }
        }
        cell->data = data;
        cell->sequence.store(pos + 1, memory_order_release);
        return true;
    }

    bool try_pop(T &data)
    {
        Cell *cell;
        size_t pos = dequeue_pos_.load(memory_order_relaxed);

        for (;;)
        {
            cell = &buffer_[pos & buffer_mask_];
            size_t seq = cell->sequence.load(memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);

            if (dif == 0)
            {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                {
                    break;
                }
            }
            else if (dif < 0)
            {
                return false;
            }
            else
            {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }

        data = move(cell->data);

        cell->sequence.store(pos + buffer_mask_ + 1, memory_order_release);
        return true;
    }
};