#include <Nova/core/path.h>
#include <Nova/platform/windows/core.h>
#include <Shlwapi.h>
#include <stdio.h>

const char *NvPathGetExtension(const char *filepath)
{
    return PathFindExtensionA(filepath) + 1;
}

const wchar_t *NvPathGetExtensionW(const wchar_t *filepath)
{
    return PathFindExtensionW(filepath) + 1;
}

char *NvPathSplitExtension(NvAllocator *allocator, const char *filepath, char **outExtension)
{
    if (filepath == NULL)
    {
        if (outExtension != NULL)
            *outExtension = NULL;

        return NULL;
    }

    const size_t filepathLength = strlen(filepath);

    char *filepathAlloc = NvMemoryAllocatorMalloc(allocator, filepathLength * sizeof(char));
    memcpy(filepathAlloc, filepath, filepathLength * sizeof(char));

    char *extension = (char *)PathFindExtension(filepathAlloc);
    if (outExtension != NULL)
        *outExtension = extension;

    *(extension - 1) = '\0';

    return filepathAlloc;
}

wchar_t *NvPathSplitExtensionW(NvAllocator *allocator, const wchar_t *filepath, wchar_t **outExtension)
{
    if (filepath == NULL)
    {
        if (outExtension != NULL)
            *outExtension = NULL;

        return NULL;
    }

    const size_t filepathLength = wcslen(filepath);

    wchar_t *filepathAlloc = NvMemoryAllocatorMalloc(allocator, filepathLength * sizeof(wchar_t));
    memcpy(filepathAlloc, filepath, filepathLength * sizeof(wchar_t));

    wchar_t *extension = (wchar_t *)PathFindExtensionW(filepathAlloc);
    if (outExtension != NULL)
        *outExtension = extension;

    *(extension - 1) = L'\0';

    return filepathAlloc;
}

char *NvPathJoin(NvAllocator *allocator, size_t nPaths, ...)
{
    va_list args;
    va_start(args, nPaths);

    char *result = NvPathJoinVa(allocator, nPaths, args);

    va_end(args);

    return result;
}

wchar_t *NvPathJoinW(NvAllocator *allocator, size_t nPaths, ...)
{
    va_list args;
    va_start(args, nPaths);

    wchar_t *result = NvPathJoinVaW(allocator, nPaths, args);

    va_end(args);

    return result;
}

char *NvPathJoinVa(NvAllocator *allocator, size_t nPaths, va_list args)
{
    const size_t bufferSize = MAX_PATH * sizeof(char);
    char *buffer = NvMemoryAllocatorMalloc(allocator, bufferSize);
    buffer[0] = '\0';

    for (size_t i = 0; i < nPaths; i++)
    {
        const char *path = va_arg(args, char *);
        char *result = PathCombineA(buffer, NULL, path);

        if (result == NULL)
        {
            fprintf(stderr, "Failed to combine path: %s.\n", _NvWin32GetLastErrorString());

            NvMemoryAllocatorFree(allocator, buffer);
            return NULL;
        }
    }

    return buffer;
}

wchar_t *NvPathJoinVaW(NvAllocator *allocator, size_t nPaths, va_list args)
{
    const size_t bufferSize = MAX_PATH * sizeof(wchar_t);
    wchar_t *buffer = NvMemoryAllocatorMalloc(allocator, bufferSize);
    buffer[0] = L'\0';

    for (size_t i = 0; i < nPaths; i++)
    {
        const wchar_t *path = va_arg(args, wchar_t *);
        wchar_t *result = PathCombineW(buffer, buffer, path);

        if (result == NULL)
        {
            fprintf(stderr, "Failed to combine path: %s.\n", _NvWin32GetLastErrorString());

            NvMemoryAllocatorFree(allocator, buffer);
            return NULL;
        }
    }

    return buffer;
}

bool NvPathIsFile(const char *filepath)
{
    const DWORD attributes = GetFileAttributesA(filepath);
    return ((attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
}

bool NvPathIsFileW(const wchar_t *filepath)
{
    const DWORD attributes = GetFileAttributesW(filepath);
    return ((attributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY);
}

bool NvPathIsDirectory(const char *filepath)
{
    return (bool)PathIsDirectoryA(filepath);
}

bool NvPathIsDirectoryW(const wchar_t *filepath)
{
    return (bool)PathIsDirectoryW(filepath);
}

bool NvPathExists(const char *filepath)
{
    return (bool)PathFileExistsA(filepath);
}

bool NvPathExistsW(const wchar_t *filepath)
{
    return (bool)PathFileExistsW(filepath);
}
