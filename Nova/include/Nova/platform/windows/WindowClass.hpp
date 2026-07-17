#pragma once
#include <utility>
#include <Windows.h>

namespace Nova
{
    class WindowClass
    {
    public:
        WindowClass() = default;

        constexpr WindowClass(ATOM windowClass, HINSTANCE hInstance) noexcept
            : class_(windowClass),
              hInstance_(hInstance) {}

        WindowClass(const WNDCLASSEXA &info);

        WindowClass(const WNDCLASSEXW &info);

        WindowClass(const WindowClass &) = delete;

        WindowClass(WindowClass &&) noexcept = default;

        WindowClass &operator=(const WindowClass &) = delete;

        WindowClass &operator=(WindowClass &&) noexcept = default;

        ~WindowClass() noexcept;

        constexpr ATOM Get() const noexcept { return class_; }

        constexpr HINSTANCE GetInstance() const noexcept { return hInstance_; }

        constexpr ATOM Reset() noexcept { return std::exchange(class_, 0); }

        constexpr operator ATOM() const noexcept { return Get(); }

    private:
        ATOM class_ = 0;
        HINSTANCE hInstance_ = nullptr;
    };
}