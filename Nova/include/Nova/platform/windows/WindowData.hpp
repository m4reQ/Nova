#pragma once
#include <Nova/platform/windows/DeviceContext.hpp>
#include <Nova/platform/windows/WGLContext.hpp>
#include <Nova/platform/windows/WindowClass.hpp>
#include <Nova/platform/windows/WindowHandle.hpp>

namespace Nova
{
    struct WindowData
    {
        WindowClass wndClass;
        WindowHandle handle;
        DeviceContext deviceContext;
        WGLContext wglContext;
    };
}