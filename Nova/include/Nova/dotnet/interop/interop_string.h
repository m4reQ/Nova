#pragma once
#include <Nova/core/build.h>
#include <Nova/core/memory.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    NV_ENCODING_AUTO = 0,
    NV_ENCODING_ANSI = 1,
    NV_ENCODING_UNICODE = 2,
} NvEncoding;

typedef struct
{
    const void *data;
    bool shouldDispose, isDisposed;
} NvInteropString;

NV_API NvInteropString NvInteropStringCreateStack(const char *string);
NV_API NvInteropString NvInteropStringCreateStackW(const wchar_t *string);
NV_API NvInteropString NvInteropStringCreateGlobal(const char *string);
NV_API NvInteropString NvInteropStringCreateGlobalW(const wchar_t *string);
NV_API char *NvInteropStringMoveToLocal(NvInteropString *string, NvAllocator *allocator);
NV_API wchar_t *NvInteropStringMoveToLocalW(NvInteropString *string, NvAllocator *allocator);
NV_API void NvInteropStringFree(NvInteropString *string);
