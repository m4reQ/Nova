#pragma once
#include <utility>
#include <span>
#include <Windows.h>

namespace Nova
{
    class Bitmap
    {
    public:
        Bitmap() = default;

        Bitmap(const Bitmap &) = delete;

        Bitmap(Bitmap &&) noexcept = default;

        constexpr Bitmap(HBITMAP handle) noexcept
            : handle_(handle) {}

        Bitmap(int width, int height, UINT planes, UINT bitCount, std::span<const std::byte> bits);

        ~Bitmap() noexcept;

        constexpr HBITMAP Get() const noexcept { return handle_; }

        constexpr HBITMAP Reset() noexcept { return std::exchange(handle_, nullptr); }

        constexpr operator HBITMAP() const noexcept { return Get(); }

        constexpr Bitmap &operator=(const Bitmap &) = delete;

        constexpr Bitmap &operator=(Bitmap &&) noexcept = default;

    private:
        HBITMAP handle_ = nullptr;
    };
}