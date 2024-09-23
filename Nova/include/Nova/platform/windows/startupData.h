#pragma once
#include <Windows.h>

typedef struct
{
    HINSTANCE executableInstance;
    HINSTANCE libraryInstance;
    size_t argsCount;
    const char **args;
} NvStartupData;
