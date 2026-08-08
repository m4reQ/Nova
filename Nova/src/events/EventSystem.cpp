#include <Nova/events/EventSystem.hpp>
#include <Nova/debug/Profile.hpp>

void Nova::EventSystem::InvokeEvent(std::unique_ptr<Event> &&event)
{
    NV_PROFILE_FUNC;

    std::scoped_lock lock(queueMutex_);
    queue_.emplace(std::move(event));
}

void Nova::EventSystem::InvokeEventImmediate(std::unique_ptr<Event> &&event, EventCallback eventCallback)
{
    NV_PROFILE_FUNC;

    eventCallback(*event);
}

void Nova::EventSystem::ProcessEventQueue(EventCallback eventCallback)
{
    NV_PROFILE_FUNC;

    while (HasPendingEvents())
    {
        std::unique_ptr<Event> event;

        // TODO Support reentrancy without taking lock for each event
        {
            std::scoped_lock lock(queueMutex_);
            event = std::move(queue_.front());
            queue_.pop();
        }

        eventCallback(*event);
    }
}

void Nova::EventSystem::ClearEventQueue()
{
    NV_PROFILE_FUNC;

    std::scoped_lock lock(queueMutex_);

    while (HasPendingEvents())
        queue_.pop();
}

bool Nova::EventSystem::HasPendingEvents() const noexcept
{
    return !queue_.empty();
}
