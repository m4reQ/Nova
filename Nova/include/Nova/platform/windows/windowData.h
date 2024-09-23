#pragma once
#include <stdbool.h>
#include <Windows.h>

typedef struct
{
    HWND window;
    bool shouldClose;
} NvWindowData;
