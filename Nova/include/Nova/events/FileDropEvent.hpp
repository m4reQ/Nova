#pragma once
#include <Nova/events/Event.hpp>
#include <filesystem>
#include <span>
#include <vector>

namespace Nova
{
    class FileDropEvent final : public Event
    {
    public:
        FileDropEvent() = default;

        FileDropEvent(std::span<const std::filesystem::path> paths) noexcept
            : paths_(paths.begin(), paths.end()) {}

        FileDropEvent(std::vector<std::filesystem::path> &&paths) noexcept
            : paths_(std::move(paths)) {}

        EventType GetEventType() const noexcept override { return EventType::FileDrop; }

        const std::string_view GetName() const noexcept override { return "FileDrop"; }

        constexpr size_t GetPathsCount() const noexcept { return paths_.size(); }

        constexpr const std::vector<std::filesystem::path> &GetPaths() const noexcept { return paths_; }

        constexpr const std::filesystem::path &GetPath(size_t index) const noexcept { return paths_[index]; }

    private:
        std::vector<std::filesystem::path> paths_;
    };
}
