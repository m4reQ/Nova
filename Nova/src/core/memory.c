#include <Nova/core/memory.h>

static void ProfileMemoryAlloc(NvAllocatorType allocatorType, size_t size)
{
#if NV_DEBUG
    return;
#endif

    // TODO Implement profiling for memory allocations
}

void *NvMemoryAllocatorMalloc(NvAllocator *allocator, size_t size)
{
    if (allocator != NULL)
    {
        ProfileMemoryAlloc(allocator->type, size);
        return allocator->malloc(allocator, size);
    }

    return NvMemoryRawMalloc(size);
}

void *NvMemoryAllocatorCalloc(NvAllocator *allocator, size_t elementSize, size_t nElements)
{
    if (allocator != NULL)
    {
        ProfileMemoryAlloc(allocator->type, elementSize * nElements);
        return allocator->calloc(allocator, elementSize, nElements);
    }

    return NvMemoryRawCalloc(elementSize, nElements);
}

void *NvMemoryAllocatorRealloc(NvAllocator *allocator, void *oldPtr, size_t newSize)
{
    if (allocator != NULL)
    {
        ProfileMemoryAlloc(allocator->type, newSize);
        return allocator->realloc(allocator, oldPtr, newSize);
    }

    return NvMemoryRawRealloc(oldPtr, newSize);
}

void NvMemoryAllocatorFree(NvAllocator *allocator, void *ptr)
{
    if (allocator != NULL)
        allocator->free(allocator, ptr);
    else
        NvMemoryRawFree(ptr);
}

void *NvMemoryRawMalloc(size_t size)
{
    ProfileMemoryAlloc(NV_ALLOCATOR_STDLIB, size);
    return malloc(size);
}

void *NvMemoryRawCalloc(size_t elementSize, size_t nElements)
{
    ProfileMemoryAlloc(NV_ALLOCATOR_STDLIB, nElements * elementSize);
    return calloc(nElements, elementSize);
}

void *NvMemoryRawRealloc(void *oldPtr, size_t newSize)
{
    ProfileMemoryAlloc(NV_ALLOCATOR_STDLIB, newSize);
    return realloc(oldPtr, newSize);
}

void NvMemoryRawFree(void *ptr)
{
    free(ptr);
}
