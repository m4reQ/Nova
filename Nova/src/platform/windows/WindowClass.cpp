#include <Nova/platform/windows/WindowClass.hpp>
#include <Nova/platform/windows/Error.hpp>

Nova::WindowClass::WindowClass(const WNDCLASSEXA &info)
    : WindowClass(RegisterClassExA(&info), info.hInstance)
{
    if (class_ == 0)
        throw Win32::Exception("Failed to register window class");
}

Nova::WindowClass::WindowClass(const WNDCLASSEXW &info)
    : WindowClass(RegisterClassExW(&info), info.hInstance)
{
    if (class_ == 0)
        throw Win32::Exception("Failed to register window class");
}

Nova::WindowClass::~WindowClass() noexcept
{
    if (class_ != 0)
        UnregisterClass(MAKEINTATOM(class_), hInstance_);
}
