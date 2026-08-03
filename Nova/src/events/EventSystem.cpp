#include <Nova/events/EventSystem.hpp>

void Nova::EventSystem::InvokeEvent(std::unique_ptr<Event> &&event)
{
    std::scoped_lock(queueMutex_);
    queue_.emplace(std::move(event));
}

void Nova::EventSystem::InvokeEventImmediate(std::unique_ptr<Event> &&event, EventCallback eventCallback)
{
    eventCallback(*event);
}

void Nova::EventSystem::ProcessEventQueue(EventCallback eventCallback)
{
    std::scoped_lock(queueMutex_);

    while (HasPendingEvents())
    {
        const auto event = std::move(queue_.front());
        queue_.pop();

        eventCallback(*event);
    }
}

void Nova::EventSystem::ClearEventQueue()
{
    std::scoped_lock(queueMutex_);

    while (HasPendingEvents())
        queue_.pop();
}

bool Nova::EventSystem::HasPendingEvents() const noexcept
{
    return !queue_.empty();
}
