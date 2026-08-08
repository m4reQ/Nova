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

        WindowClass(WindowClass &&other) noexcept
            : class_(std::exchange(other.class_, static_cast<unsigned short>(0))),
              hInstance_(std::exchange(other.hInstance_, nullptr)) {}

        WindowClass &operator=(const WindowClass &) = delete;

        constexpr WindowClass &operator=(WindowClass &&other) noexcept
        {
            class_ = std::exchange(other.class_, static_cast<unsigned short>(0));
            hInstance_ = std::exchange(other.hInstance_, nullptr);

            return *this;
        }

        ~WindowClass() noexcept;

        constexpr ATOM Get() const noexcept { return class_; }

        constexpr HINSTANCE GetInstance() const noexcept { return hInstance_; }

        constexpr ATOM Reset() noexcept { return std::exchange(class_, static_cast<unsigned short>(0)); }

        constexpr operator ATOM() const noexcept { return Get(); }

    private:
        ATOM class_ = 0;
        HINSTANCE hInstance_ = nullptr;
    };
}