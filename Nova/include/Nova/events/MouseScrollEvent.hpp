#pragma once
#include <Nova/events/Event.hpp>

namespace Nova
{
    class MouseScrollEvent final : public Event
    {
    public:
        MouseScrollEvent() = default;

        constexpr MouseScrollEvent(double v, double h) noexcept
            : v_(v), h_(h) {}

        EventType GetEventType() const noexcept override { return EventType::MouseScroll; }

        const std::string_view GetName() const noexcept override { return "MouseScroll"; }

        constexpr double GetVertical() const noexcept { return v_; }

        constexpr double GetHorizontal() const noexcept { return h_; }

    private:
        double v_;
        double h_;
    };
}
