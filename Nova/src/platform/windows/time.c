#include <Nova/core/time.h>
#include <Windows.h>

uint64_t NvTimePerformanceGetNow(void)
{
    LARGE_INTEGER now;
    const BOOL success = QueryPerformanceCounter(&now);
    return success ? (uint64_t)now.QuadPart : -1;
}

uint64_t NvTimePerformanceGetResolution(void)
{
    static uint64_t s_Frequency = 0;
    if (s_Frequency == 0)
    {
        LARGE_INTEGER frequency;
        const BOOL success = QueryPerformanceFrequency(&frequency);
        s_Frequency = success ? (uint64_t)frequency.QuadPart : -1;
    }

    return s_Frequency;
}
