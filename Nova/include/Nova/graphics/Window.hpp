#pragma once
#include <Nova/core/Build.hpp>
#include <string_view>
#include <optional>
#include <utility>
#include <filesystem>
#include <cstdint>
#include <glfw/glfw3.h>
#include <glad/gl.h>

namespace Nova
{
    enum class FullscreenMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };

    enum WindowFlags
    {
        Default = 0,
        Resizable = 1,
        Borderless = 2,
        Transparent = 4,
        Vsync = 8,
        CaptureCursor = 16,
        StartMaximized = 32,
    };

    struct WindowSettings
    {
        int Width;
        int Height;
        const std::string_view Title;
        WindowFlags Flags = WindowFlags::Default;
        FullscreenMode FullscreenMode = FullscreenMode::Windowed;
        const std::optional<std::filesystem::path> IconFilepath = std::nullopt;
        const std::optional<std::filesystem::path> CursorFilepath = std::nullopt;
    };

    namespace Window
    {
        NV_API void Resize(std::int32_t width, std::int32_t height) noexcept;

        NV_API void Move(std::int32_t x, std::int32_t y) noexcept;

        NV_API void SetTitle(const std::string_view title) noexcept;

        NV_API void SetTitle(const std::wstring_view title) noexcept;

        NV_API void Close() noexcept;

        NV_API void SetIcon(std::int32_t width, std::int32_t height, std::uint8_t* data);

        NV_API void SetIcon(const std::filesystem::path &filepath);

        NV_API void SetCursor(std::int32_t width, std::int32_t height, std::uint8_t* data, std::int32_t xHot = 0, std::int32_t yHot = 0);

        NV_API void SetCursor(const std::filesystem::path &filepath, std::int32_t xHot = 0, std::int32_t yHot = 0);

        NV_API void SetFullscreen(FullscreenMode mode) noexcept;

        NV_API void Maximize();

        NV_API void Iconfiy();

        NV_API std::pair<int, int> GetSize() noexcept;

        NV_API void GetSize(int &width, int &height) noexcept;

        NV_API int GetWidth() noexcept;

        NV_API int GetHeight() noexcept;

        NV_API float GetAspectRatio() noexcept;

        NV_API std::pair<int, int> GetFramebufferSize() noexcept;

        NV_API void GetFramebufferSize(int &width, int &height) noexcept;

        NV_API int GetFramebufferWidth() noexcept;

        NV_API int GetFramebufferHeight() noexcept;

        NV_API float GetFramebufferAspectRatio() noexcept;

        NV_API std::pair<int, int> GetPosition() noexcept;

        NV_API void GetPosition(int &x, int &y) noexcept;

        NV_API bool IsVisible() noexcept;

        NV_API bool IsFullscreen() noexcept;

        NV_API bool ShouldClose() noexcept;

        NV_API GLFWwindow *GetNativeHandle() noexcept;

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        void Initialize_(const WindowSettings &settings);

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        void Shutdown_() noexcept;

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        void Update_() noexcept;

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        void SwapBuffers_() noexcept;

        /// <summary>
        /// Private API. Don't use directly!
        /// </summary>
        GLADloadfunc GetLoaderFunc_() noexcept;
    }

    constexpr WindowFlags operator&(WindowFlags a, WindowFlags b) noexcept
    {
        return (WindowFlags)((std::underlying_type_t<WindowFlags>)a & (std::underlying_type_t<WindowFlags>)b);
    }

    constexpr WindowFlags operator|(WindowFlags a, WindowFlags b) noexcept
    {
        return (WindowFlags)((std::underlying_type_t<WindowFlags>)a | (std::underlying_type_t<WindowFlags>)b);
    }

    constexpr WindowFlags operator^(WindowFlags a, WindowFlags b) noexcept
    {
        return (WindowFlags)((std::underlying_type_t<WindowFlags>)a ^ (std::underlying_type_t<WindowFlags>)b);
    }
}
