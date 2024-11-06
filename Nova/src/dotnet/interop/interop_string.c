#include <Nova/dotnet/interop/interop_string.h>
#include <Nova/dotnet/interop/interop_memory.h>
#include <string.h>

NvInteropString NvInteropStringCreateStack(const char *string)
{
    return (NvInteropString){
        .data = string,
        .isDisposed = false,
        .shouldDispose = false,
    };
}

NvInteropString NvInteropStringCreateStackW(const wchar_t *string)
{
    return (NvInteropString){
        .data = string,
        .isDisposed = false,
        .shouldDispose = false,
    };
}

NvInteropString NvInteropStringCreateGlobal(const char *string)
{
    const size_t len = strlen(string) + 1;
    char *data = NvInteropMemoryAlloc(len * sizeof(char));
    strcpy(data, string);
    data[len] = '\0';

    return (NvInteropString){
        .data = data,
        .isDisposed = false,
        .shouldDispose = true,
    };
}

NvInteropString NvInteropStringCreateGlobalW(const wchar_t *string)
{
    const size_t len = wcslen(string) + 1;
    wchar_t *data = NvInteropMemoryAlloc(len * sizeof(wchar_t));
    wcscpy(data, string);
    data[len] = L'\0';

    return (NvInteropString){
        .data = data,
        .isDisposed = false,
        .shouldDispose = true,
    };
}

void NvInteropStringFree(NvInteropString *string)
{
    if (!string->shouldDispose || string->isDisposed)
        return;

    NvInteropMemoryFree((void *)string->data);
    string->isDisposed = true;
}

char *NvInteropStringMoveToLocal(NvInteropString *string, NvAllocator *allocator)
{
    if (string->isDisposed)
        return NULL;

    const size_t length = strlen(string->data);
    char *dataLocal = NvMemoryAllocatorMalloc(allocator, (length + 1) * sizeof(char));

    memcpy(dataLocal, string->data, length * sizeof(char));
    dataLocal[length] = '\0';

    NvInteropStringFree(string);

    return dataLocal;
}

wchar_t *NvInteropStringMoveToLocalW(NvInteropString *string, NvAllocator *allocator)
{
    if (string->isDisposed)
        return NULL;

    const size_t length = wcslen(string->data);
    wchar_t *dataLocal = NvMemoryAllocatorMalloc(allocator, (length + 1) * sizeof(wchar_t));

    memcpy(dataLocal, string->data, length * sizeof(wchar_t));
    dataLocal[length] = L'\0';

    NvInteropStringFree(string);

    return dataLocal;
}
