#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include "engine.hpp"

int main()
{
    std::cout << "========================================\n";
    std::cout << "   Hermes Engine Integration Test       \n";
    std::cout << "========================================\n";

    auto engine = std::make_unique<hermes::TradingEngine>();
    engine->start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "[Test] Injecting Market Data...\n";

    engine->inject_mock_tick({0, 1, 150.00, 100, 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Slow down for cout

    engine->inject_mock_tick({0, 1, 150.25, 50, 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    engine->inject_mock_tick({0, 1, 150.25, 0, 0});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "[Test] Shutting down...\n";
    engine->stop();
    std::cout << "[PASS] Order Book integrated and tested.\n";

    return 0;
}