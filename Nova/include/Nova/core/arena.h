#pragma once
#include <Nova/core/build.h>
#include <Nova/core/memory.h>
#include <stdint.h>
#include <stddef.h>

typedef struct NvArenaRegion
{
    size_t capacity, size;
    struct NvArenaRegion *prev, *next;
    uintptr_t data[];
} NvArenaRegion;

typedef struct
{
    NvArenaRegion *begin, *end;
} NvArena;

NV_API void NvArenaDestroy(NvArena *arena);
NV_API void *NvArenaMalloc(NvArena *arena, size_t size);
NV_API void *NvArenaCalloc(NvArena *arena, size_t elementSize, size_t elementsCount);
NV_API void *NvArenaRealloc(NvArena *arena, void *ptr, size_t size);
NV_API size_t NvArenaGetSize(NvArena *arena);
NV_API NvAllocator NvArenaGetAllocator(NvArena *arena);
