#include <Nova/platform/windows/WindowHandle.hpp>
#include <Nova/platform/windows/Error.hpp>
#include <stdexcept>

Nova::WindowHandle::WindowHandle(HWND handle) noexcept
    : handle_(
          handle,
          [](auto x)
          {
              if (x != nullptr)
                  DestroyWindow(x);
          }) {}

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    const std::string_view className,
    const std::string_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance,
    void *userData)
    : WindowHandle(
          CreateWindowExA(
              exStyle,
              className.data(),
              windowName.data(),
              style,
              x,
              y,
              width,
              height,
              nullptr,
              nullptr,
              hInstance,
              userData))
{
    if (handle_ == nullptr)
        throw Win32::Exception("Failed to create window");
}

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    ATOM wndClass,
    const std::string_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance,
    void *userData)
    : WindowHandle(
          CreateWindowExA(
              exStyle,
              reinterpret_cast<LPCSTR>(MAKEINTATOM(wndClass)),
              windowName.data(),
              style,
              x,
              y,
              width,
              height,
              nullptr,
              nullptr,
              hInstance,
              userData))
{
    if (handle_ == nullptr)
        throw Win32::Exception("Failed to create window");
}

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    const std::wstring_view className,
    const std::wstring_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance,
    void *userData)
    : WindowHandle(
          CreateWindowExW(
              exStyle,
              className.data(),
              windowName.data(),
              style,
              x,
              y,
              width,
              height,
              nullptr,
              nullptr,
              hInstance,
              userData))
{
    if (handle_ == nullptr)
        throw Win32::Exception("Failed to create window");
}

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    ATOM wndClass,
    const std::wstring_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance,
    void *userData)
    : WindowHandle(
          CreateWindowExW(
              exStyle,
              reinterpret_cast<LPCWSTR>(MAKEINTATOM(wndClass)),
              windowName.data(),
              style,
              x,
              y,
              width,
              height,
              nullptr,
              nullptr,
              hInstance,
              userData))
{
    if (handle_ == nullptr)
        throw Win32::Exception("Failed to create window");
}
