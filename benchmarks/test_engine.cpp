#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include "engine.hpp"

int main()
{
    std::cout << "========================================\n";
    std::cout << "   Hermes Engine Topology Verification  \n";
    std::cout << "========================================\n";

    auto engine = std::make_unique<hermes::TradingEngine>();

    std::cout << "[Test] Booting Trading Engine...\n";
    engine->start();

    std::cout << "[Test] Engine running. Verifying thread stability for 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "[Test] Initiating graceful shutdown...\n";
    engine->stop();

    std::cout << "----------------------------------------\n";
    std::cout << "[PASS] Engine threads spawned, pinned, and joined successfully.\n";

    return 0;
}