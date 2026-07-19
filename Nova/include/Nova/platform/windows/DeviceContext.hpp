#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <utility>
#include <string_view>
#include <Windows.h>

namespace Nova
{
    class DeviceContext
    {
    public:
        DeviceContext() = default;

        DeviceContext(
            const std::wstring_view driver,
            const std::wstring_view device,
            const std::wstring_view port,
            const DEVMODEW &pdm);

        DeviceContext(
            const std::string_view driver,
            const std::string_view device,
            const std::string_view port,
            const DEVMODEA &pdm);

        /// @brief Wraps already created device context. The device context is destroyed upon object destruction.
        DeviceContext(HDC &&dc) noexcept;

        /// @brief Get device context for a given window. Automatically releases DC upon destruction.
        DeviceContext(HWND window);

        DeviceContext(const DeviceContext &) = delete;

        DeviceContext(DeviceContext &&) noexcept = default;

        DeviceContext &operator=(const DeviceContext &) = delete;

        DeviceContext &operator=(DeviceContext &&) noexcept = default;

        constexpr HDC Get() const noexcept { return handle_.Get(); }

        constexpr HDC Reset() noexcept { return handle_.Reset(); }

        constexpr operator HDC() const noexcept { return Get(); }

    private:
        AutoRelease<HDC> handle_;
    };
}