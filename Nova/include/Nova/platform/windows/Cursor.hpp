#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <Windows.h>
#include <filesystem>
#include <memory>
#include <span>

namespace Nova
{
    class Cursor
    {
    public:
        Cursor() = default;

        Cursor(HCURSOR handle) noexcept;

        Cursor(const std::filesystem::path &filepath, HINSTANCE instance);

        Cursor(Cursor &&) noexcept = default;

        constexpr HCURSOR Get() const noexcept { return handle_.Get(); }

        constexpr HCURSOR Reset() noexcept { return handle_.Reset(); }

        constexpr operator HCURSOR() const noexcept { return Get(); }

        Cursor &operator=(Cursor &&) noexcept = default;

    private:
        AutoRelease<HCURSOR> handle_;
    };
}