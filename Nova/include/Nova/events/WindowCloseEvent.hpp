#pragma once
#include <Nova/core/Input.hpp>
#include <Nova/events/Event.hpp>

namespace Nova
{
    class WindowCloseEvent final : public Event
    {
    public:
        WindowCloseEvent() = default;

        constexpr WindowCloseEvent(double time) noexcept
            : time_(time) {}

        EventType GetEventType() const noexcept override { return EventType::WindowClose; }

        const std::string_view GetName() const noexcept override { return "WindowClose"; }

        constexpr double GetTime() const noexcept { return time_; }

    private:
        double time_;
    };
}
