#include <Nova/platform/windows/WinRect.hpp>

using namespace Nova;

WinRect WinRect::Client(HWND window)
{
    RECT rect;
    GetClientRect(window, &rect);

    return FromRect(rect);
}

WinRect WinRect::Window(HWND window)
{
    RECT rect;
    GetWindowRect(window, &rect);

    return FromRect(rect);
}

WinRect WinRect::FromRect(const RECT &rect)
{
    return WinRect{
        .X = rect.left,
        .Y = rect.top,
        .Width = rect.right - rect.left,
        .Height = rect.bottom - rect.top,
    };
}