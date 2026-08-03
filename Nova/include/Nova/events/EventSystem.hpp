#pragma once
#include <Nova/events/Event.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>
#include <functional>

namespace Nova
{
    using EventCallback = std::function<void(const Event &event)>;

    class EventSystem
    {
    public:
        EventSystem() = default;

        EventSystem(EventSystem &&) noexcept = default;

        EventSystem(const EventSystem &) = delete;

        EventSystem &operator=(EventSystem &&) noexcept = default;

        EventSystem &operator=(const EventSystem &) = delete;

        template <typename T, typename... Args>
            requires std::is_base_of_v<Event, T>
        void InvokeEvent(Args &&...args)
        {
            InvokeEvent(std::make_unique<T>(std::forward<Args>(args)...));
        }

        void InvokeEvent(std::unique_ptr<Event> &&event);

        template <typename T, typename... Args>
            requires std::is_base_of_v<Event, T>
        void InvokeEventImmediate(Args &&...args, EventCallback eventCallback)
        {
            InvokeEventImmediate(std::make_unique<T>(std::forward<Args>(args)...), eventCallback);
        }

        void InvokeEventImmediate(std::unique_ptr<Event> &&event, EventCallback eventCallback);

        void ProcessEventQueue(EventCallback eventCallback);

        void ClearEventQueue();

        bool HasPendingEvents() const noexcept;

    private:
        std::queue<std::unique_ptr<Event>> queue_;
        std::mutex queueMutex_;
    };
}