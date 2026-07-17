#pragma once
#include <utility>
#include <Windows.h>

namespace Nova
{
    class DeviceContext
    {
    public:
        DeviceContext() = default;

        constexpr DeviceContext(HDC dc, bool freeContext = false)
            : context_(dc),
              freeContext_(freeContext) {}

        DeviceContext(HWND window);

        DeviceContext(const DeviceContext &) = delete;

        ~DeviceContext() noexcept;

        DeviceContext &operator=(const DeviceContext &) = delete;

        constexpr HDC Get() const noexcept { return context_; }

        constexpr HDC Reset() noexcept { return std::exchange(context_, nullptr); }

        constexpr operator HDC() const noexcept { return Get(); }

    private:
        HDC context_ = nullptr;
        bool freeContext_ = false;
    };
}