#pragma once

#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

namespace hermes
{
    inline bool pin_current_thread(int core_id)
    {
#ifdef _WIN32
        if (core_id < 0 || core_id >= 64)
        {
            return false;
        }
        HANDLE thread_handle = GetCurrentThread();
        DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << core_id);
        DWORD_PTR result = SetThreadAffinityMask(thread_handle, mask);
        return (result != 0);
#else
        return false;
#endif
    }

    inline int get_current_core()
    {
#ifdef _WIN32
        return GetCurrentProcessorNumber();
#else
        return -1;
#endif
    }
}