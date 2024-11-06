#pragma once
#include <Nova/core/build.h>
#include <Nova/core/memory.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

NV_API const char *NvPathGetExtension(const char *filepath);
NV_API const wchar_t *NvPathGetExtensionW(const wchar_t *filepath);
NV_API char *NvPathSplitExtension(NvAllocator *allocator, const char *filepath, char **outExtension);
NV_API wchar_t *NvPathSplitExtensionW(NvAllocator *allocator, const wchar_t *filepath, wchar_t **outExtension);
NV_API char *NvPathJoin(NvAllocator *allocator, size_t nPaths, ...);
NV_API char *NvPathJoinVa(NvAllocator *allocator, size_t nPaths, va_list args);
NV_API wchar_t *NvPathJoinW(NvAllocator *allocator, size_t nPaths, ...);
NV_API wchar_t *NvPathJoinVaW(NvAllocator *allocator, size_t nPaths, va_list args);
NV_API bool NvPathIsFile(const char *filepath);
NV_API bool NvPathIsFileW(const wchar_t *filepath);
NV_API bool NvPathIsDirectory(const char *filepath);
NV_API bool NvPathIsDirectoryW(const wchar_t *filepath);
NV_API bool NvPathExists(const char *filepath);
NV_API bool NvPathExistsW(const wchar_t *filepath);
