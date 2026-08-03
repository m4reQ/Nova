#include <Nova/core/System.hpp>
#include <Windows.h>
#include <Psapi.h>

using namespace Nova;

// Fucking windows API without prefixes
ProcessMemoryInfo System::GetProcMemoryInfo() noexcept
{
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX info;
    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS *)&info, sizeof(info));

    return ProcessMemoryInfo{
        .VirtualMemoryUsed = info.PrivateUsage,
        .PhysicalMemoryUsed = info.WorkingSetSize,
    };
}