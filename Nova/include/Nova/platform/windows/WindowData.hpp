#pragma once
#include <Nova/platform/windows/DeviceContext.hpp>
#include <Nova/platform/windows/WGLContext.hpp>
#include <Nova/platform/windows/WindowClass.hpp>
#include <Nova/platform/windows/WindowHandle.hpp>
#include <Nova/platform/windows/Icon.hpp>
#include <optional>

namespace Nova
{
    struct WindowData
    {
        HINSTANCE instance;
        WindowClass wndClass;
        WindowHandle handle;
        DeviceContext deviceContext;
        WGLContext wglContext;
        std::optional<Icon> userIcon;
    };
}