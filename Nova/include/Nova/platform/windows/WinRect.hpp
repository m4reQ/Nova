#pragma once
#include <utility>
#include <Windows.h>

namespace Nova
{
    struct WinRect
    {
        RECT rect;

        static WinRect Client(HWND window);

        static WinRect Window(HWND window);

        constexpr WinRect(const RECT &rect_) noexcept
            : rect(rect_) {}

        constexpr int GetWidth() const noexcept { return rect.right - rect.left; }

        constexpr int GetHeight() const noexcept { return rect.bottom - rect.top; }

        constexpr std::pair<int, int> GetSize() const noexcept { return std::make_pair(GetWidth(), GetHeight()); }

        constexpr int GetX() const noexcept { return rect.left; }

        constexpr int GetY() const noexcept { return rect.right; }

        constexpr std::pair<int, int> GetPosition() const noexcept { return std::make_pair(GetX(), GetY()); }
    };
}