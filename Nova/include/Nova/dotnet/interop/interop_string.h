#pragma once
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

NvInteropString NvInteropStringCreateStack(const char *string);
NvInteropString NvInteropStringCreateStackW(const wchar_t *string);
NvInteropString NvInteropStringCreateGlobal(const char *string);
NvInteropString NvInteropStringCreateGlobalW(const wchar_t *string);
void NvInteropStringFree(NvInteropString *string);
char *NvInteropStringDup(const NvInteropString *string);
wchar_t *NvInteropStringDupW(const NvInteropString *string);
