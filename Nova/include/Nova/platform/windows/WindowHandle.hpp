#pragma once
#include <utility>
#include <string_view>
#include <Windows.h>

namespace Nova
{
    class WindowHandle
    {
    public:
        WindowHandle() = default;

        WindowHandle(const WindowHandle &) = delete;

        WindowHandle(WindowHandle &&) noexcept = default;

        constexpr WindowHandle(HWND handle) noexcept
            : handle_(handle) {}

        WindowHandle(
            DWORD exStyle,
            const std::string_view className,
            const std::string_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance);

        WindowHandle(
            DWORD exStyle,
            ATOM wndClass,
            const std::string_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance);

        WindowHandle(
            DWORD exStyle,
            const std::wstring_view className,
            const std::wstring_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance);

        WindowHandle(
            DWORD exStyle,
            ATOM wndClass,
            const std::wstring_view windowName,
            DWORD style,
            int x,
            int y,
            int width,
            int height,
            HINSTANCE hInstance);

        ~WindowHandle() noexcept;

        WindowHandle &operator=(const WindowHandle &) = delete;

        WindowHandle &operator=(WindowHandle &&) noexcept = default;

        constexpr HWND Get() const noexcept { return handle_; }

        constexpr HWND Reset() noexcept { return std::exchange(handle_, nullptr); }

        constexpr operator HWND() const noexcept { return Get(); }

    private:
        HWND handle_ = nullptr;
    };
}