#include <Nova/io/file.h>
#include <io.h>
#include <Windows.h>

size_t _NvFileGetSize(FILE *file)
{
    HANDLE handle = (HANDLE)_get_osfhandle(fileno(file));

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(handle, &fileSize))
        return -1;

    return (size_t)fileSize.QuadPart;
}
