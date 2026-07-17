#include <Nova/platform/windows/WindowClass.hpp>
#include <stdexcept>

Nova::WindowClass::WindowClass(const WNDCLASSEXA &info)
    : WindowClass(RegisterClassExA(&info), info.hInstance)
{
    if (class_ == 0)
        throw std::runtime_error("Failed to register window class.");
}

Nova::WindowClass::WindowClass(const WNDCLASSEXW &info)
    : WindowClass(RegisterClassExW(&info), info.hInstance)
{
    if (class_ == 0)
        throw std::runtime_error("Failed to register window class.");
}

Nova::WindowClass::~WindowClass() noexcept
{
    if (class_ != 0)
        UnregisterClass(reinterpret_cast<LPCTSTR>(class_), hInstance_);
}
