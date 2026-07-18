#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <filesystem>
#include <span>
#include <memory>
#include <Windows.h>

namespace Nova
{
    class Icon
    {
    public:
        Icon() = default;

        Icon(Icon &&) noexcept = default;

        Icon(HICON icon) noexcept;

        Icon(ICONINFO &info);

        Icon(const std::filesystem::path &filepath, HINSTANCE instance);

        Icon(int width, int height, std::span<const std::byte> data);

        constexpr HICON Get() const noexcept { return icon_.Get(); }

        constexpr HICON Reset() noexcept { return icon_.Reset(); }

        constexpr operator HICON() const noexcept { return Get(); }

        Icon &operator=(Icon &&) noexcept = default;

    private:
        AutoRelease<HICON> icon_;
    };
}