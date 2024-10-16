#include <Nova/dotnet/interop/interop_memory.h>
#include <Windows.h>

void *NvInteropMemoryAlloc(size_t size)
{
    return GlobalAlloc(GMEM_FIXED, size);
}

void NvInteropMemoryFree(void *ptr)
{
    GlobalFree(ptr);
}
