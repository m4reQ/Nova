#pragma once
#include <Nova/input/Button.hpp>
#include <Nova/events/Event.hpp>

namespace Nova
{
    class MouseButtonUpEvent final : public Event
    {
    public:
        MouseButtonUpEvent() = default;

        constexpr MouseButtonUpEvent(Button button) noexcept
            : button_(button) {}

        EventType GetEventType() const noexcept override { return EventType::MouseButtonUp; }

        const std::string_view GetName() const noexcept override { return "MouseButtonUp"; }

        constexpr Button GetButton() const noexcept { return button_; }

    private:
        Button button_;
    };
}
