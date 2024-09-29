#include <Nova/core/arena.h>
#include <stdlib.h>
#include <memory.h>

#define DEFAULT_REGION_CAPACITY 1024

static void AllocatorFree(NvAllocator *allocator, void *ptr)
{
    NV_UNUSED(allocator);
    NV_UNUSED(ptr);
    return;
}

static void *AllocatorMalloc(NvAllocator *allocator, size_t size)
{
    return NvArenaMalloc((NvArena *)allocator->data, size);
}

static void *AllocatorCalloc(NvAllocator *allocator, size_t elementSize, size_t elementsCount)
{
    return NvArenaCalloc((NvArena *)allocator->data, elementSize, elementsCount);
}

static void *AllocatorRealloc(NvAllocator *allocator, void *old, size_t newSize)
{
    return NvArenaRealloc((NvArena *)allocator->data, old, newSize);
}

static NvArenaRegion *GetFirstSuitableRegion(const NvArena *arena, size_t sizePages)
{
    NvArenaRegion *current = arena->begin;
    while (current != NULL && current->size + sizePages > current->capacity)
        current = current->next;

    return current;
}

static NvArenaRegion *CreateNewRegion(size_t capacityPages)
{
    NvArenaRegion *region = _NvMemoryRawMalloc(sizeof(NvArenaRegion) + sizeof(uintptr_t) * capacityPages);
    *region = (NvArenaRegion){
        .capacity = capacityPages,
        .size = 0,
        .next = NULL,
    };

    return region;
}

void NvArenaDestroy(NvArena *arena)
{
    if (!arena)
        return;

    NvArenaRegion *current = arena->begin;
    while (current != NULL)
    {
        NvArenaRegion *next = current->next;
        _NvMemoryRawFree(current);

        current = next;
    }

    memset(arena, 0, sizeof(NvArena));
}

void *NvArenaMalloc(NvArena *arena, size_t size)
{
    size += sizeof(size_t);

    size_t sizePages = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

    if (!arena->end)
    {
        size_t capacityPages = max(DEFAULT_REGION_CAPACITY, sizePages);

        arena->end = CreateNewRegion(capacityPages);
        arena->begin = arena->end;
    }

    while (arena->end->size + sizePages > arena->end->capacity && arena->end->next)
        arena->end = arena->end->next;

    if (arena->end->size + sizePages > arena->end->capacity)
    {
        size_t capacityPages = max(DEFAULT_REGION_CAPACITY, sizePages);

        arena->end->next = CreateNewRegion(capacityPages);
        arena->end = arena->end->next;
    }

    void *resultBase = &arena->end->data[arena->end->size];
    *(size_t *)resultBase = size;

    arena->end->size += sizePages;

    return (uintptr_t *)resultBase + 1;
}

void *NvArenaCalloc(NvArena *arena, size_t elementSize, size_t elementsCount)
{
    size_t sizeBytes = elementSize * elementsCount;
    void *result = NvArenaMalloc(arena, sizeBytes);
    memset(result, 0, sizeBytes);

    return result;
}

void *NvArenaRealloc(NvArena *arena, void *ptr, size_t size)
{
    size_t oldSize = *(size_t *)((uintptr_t *)ptr - 1);
    if (size <= oldSize)
        return ptr;

    void *newPtr = NvArenaMalloc(arena, size);
    memcpy(newPtr, ptr, size);

    return newPtr;
}

size_t NvArenaGetSize(NvArena *arena)
{
    if (!arena)
        return 0;

    size_t size = 0;

    NvArenaRegion *current = arena->begin;
    while (current != NULL)
    {
        size += current->size;
        current = current->next;
    }

    return size * sizeof(uintptr_t);
}

NvAllocator NvArenaGetAllocator(NvArena *arena)
{
    return (NvAllocator){
        .free = AllocatorFree,
        .malloc = AllocatorMalloc,
        .calloc = AllocatorCalloc,
        .realloc = AllocatorRealloc,
        .data = arena,
    };
}
