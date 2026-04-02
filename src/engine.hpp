#pragma once

#include <thread>
#include <atomic>
#include <iostream>
#include "os_utils.hpp"
#include "SPSCOmptimisedQueue.hpp"

using namespace std;

namespace hermes
{

    struct alignas(64) Tick
    {
        uint64_t timestamp;
        uint32_t instrument_id;
        double price;
        uint32_t volume;
    };

    struct alignas(64) Order
    {
        uint64_t timestamp;
        uint32_t instrument_id;
        double price;
        uint32_t volume;
        uint8_t side;
    };

    class TradingEngine
    {
    private:
        SPSCOptimisedQueue<Tick, 1048576> rx_to_strategy_queue;
        SPSCOptimisedQueue<Order, 1048576> strategy_to_tx_queue;

        thread rx_thread;
        thread strategy_thread;
        thread tx_thread;

        atomic<bool> running{false};

        const int RX_CORE = 2;
        const int STRATEGY_CORE = 4;
        const int TX_CORE = 6;

        void rx_loop()
        {
            pin_current_thread(RX_CORE);
            cout << "[Rx] Thread locked to core: " << get_current_core() << "\n";
            while (running.load(memory_order_relaxed))
            {
            }
        }

        void strategy_loop()
        {
            pin_current_thread(STRATEGY_CORE);
            cout << "[Strategy] Thread locked to core: " << get_current_core() << "\n";
            Tick tick;
            while (running.load(memory_order_relaxed))
            {
                if (rx_to_strategy_queue.pop(tick))
                {
                }
            }
        }

        void tx_loop()
        {
            pin_current_thread(TX_CORE);
            cout << "[Tx] Thread locked to core " << get_current_core() << "\n";
            Order order;
            while (running.load(memory_order_relaxed))
            {
                if (strategy_to_tx_queue.pop(order))
                {
                }
            }
        }

    public:
        TradingEngine() = default;
        ~TradingEngine()
        {
            stop();
        }

        void start()
        {
            running.store(true, memory_order_release);
            rx_thread = thread(&TradingEngine::rx_loop, this);
            strategy_thread = thread(&TradingEngine::strategy_loop, this);
            tx_thread = thread(&TradingEngine::tx_loop, this);
            cout << "[Engine] High-Frequency Pipeline Initialized.\n";
        }

        void stop()
        {
            running.store(false, memory_order_release);
            if (rx_thread.joinable())
                rx_thread.join();
            if (strategy_thread.joinable())
                strategy_thread.join();
            if (tx_thread.joinable())
                tx_thread.join();
            cout << "[Engine] Pipeline Shutdown Gracefully.\n";
        }
    };
}