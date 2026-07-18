#include <Nova/platform/windows/WinRect.hpp>

using namespace Nova;

WinRect WinRect::Client(HWND window)
{
    RECT rect{};
    GetClientRect(window, &rect);

    return WinRect(rect);
}

WinRect WinRect::Window(HWND window)
{
    RECT rect{};
    GetWindowRect(window, &rect);

    return WinRect(rect);
}
