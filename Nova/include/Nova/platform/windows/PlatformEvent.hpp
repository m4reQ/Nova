#pragma once
#include <Nova/events/Event.hpp>
#include <Windows.h>

namespace Nova
{
    class PlatformEvent final : public Event
    {
    public:
        constexpr PlatformEvent(
            HWND window,
            UINT message,
            WPARAM wParam,
            LPARAM lParam) noexcept
            : window_(window),
              message_(message),
              wParam_(wParam),
              lParam_(lParam) {}

        EventType GetEventType() const noexcept override { return EventType::PlatformEvent; }

        const std::string_view GetName() const noexcept override { return "Win32PlatformEvent"; }

        constexpr HWND GetWindow() const noexcept { return window_; }

        constexpr UINT GetMsg() const noexcept { return message_; }

        constexpr WPARAM GetWParam() const noexcept { return wParam_; }

        constexpr LPARAM GetLParam() const noexcept { return lParam_; }

    private:
        HWND window_;
        UINT message_;
        WPARAM wParam_;
        LPARAM lParam_;
    };
}