#pragma once
#include <string_view>

namespace Nova
{
    enum class EventType
    {
        WindowResize,
        WindowMove,
        WindowClose,
        WindowFocus,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseScroll,
        KeyDown,
        KeyUp,
        FileDrop,
        PlatformEvent,
    };

    class Event
    {
    public:
        virtual ~Event() noexcept = default;

        virtual EventType GetEventType() const noexcept = 0;

        virtual const std::string_view GetName() const noexcept = 0;
    };
}