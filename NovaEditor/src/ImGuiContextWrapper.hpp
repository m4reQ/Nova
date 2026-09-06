#pragma once
#include <Nova/graphics/Window.hpp>
#include <imgui.h>
#include <utility>

class ImGuiContextWrapper
{
public:
    ImGuiContextWrapper(const Nova::Window &window);

    ImGuiContextWrapper(const ImGuiContextWrapper &) = delete;

    constexpr ImGuiContextWrapper(ImGuiContextWrapper &&other) noexcept
        : context_(std::exchange(other.context_, nullptr)) {}

    ~ImGuiContextWrapper() noexcept;

    ImGuiContextWrapper &operator=(const ImGuiContextWrapper &) = delete;

    constexpr ImGuiContextWrapper &operator=(ImGuiContextWrapper &&other) noexcept
    {
        context_ = std::exchange(other.context_, nullptr);
        return *this;
    }

    constexpr ImGuiContext *Get() noexcept { return context_; }

private:
    ImGuiContext *context_;
};