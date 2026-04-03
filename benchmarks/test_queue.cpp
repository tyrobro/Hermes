#include <iostream>
#include <thread>
#include <atomic>
#include <cassert>
#include <chrono>
#include <memory>
#include "SPSCOmptimisedQueue.hpp"
#include "os_utils.hpp"

using namespace hermes;

const uint64_t MESSAGES_TO_SEND = 10000000;

void producer_thread(SPSCOptimisedQueue<uint64_t, 1048576> &queue)
{
    pin_current_thread(2);
    for (uint64_t i = 1; i <= MESSAGES_TO_SEND; ++i)
    {
        while (!queue.push(i))
        {
        }
    }
}

void consumer_thread(SPSCOptimisedQueue<uint64_t, 1048576> &queue, std::atomic<bool> &success)
{
    pin_current_thread(4);
    uint64_t expected_value = 1;
    uint64_t received_value;

    while (expected_value <= MESSAGES_TO_SEND)
    {
        if (queue.pop(received_value))
        {
            if (received_value != expected_value)
            {
                std::cerr << "[-] FATAL DATA RACE: Expected " << expected_value
                          << " but received " << received_value << "\n";
                success.store(false);
                return;
            }
            expected_value++;
        }
    }
    success.store(true);
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "   Hermes Lock-Free Architecture Tests  \n";
    std::cout << "========================================\n";

    std::cout << "[Test 1] Verifying False Sharing Mitigations...\n";
    try
    {
        SPSCOptimisedQueue<uint64_t, 1048576>::verify_cache_alignment();
        std::cout << "   -> [PASS] Atomic indices are safely isolated on 64-byte boundaries.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "   -> [FAIL] " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Test 2] Blasting 10,000,000 sequenced messages to test acquire/release semantics...\n";

    auto queue = std::make_unique<SPSCOptimisedQueue<uint64_t, 1048576>>();
    std::atomic<bool> test_passed{false};

    auto start = std::chrono::high_resolution_clock::now();

    std::thread prod(producer_thread, std::ref(*queue));
    std::thread cons(consumer_thread, std::ref(*queue), std::ref(test_passed));

    prod.join();
    cons.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (test_passed.load())
    {
        std::cout << "   -> [PASS] Zero data races detected. Acquire/Release semantics are flawless.\n";
        std::cout << "   -> Executed in " << elapsed.count() << " seconds.\n";
    }
    else
    {
        std::cerr << "   -> [FAIL] Queue integrity compromised under load.\n";
        return 1;
    }

    std::cout << "========================================\n";
    return 0;
}