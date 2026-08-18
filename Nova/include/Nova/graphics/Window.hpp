#pragma once
#include <Nova/graphics/WindowData.hpp>
#include <Nova/graphics/FullscreenMode.hpp>
#include <Nova/graphics/Rect.hpp>
#include <Nova/core/StartupData.hpp>
#include <Nova/core/Flag.hpp>
#include <Nova/events/EventSystem.hpp>
#include <Nova/input/InputSystem.hpp>
#include <memory>
#include <span>
#include <optional>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace Nova
{
    enum WindowFlags
    {
        Default = 0,
        Resizable = (1 << 0),
        Borderless = (1 << 1),
        Transparent = (1 << 2),
        Vsync = (1 << 3),
        CaptureCursor = (1 << 4),
        StartMaximized = (1 << 5),
        StartMinimized = (1 << 6),
        AllowFileDrop = (1 << 7),
    };

    NV_DEFINE_BITWISE_OPERATORS(WindowFlags);

    struct WindowSettings
    {
        int Width;
        int Height;
        std::string Title;
        WindowFlags Flags = WindowFlags::Default;
        FullscreenMode FullscreenMode = FullscreenMode::Windowed;
        const std::optional<std::filesystem::path> IconFilepath = std::nullopt;
        const std::optional<std::filesystem::path> CursorFilepath = std::nullopt;
    };

    class Window
    {
    public:
        Window() = default;

        Window(const Window &) = delete;

        Window(Window &&) noexcept = default;

        Window(
            const WindowSettings &settings,
            const StartupData &startupData,
            EventSystem &eventSystem,
            InputSystem &inputSystem);

        void *GetNativeHandle() noexcept;

        const void *GetNativeHandle() const noexcept;

        constexpr WindowData &GetData() noexcept { return data_; }

        constexpr const WindowData &GetData() const noexcept { return data_; }

        int GetWidth() const noexcept;

        int GetHeight() const noexcept;

        std::pair<int, int> GetSize() const noexcept;

        Nova::Rect<unsigned int> GetViewport() const noexcept;

        float GetAspectRatio() const noexcept;

        int GetClientWidth() const noexcept;

        int GetClientHeight() const noexcept;

        float GetClientAspectRatio() const noexcept;

        std::pair<int, int> GetClientSize() const noexcept;

        Nova::Rect<unsigned int> GetClientViewport() const noexcept;

        int GetX() const noexcept;

        int GetY() const noexcept;

        std::pair<int, int> GetPosition() const noexcept;

        std::string GetTitle() const noexcept;

        bool ShouldClose() const noexcept;

        FullscreenMode GetFullscreenMode() const noexcept;

        bool IsFullscreen() const noexcept;

        bool IsWindowed() const noexcept;

        bool IsVisible() const noexcept;

        void Resize(int width, int height) noexcept;

        void Move(int x, int y) noexcept;

        void Close() noexcept;

        void SetTitle(const std::string_view title) noexcept;

        void SetTitle(const std::wstring_view title) noexcept;

        void Maximize() noexcept;

        void Minimize() noexcept;

        void SwapBuffers() noexcept;

        void ProcessEvents();

        void SetIcon(const std::filesystem::path &filepath);

        void SetIcon(int width, int height, std::span<const std::byte> data);

        void SetCursor(const std::filesystem::path &filepath);

        void SetCursorCaptured(bool captured) noexcept;

        void SetFullscreen(FullscreenMode mode) noexcept;

        Window &operator=(const Window &) = delete;

        Window &operator=(Window &&) noexcept = default;

    private:
        EventSystem &eventSystem_;
        InputSystem &inputSystem_;
        WindowData data_;

        static LRESULT CALLBACK WindowProc(HWND win, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

        LRESULT HandleMessage(HWND win, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    };
}