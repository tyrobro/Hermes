#pragma once

#include <thread>
#include <atomic>
#include <iostream>
#include "os_utils.hpp"
#include "SPSCOmptimisedQueue.hpp"
#include "order_book.hpp"

namespace hermes
{
    struct alignas(64) Tick
    {
        uint64_t timestamp;
        uint32_t instrument_id;
        double price;
        uint32_t volume;
        uint8_t side;
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

        OrderBook books[10];

        std::thread rx_thread;
        std::thread strategy_thread;
        std::thread tx_thread;

        std::atomic<bool> running{false};

        const int RX_CORE = 2;
        const int STRATEGY_CORE = 4;
        const int TX_CORE = 6;

        uint32_t imbalance_ratio = 3;

    public:
        void set_imbalance_ratio(uint32_t ratio)
        {
            if (ratio < 2)
                ratio = 2;
            imbalance_ratio = ratio;
        }

    private:
        void rx_loop()
        {
            pin_current_thread(RX_CORE);
            while (running.load(std::memory_order_relaxed))
            {
            }
        }

        void strategy_loop()
        {
            pin_current_thread(STRATEGY_CORE);
            Tick tick;

            while (running.load(std::memory_order_relaxed))
            {
                if (rx_to_strategy_queue.pop(tick))
                {
                    uint32_t price_tick = static_cast<uint32_t>(tick.price * 100);

                    books[tick.instrument_id].update_level(tick.side, price_tick, tick.volume);

                    uint32_t best_bid = books[tick.instrument_id].get_best_bid();
                    uint32_t best_ask = books[tick.instrument_id].get_best_ask();

                    uint32_t bid_vol = books[tick.instrument_id].get_volume_at_price(0, best_bid);
                    uint32_t ask_vol = books[tick.instrument_id].get_volume_at_price(1, best_ask);

                    if (bid_vol > 0 && (bid_vol * imbalance_ratio < ask_vol))
                    {
                    }
                }
            }
        }

        void tx_loop()
        {
            pin_current_thread(TX_CORE);
            std::cout << "[Tx] Thread locked to core " << get_current_core() << "\n";
            Order order;
            while (running.load(std::memory_order_relaxed))
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
            running.store(true, std::memory_order_release);
            rx_thread = std::thread(&TradingEngine::rx_loop, this);
            strategy_thread = std::thread(&TradingEngine::strategy_loop, this);
            tx_thread = std::thread(&TradingEngine::tx_loop, this);
            std::cout << "[Engine] High-Frequency Pipeline Initialized.\n";
        }

        void stop()
        {
            running.store(false, std::memory_order_release);
            if (rx_thread.joinable())
                rx_thread.join();
            if (strategy_thread.joinable())
                strategy_thread.join();
            if (tx_thread.joinable())
                tx_thread.join();
        }

        void inject_mock_tick(const Tick &t)
        {
            while (!rx_to_strategy_queue.push(t))
            {
            }
        }
    };
}