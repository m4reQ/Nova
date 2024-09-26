#include <Nova/core/memory.h>
#include <Windows.h>

void *_NvMemoryRawMalloc(size_t size)
{
    return VirtualAllocEx(
        GetCurrentProcess(),
        NULL,
        size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
}

void _NvMemoryRawFree(void *ptr)
{
    VirtualFreeEx(
        GetCurrentProcess(),
        ptr,
        0,
        MEM_RELEASE);
}
