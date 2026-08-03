#pragma once
#include <Nova/input/Key.hpp>
#include <Nova/events/Event.hpp>

namespace Nova
{
    class KeyUpEvent final : public Event
    {
    public:
        KeyUpEvent() = default;

        constexpr KeyUpEvent(Key key) noexcept
            : key_(key) {}

        EventType GetEventType() const noexcept override { return EventType::KeyUp; }

        const std::string_view GetName() const noexcept override { return "KeyUp"; }

        constexpr Key GetKey() const noexcept { return key_; }

    private:
        Key key_;
    };
}
