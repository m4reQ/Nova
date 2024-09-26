#include <Nova/core/file.h>
#include <io.h>
#include <Windows.h>

size_t _NvFileGetSize(FILE *file)
{
    HANDLE handle = (HANDLE)_get_osfhandle(fileno(file));

    DWORD fileSizeHigh = 0;
    DWORD fileSizeLow = GetFileSize(handle, &fileSizeHigh);

    return (size_t)fileSizeLow | ((size_t)fileSizeHigh << 32);
}
