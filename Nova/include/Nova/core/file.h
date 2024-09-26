#pragma once
#include <Nova/core/build.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct
{
    FILE *file;
    size_t fileSize;
} NvFile;

NV_API bool NvFileOpen(NvFile *out, const char *filepath, const char *mode);
NV_API bool NvFileOpenUnicode(NvFile *out, const wchar_t *filepath, const wchar_t *mode);
NV_API void NvFileClose(NvFile *file);
NV_API size_t NvFileRead(NvFile *file, size_t bytesCount, void *buffer);
NV_API size_t NvFileReadToEnd(NvFile *file, size_t bufferSize, void *buffer);
NV_API char *NvFileReadLine(NvFile *file, size_t bufferSize, void *buffer);
NV_API size_t NvFileWrite(NvFile *file, size_t bytesCount, const void *data);

size_t _NvFileGetSize(FILE *file);
