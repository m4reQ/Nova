#pragma once
#include <Nova/graphics/Window.hpp>
#include <Nova/graphics/Renderer.hpp>
#include <Nova/events/EventSystem.hpp>
#include <Nova/events/Event.hpp>
#include <Nova/input/InputSystem.hpp>
#include <chrono>

namespace Nova
{
    class Application
    {
    public:
        Application(const std::string_view name, const StartupData &startupData, const WindowSettings &windowSettings);

        Application(const Application &) = delete;

        Application(Application &&) noexcept = default;

        virtual ~Application() noexcept = default;

        void Run();

        virtual void OnRender() = 0;

        virtual void OnUpdate() = 0;

        virtual void OnEvent(const Event &event) = 0;

        constexpr Window &GetWindow() noexcept { return window_; }

        constexpr const Window &GetWindow() const noexcept { return window_; }

        constexpr EventSystem &GetEventSystem() noexcept { return eventSystem_; }

        constexpr const EventSystem &GetEventSystem() const noexcept { return eventSystem_; }

        constexpr const InputSystem &GetInputSystem() const noexcept { return inputSystem_; }

        constexpr InputSystem &GetInputSystem() noexcept { return inputSystem_; }

        constexpr Renderer &GetRenderer() noexcept { return renderer_; }

        constexpr const Renderer &GetRenderer() const noexcept { return renderer_; }

        constexpr const std::string_view GetName() const noexcept { return name_; }

        constexpr double GetFrametime() const noexcept { return frametime_; }

        constexpr double GetFPS() const noexcept { return 1.0 / frametime_; }

        double GetTime() const noexcept;

        Application &operator=(const Application &) = delete;

        Application &operator=(Application &&) noexcept = default;

    private:
        EventSystem eventSystem_;
        InputSystem inputSystem_;
        Window window_;
        Renderer renderer_;
        std::string name_;
        std::chrono::high_resolution_clock::time_point appStart_;
        double frametime_ = 0.001;
    };

    template <typename T, typename... Args>
        requires std::is_base_of_v<Application, T>
    void Run(Args &&...args)
    {
        auto application = std::make_unique<T>(std::forward<Args>(args)...);
        application->Run();
    }
}