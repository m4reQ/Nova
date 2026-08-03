#pragma once
#include <Nova/input/Key.hpp>
#include <Nova/events/Event.hpp>

namespace Nova
{
    class KeyDownEvent final : public Event
    {
    public:
        KeyDownEvent() = default;

        constexpr KeyDownEvent(Key key, bool isRepeated) noexcept
            : key_(key),
              isRepeated_(isRepeated) {}

        EventType GetEventType() const noexcept override { return EventType::KeyDown; }

        const std::string_view GetName() const noexcept override { return "KeyDown"; }

        constexpr Key GetKey() const noexcept { return key_; }

        constexpr bool IsRepeated() const noexcept { return isRepeated_; }

    private:
        Key key_;
        bool isRepeated_;
    };
}
