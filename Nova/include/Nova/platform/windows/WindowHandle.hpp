#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <string_view>
#include <Windows.h>

namespace Nova
{
    class WindowHandle
    {
    public:
        WindowHandle() = default;

        WindowHandle(WindowHandle &&) noexcept = default;

        WindowHandle(HWND handle) noexcept;

        WindowHandle(
            DWORD exStyle,
            const std::string_view className,
            const std::string_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance,
            void *userData);

        WindowHandle(
            DWORD exStyle,
            ATOM wndClass,
            const std::string_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance,
            void *userData);

        WindowHandle(
            DWORD exStyle,
            const std::wstring_view className,
            const std::wstring_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance,
            void *userData);

        WindowHandle(
            DWORD exStyle,
            ATOM wndClass,
            const std::wstring_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance,
            void *userData);

        WindowHandle &operator=(WindowHandle &&) noexcept = default;

        constexpr HWND Get() const noexcept { return handle_.Get(); }

        constexpr HWND Reset() noexcept { return handle_.Reset(); }

        constexpr operator HWND() const noexcept { return Get(); }

    private:
        AutoRelease<HWND> handle_;
    };
}