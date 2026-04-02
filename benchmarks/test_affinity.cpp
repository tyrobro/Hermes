#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include "os_utils.hpp"

using namespace std;

void worker_verify_affinity(int target_core, atomic<bool> &passed)
{
    if (!hermes::pin_current_thread(target_core))
    {
        cerr << "[-] Error: Failed to apply affinity mask for core " << target_core << "\n";
        passed = false;
        return;
    }

    cout << "[+] Thread started on Core " << hermes::get_current_core() << " (Target: " << target_core << ")\n";

    bool stayed_on_core = true;
    for (volatile int i = 0; i < 50000000; i++)
    {
        if (hermes::get_current_core() != target_core)
        {
            stayed_on_core = false;
            break;
        }
    }

    passed = stayed_on_core;
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "   Hermes Thread Affinity Verification  \n";
    std::cout << "========================================\n";

    const int core_a = 2;
    const int core_b = 3;

    std::atomic<bool> success_a{false};
    std::atomic<bool> success_b{false};

    std::thread t1(worker_verify_affinity, core_a, std::ref(success_a));
    std::thread t2(worker_verify_affinity, core_b, std::ref(success_b));

    t1.join();
    t2.join();

    std::cout << "----------------------------------------\n";
    if (success_a && success_b)
    {
        std::cout << "[PASS] Threads successfully locked to silicon.\n";
        std::cout << "[PASS] OS scheduler isolation verified.\n";
        return 0;
    }
    else
    {
        std::cout << "[FAIL] Threads migrated across cores! Affinity broken.\n";
        return 1;
    }
}