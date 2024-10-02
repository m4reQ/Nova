#pragma once
#include <Nova/core/build.h>
#include <Nova/core/memory.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

// TODO Implement wide char API

#define NV_SV(x) \
    (NvStringView) { .data = x, .length = x ? strlen(x) : 0 }
#define NV_WSV(x) \
    (NvWStringView) { .data = x, .length = x ? wcslen(x) : 0 }

typedef struct
{
    char *data;
    size_t length;
} NvStringView;

typedef struct
{
    wchar_t *data;
    size_t length;
} NvWStringView;

// ANSI API
NV_API char NvStringGetDecimalSeparator(void);
NV_API wchar_t *NvStringToWide(const NvStringView sv, NvAllocator *allocator);
NV_API NvStringView NvStringLStrip(const NvStringView sv);
NV_API NvStringView NvStringRStrip(const NvStringView sv);
NV_API NvStringView NvStringStrip(const NvStringView sv);
NV_API NvStringView NvStringSplit(const NvStringView sv, char delim, NvStringView *rest);
NV_API char *NvStringDuplicate(const NvStringView sv, NvAllocator *allocator);
NV_API NvStringView NvStringToLower(const NvStringView sv);
NV_API NvStringView NvStringToUpper(const NvStringView sv);
NV_API bool NvStringTryParseInt32(const NvStringView sv, int32_t *out);
NV_API bool NvStringTryParseInt64(const NvStringView sv, int64_t *out);
NV_API bool NvStringTryParseFloat(const NvStringView sv, float *out);
NV_API bool NvStringTryParseDouble(const NvStringView sv, double *out);
NV_API NvStringView NvStringChopLeft(const NvStringView sv, size_t count);
NV_API NvStringView NvStringChopRight(const NvStringView sv, size_t count);
NV_API NvStringView NvStringChopUntil(const NvStringView sv, bool (*predicate)(char), size_t *choppedCount);
NV_API bool NvStringContains(const NvStringView sv, char character);
NV_API bool NvStringContainsAny(const NvStringView sv, const NvStringView characters);
NV_API NvStringView NvStringReplace(const NvStringView sv, char from, char to);
NV_API NvStringView NvStringSubstring(const NvStringView sv, size_t begin, int64_t end);
NV_API char *NvStringConcat(size_t partsCount, NvStringView *parts, NvAllocator *allocator);
NV_API bool NvStringStartsWithChar(const NvStringView sv, char value);
NV_API bool NvStringStartsWith(const NvStringView sv, const NvStringView other);
NV_API bool NvStringEndsWithChar(const NvStringView sv, char value);
NV_API bool NvStringIsEmpty(const NvStringView sv);
