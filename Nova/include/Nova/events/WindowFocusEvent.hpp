#pragma once
#include <Nova/events/Event.hpp>

namespace Nova
{
    class WindowFocusEvent final : public Event
    {
    public:
        WindowFocusEvent() = default;

        constexpr WindowFocusEvent(bool isFocused) noexcept
            : isFocused_(isFocused) {}

        EventType GetEventType() const noexcept override { return EventType::WindowFocus; }

        const std::string_view GetName() const noexcept override { return "WindowFocus"; }

        constexpr bool IsFocused() const noexcept { return isFocused_; }

    private:
        bool isFocused_;
    };
}
