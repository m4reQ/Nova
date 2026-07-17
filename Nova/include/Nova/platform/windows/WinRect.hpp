#pragma once
#include <Windows.h>
#include <utility>

namespace Nova
{
    struct WinRect
    {
        int X, Y, Width, Height;

        static WinRect Client(HWND window);

        static WinRect Window(HWND window);

        static WinRect FromRect(const RECT &rect);

        constexpr std::pair<int, int> GetSize() const noexcept { return std::make_pair(Width, Height); }

        constexpr std::pair<int, int> GetPosition() const noexcept { return std::make_pair(X, Y); }
    };
}