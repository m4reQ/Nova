#pragma once
#include <stdbool.h>
#include <Windows.h>

typedef struct
{
    HINSTANCE instance;
    HWND window;
    HICON icon;
    WINDOWPLACEMENT savedPlacement;
    DWORD originalStyle;
    bool shouldClose, isFullscreen, isVisible;
} NvWindowData;
