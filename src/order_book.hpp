#pragma once

#include <cstdint>
#include <algorithm>

namespace hermes
{

    class OrderBook
    {
    private:
        static constexpr uint32_t MAX_PRICE_LEVELS = 100000;
        uint32_t bid_volumes[MAX_PRICE_LEVELS] = {0};
        uint32_t ask_volumes[MAX_PRICE_LEVELS] = {0};
        uint32_t best_bid = 0;
        uint32_t best_ask = MAX_PRICE_LEVELS;

    public:
        OrderBook() = default;

        inline void update_level(uint8_t side, uint32_t price_tick, uint32_t volume)
        {
            if (price_tick >= MAX_PRICE_LEVELS)
                return;
            if (side == 0)
            {
                bid_volumes[price_tick] = volume;
                if (volume > 0 && price_tick > best_bid)
                {
                    best_bid = price_tick;
                }
                else if (volume == 0 && price_tick == best_bid)
                {
                    while (best_bid > 0 && bid_volumes[best_bid] == 0)
                    {
                        best_bid--;
                    }
                }
            }
            else
            {
                ask_volumes[price_tick] = volume;
                if (volume > 0 && price_tick < best_ask)
                {
                    best_ask = price_tick;
                }
                else if (volume == 0 && price_tick == best_ask)
                {
                    while (best_ask < MAX_PRICE_LEVELS - 1 && ask_volumes[best_ask] == 0)
                    {
                        best_ask++;
                    }
                }
            }
        }
        inline uint32_t get_best_bid() const
        {
            return best_bid;
        }
        inline uint32_t get_best_ask() const
        {
            return best_ask;
        }
    };
}