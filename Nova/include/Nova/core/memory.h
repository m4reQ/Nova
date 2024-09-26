#pragma once
#include <stdint.h>
#include <stdlib.h>

#define NV_ALLOCATOR_MALLOC(allocator, size) allocator ? allocator->malloc(allocator, size) : malloc(size)
#define NV_ALLOCATOR_CALLOC(allocator, elementSize, elementsCount) allocator ? allocator->calloc(allocator, elementSize, elementsCount) : calloc(elementSize, elementsCount)
#define NV_ALLOCATOR_REALLOC(allocator, old, newSize) allocator ? allocator->realloc(allocator, old, newSize) : realloc(old, newSize)
#define NV_ALLOCATOR_FREE(allocator, ptr) allocator ? allocator->free(allocator, ptr) : free(ptr)

typedef struct NvAllocator
{
    void (*free)(struct NvAllocator *allocator, void *ptr);
    void *(*malloc)(struct NvAllocator *allocator, size_t size);
    void *(*calloc)(struct NvAllocator *allocator, size_t elementSize, size_t elementsCount);
    void *(*realloc)(struct NvAllocator *allocator, void *old, size_t newSize);
    void *data;
} NvAllocator;

void *_NvMemoryRawMalloc(size_t size);
void _NvMemoryRawFree(void *ptr);
