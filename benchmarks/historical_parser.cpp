#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "engine.hpp"

namespace hermes
{

    inline bool parse_tick_line(const std::string &line, Tick &tick)
    {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        size_t pos4 = line.find(',', pos3 + 1);

        if (pos1 == std::string::npos || pos2 == std::string::npos ||
            pos3 == std::string::npos || pos4 == std::string::npos)
        {
            return false;
        }

        try
        {
            tick.timestamp = std::stoull(line.substr(0, pos1));
            tick.instrument_id = std::stoul(line.substr(pos1 + 1, pos2 - pos1 - 1));
            tick.price = std::stod(line.substr(pos2 + 1, pos3 - pos2 - 1));
            tick.volume = std::stoul(line.substr(pos3 + 1, pos4 - pos3 - 1));
            tick.side = static_cast<uint8_t>(std::stoul(line.substr(pos4 + 1)));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

}

int main(int argc, char *argv[])
{
    std::cout << "========================================\n";
    std::cout << "   Hermes Historical Backtest Engine    \n";
    std::cout << "========================================\n";

    if (argc < 2)
    {
        std::cerr << "[-] Error: Missing data file.\n";
        std::cerr << "Usage: .\\historical_parser.exe <path_to_csv>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "[-] Error: Could not open file: " << filename << "\n";
        return 1;
    }

    auto engine = std::make_unique<hermes::TradingEngine>();
    engine->set_imbalance_ratio(3);
    engine->start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string line;
    hermes::Tick tick;
    uint64_t ticks_processed = 0;

    std::cout << "[Backtest] Parsing file: " << filename << "\n";
    std::cout << "[Backtest] Injecting data into pipeline...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    while (std::getline(file, line))
    {
        if (hermes::parse_tick_line(line, tick))
        {
            engine->inject_mock_tick(tick);
            ticks_processed++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "[Backtest] End of file reached. Shutting down...\n";
    engine->stop();

    double throughput = (ticks_processed / elapsed.count()) / 1000000.0;

    std::cout << "----------------------------------------\n";
    std::cout << "[RESULTS] Ticks Processed : " << ticks_processed << "\n";
    std::cout << "[RESULTS] Time Elapsed    : " << elapsed.count() << " seconds\n";
    std::cout << "[RESULTS] Engine Throughput: " << throughput << " Million Ticks / Sec\n";
    std::cout << "========================================\n";

    return 0;
}