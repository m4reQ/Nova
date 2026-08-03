#pragma once

namespace Nova
{
    struct ProcessMemoryInfo
    {
        size_t VirtualMemoryUsed;
        size_t PhysicalMemoryUsed;
    };

    namespace System
    {
        ProcessMemoryInfo GetProcMemoryInfo() noexcept;
    }
}