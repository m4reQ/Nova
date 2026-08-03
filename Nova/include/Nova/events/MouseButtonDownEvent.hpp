#pragma once
#include <Nova/input/Button.hpp>
#include <Nova/events/Event.hpp>

namespace Nova
{
    class MouseButtonDownEvent final : public Event
    {
    public:
        MouseButtonDownEvent() = default;

        constexpr MouseButtonDownEvent(Button button) noexcept
            : button_(button) {}

        EventType GetEventType() const noexcept override { return EventType::MouseButtonDown; }

        const std::string_view GetName() const noexcept override { return "MouseButtonDown"; }

        constexpr Button GetButton() const noexcept { return button_; }

    private:
        Button button_;
    };
}
