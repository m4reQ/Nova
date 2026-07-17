#include <Nova/platform/windows/WindowHandle.hpp>
#include <stdexcept>

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    const std::string_view className,
    const std::string_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance)
    : handle_(
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
              nullptr))
{
    if (handle_ == nullptr)
        throw std::runtime_error("Failed to create window.");
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
    HINSTANCE hInstance)
    : WindowHandle(
          exStyle,
          reinterpret_cast<LPCSTR>(wndClass),
          windowName.data(),
          style,
          x,
          y,
          width,
          height,
          hInstance) {}

Nova::WindowHandle::WindowHandle(
    DWORD exStyle,
    const std::wstring_view className,
    const std::wstring_view windowName,
    DWORD style,
    int x,
    int y,
    int width,
    int height,
    HINSTANCE hInstance)
    : handle_(
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
              nullptr))
{
    if (handle_ == nullptr)
        throw std::runtime_error("Failed to create window.");
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
    HINSTANCE hInstance)
    : WindowHandle(
          exStyle,
          reinterpret_cast<LPCWSTR>(wndClass),
          windowName.data(),
          style,
          x,
          y,
          width,
          height,
          hInstance) {}

Nova::WindowHandle::~WindowHandle() noexcept
{
    if (handle_ != nullptr)
        DestroyWindow(handle_);
}
