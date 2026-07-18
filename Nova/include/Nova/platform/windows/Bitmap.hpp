#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <span>
#include <Windows.h>

namespace Nova
{
    class Bitmap
    {
    public:
        Bitmap() = default;

        Bitmap(Bitmap &&) noexcept = default;

        Bitmap(HBITMAP handle) noexcept;

        Bitmap(int width, int height, UINT planes, UINT bitCount, std::span<const std::byte> bits);

        constexpr HBITMAP Get() const noexcept { return handle_.Get(); }

        constexpr HBITMAP Reset() noexcept { return handle_.Reset(); }

        constexpr operator HBITMAP() const noexcept { return Get(); }

        constexpr Bitmap &operator=(Bitmap &&) noexcept = default;

    private:
        AutoRelease<HBITMAP> handle_;
    };
}