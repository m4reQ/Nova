#pragma once
#include <Nova/events/Event.hpp>
#include <glm/vec2.hpp>

namespace Nova
{
    class MouseMoveEvent final : public Event
    {
    public:
        MouseMoveEvent() = default;

        constexpr MouseMoveEvent(double x, double y, double deltaX, double deltaY) noexcept
            : x_(x), y_(y), deltaX_(deltaX), deltaY_(deltaY) {}

        EventType GetEventType() const noexcept override { return EventType::MouseMove; }

        const std::string_view GetName() const noexcept override { return "MouseMove"; }

        constexpr double GetX() const noexcept { return x_; }

        constexpr double GetY() const noexcept { return y_; }

        constexpr glm::dvec2 GetPosition() const noexcept { return glm::dvec2{x_, y_}; }

        constexpr double GetDeltaX() const noexcept { return deltaX_; }

        constexpr double GetDeltaY() const noexcept { return deltaX_; }

        constexpr glm::dvec2 GetDelta() const noexcept { return glm::dvec2{deltaX_, deltaY_}; }

    private:
        double x_;
        double y_;
        double deltaX_;
        double deltaY_;
    };
}
