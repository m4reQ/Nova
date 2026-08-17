#include <Nova/core/Application.hpp>
#include <Nova/core/System.hpp>
#include <Nova/assets/Assets.hpp>
#include <Nova/debug/Profile.hpp>

using FrameClock = std::chrono::high_resolution_clock;

template <class T>
static constexpr double GetDurationSeconds(const T &start, const T &end) noexcept
{
    return std::chrono::duration<double, std::ratio<1>>(end - start).count();
}

Nova::Application::Application(const std::string_view name, const StartupData &startupData, const Nova::WindowSettings &windowSettings)
    : name_(name),
      appStart_(FrameClock::now()),
      window_(
          windowSettings,
          startupData,
          eventSystem_,
          inputSystem_),
      renderer_(window_) {}

void Nova::Application::Run()
{
    {
        NV_PROFILE_SCOPE("::Load");
        OnLoad();
    }

    while (!window_.ShouldClose())
    {
        NV_PROFILE_SCOPE("::ProcessFrame");
        NV_PROFILE_COUNTER("Memory (MB)", static_cast<double>(System::GetProcMemoryInfo().VirtualMemoryUsed) / 1000000.0);
        NV_PROFILE_COUNTER("Frametime (s)", GetFrametime());

        const auto frameStart = FrameClock::now();

        // TODO Convert asset system to an object
        Nova::Assets::ProcessLoadingTasks_();
        window_.ProcessEvents();
        eventSystem_.ProcessEventQueue(
            [&](const Event &event)
            {
                OnEvent(event);
            });

        {
            NV_PROFILE_SCOPE("::Update");
            OnUpdate();
        }

        {
            NV_PROFILE_SCOPE("::Render");
            OnRender();
        }

        window_.SwapBuffers();

        frametime_ = GetDurationSeconds(frameStart, FrameClock::now());
    }

    OnClose();
}

double Nova::Application::GetTime() const noexcept
{
    return GetDurationSeconds(appStart_, FrameClock::now());
}
