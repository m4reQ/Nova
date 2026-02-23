#include <Nova/dotnet/DotnetMemory.hpp>
#include <Nova/core/Build.hpp>

#ifdef NV_WINDOWS
#include <Windows.h>
#else
#include <cstdlib>
#endif

using namespace Nova;

void *DotnetMemory::AllocHGlobal(size_t size)
{
    void *data;

#ifdef NV_WINDOWS
    data = HeapAlloc(GetProcessHeap(), 0, size);
#else
    data = malloc(size);
#endif

    NV_ASSERT(data != nullptr, "Failed to allocate global string pointer.");
    return data;
}

void DotnetMemory::FreeHGlobal(void *memory)
{
#ifdef NV_WINDOWS
    HeapFree(GetProcessHeap(), 0, memory);
#else
    free(memory);
#endif
}
