#pragma once
#include <Nova/events/Event.hpp>

namespace Nova
{
    class WindowMoveEvent final : public Event
    {
    public:
        WindowMoveEvent() = default;

        constexpr WindowMoveEvent(unsigned int x, unsigned int y) noexcept
            : x_(x), y_(y) {}

        EventType GetEventType() const noexcept override { return EventType::WindowMove; }

        const std::string_view GetName() const noexcept override { return "WindowMove"; }

        constexpr unsigned int GetX() const noexcept { return x_; }

        constexpr unsigned int GetY() const noexcept { return y_; }

    private:
        unsigned int x_;
        unsigned int y_;
    };
}