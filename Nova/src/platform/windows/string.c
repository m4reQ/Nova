#include <Nova/core/string.h>
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

#define MAX_DECIMAL_SEPARATOR_CHARS 4

char NvStringGetDecimalSeparator(void)
{
    char separator[MAX_DECIMAL_SEPARATOR_CHARS];
    GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, separator, MAX_DECIMAL_SEPARATOR_CHARS);

    return separator[0];
}

wchar_t *NvStringToWide(const NvStringView sv, NvAllocator *allocator)
{
    int requiredSize = MultiByteToWideChar(
        CP_UTF8,
        0,
        sv.data,
        -1,
        NULL,
        0);
    wchar_t *result = NV_ALLOCATOR_MALLOC(allocator, requiredSize);
    int bytesWritten = MultiByteToWideChar(
        CP_UTF8,
        0,
        sv.data,
        -1,
        result,
        requiredSize);

    if (bytesWritten != requiredSize)
    {
        fprintf(stderr, "Failed to convert multibyte string to wide char string.\n");
        return NULL;
    }

    return result;
}

char *NvWStringToMultibyte(const wchar_t *str)
{
    int requiredSize = WideCharToMultiByte(
        CP_UTF8,
        0,
        str,
        -1,
        NULL,
        0,
        NULL,
        NULL);
    char *result = malloc(requiredSize);
    int bytesWritten = WideCharToMultiByte(
        CP_UTF8,
        0,
        str,
        -1,
        result,
        requiredSize,
        NULL,
        NULL);

    if (bytesWritten != requiredSize)
    {
        fprintf(stderr, "Failed to convert wide char string to multibyte string.\n");
        return NULL;
    }

    return result;
}
