#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;
template <typename T>
class MutexQueue
{
private:
    queue<T> queue_;
    mutex mutex_;
    condition_variable cv_;

public:
    void push(T item)
    {
        {
            lock_guard<mutex> lock(mutex_);
            queue_.push(move(item));
        }
        cv_.notify_one();
    }

    bool try_pop(T &item)
    {
        lock_guard<mutex> lock(mutex_);
        if (queue_.empty())
            return false;
        item = move(queue_.front());
        queue_.pop();
        return true;
    }

    void wait_and_pop(T &item)
    {
        unique_lock<mutex> lock(mutex_);
        cv_.wait(lock, [this]()
                 { return !queue_.empty; });
        item = move(queue_.front());
        queue_.pop();
    }
};