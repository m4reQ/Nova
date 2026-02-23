#pragma once
#include <Windows.h>

namespace Nova
{
    struct WindowData
    {
        HINSTANCE Instance;
        HWND Window;
        HICON Icon;
        WINDOWPLACEMENT SavedPlacement;
        DWORD OriginalStyle;
        bool ShouldClose, IsFullscreen, IsVisible;
    };
}