#pragma once
#include <Nova/events/Event.hpp>

namespace Nova
{
    class WindowResizeEvent final : public Event
    {
    public:
        WindowResizeEvent() = default;

        constexpr WindowResizeEvent(unsigned int width, unsigned int height) noexcept
            : width_(width), height_(height) {}

        EventType GetEventType() const noexcept override { return EventType::WindowResize; }

        const std::string_view GetName() const noexcept override { return "WindowResize"; }

        constexpr unsigned int GetWidth() const noexcept { return width_; }

        constexpr unsigned int GetHeight() const noexcept { return height_; }

    private:
        unsigned int width_;
        unsigned int height_;
    };
}