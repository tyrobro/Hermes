#pragma once

#include <atomic>
#include <memory>

using namespace std;

template <typename T>

class LockFreeStack
{
private:
    struct Node
    {
        T data;
        Node *next;
        Node(const T &data) : data(data), next(nullptr) {}
    };

    atomic<Node *> head_{nullptr};

public:
    void push(const T &data)
    {
        Node *new_node = new Node(data);
        new_node->next = head_.load(memory_order_relaxed);
        while (!head_.compare_exchange_weak(new_node->next, new_node, memory_order_release, memory_order_relaxed))
        {
        }
    }

    bool try_pop(T &result)
    {
        Node *old_head = head_.load(memory_order_acquire);
        while (old_head && !head_.compare_exchange_weak(old_head, old_head->next, memory_order_acquire, memory_order_relaxed))
        {
        }

        if (!old_head)
            return false;

        result = move(old_head->data);
        return true;
    }
};