#pragma once
#include <Nova/platform/windows/DeviceContext.hpp>
#include <Nova/platform/windows/WindowClass.hpp>
#include <Nova/platform/windows/WindowHandle.hpp>
#include <Nova/platform/windows/Icon.hpp>
#include <Nova/platform/windows/Cursor.hpp>
#include <Nova/platform/windows/WinRect.hpp>
#include <Nova/graphics/FullscreenMode.hpp>
#include <optional>

namespace Nova
{
    struct WindowData
    {
        HINSTANCE instance;
        WindowClass wndClass;
        WindowHandle handle;
        DeviceContext deviceContext;
        std::optional<Icon> userIcon;
        std::optional<Cursor> userCursor;
        WinRect savedRect;
        FullscreenMode fsMode;
        DWORD savedStyle;
        DWORD savedStyleEx;
        bool shouldClose = false;
        bool savedMaximizedState = false;
    };
}