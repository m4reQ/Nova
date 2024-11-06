#pragma once
#include <Nova/core/build.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum
{
    NV_ALLOCATOR_UNDEFINED,
    NV_ALLOCATOR_ARENA,
    NV_ALLOCATOR_STDLIB,
} NvAllocatorType;

typedef struct NvAllocator
{
    NvAllocatorType type;
    void (*free)(struct NvAllocator *allocator, void *ptr);
    void *(*malloc)(struct NvAllocator *allocator, size_t size);
    void *(*calloc)(struct NvAllocator *allocator, size_t elementSize, size_t elementsCount);
    void *(*realloc)(struct NvAllocator *allocator, void *old, size_t newSize);
    void *data;
} NvAllocator;

NV_API void *NvMemoryAllocatorMalloc(NvAllocator *allocator, size_t size);
NV_API void *NvMemoryAllocatorCalloc(NvAllocator *allocator, size_t elementSize, size_t nElements);
NV_API void *NvMemoryAllocatorRealloc(NvAllocator *allocator, void *oldPtr, size_t newSize);
NV_API void NvMemoryAllocatorFree(NvAllocator *allocator, void *ptr);
NV_API void *NvMemoryRawMalloc(size_t size);
NV_API void *NvMemoryRawCalloc(size_t elementSize, size_t nElements);
NV_API void *NvMemoryRawRealloc(void *oldPtr, size_t newSize);
NV_API void NvMemoryRawFree(void *ptr);

// TODO Add platform allocation functions
