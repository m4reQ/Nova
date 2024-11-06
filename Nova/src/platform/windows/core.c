#include <Nova/platform/windows/core.h>
#include <Windows.h>
#include <stdio.h>

#define MESSAGE_BUFFER_LENGTH 64 * 1024

const char *_NvWin32GetLastErrorString(void)
{
    static char s_MessageBuffer[MESSAGE_BUFFER_LENGTH];

    const DWORD lastError = GetLastError();
    const DWORD msgLength = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        lastError,
        LANG_NEUTRAL,
        s_MessageBuffer,
        MESSAGE_BUFFER_LENGTH,
        NULL);

    if (msgLength == 0)
    {
        fprintf(stderr, "Failed to format Win32 message.\n");
        return "";
    }

    return s_MessageBuffer;
}
