#include <Nova/core/Application.hpp>

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
      renderer_() {}

void Nova::Application::Run()
{
    // NV_LOG_INITIALIZE("./NovaLog.txt");
    // NV_LOG_INFO("Using working directory \"{}\".", std::filesystem::current_path().string());
    // NV_PROFILE_SET_ENABLED(true);
    // NV_PROFILE_BEGIN_SESSION("./NovaProfileSession.json");
    // NV_PROFILE_SET_CURRENT_THREAD_NAME("MainThread");
    // NV_PROFILE_SET_CURRENT_THREAD_INDEX(0);

    OnLoad();

    while (!window_.ShouldClose())
    {
        const auto frameStart = FrameClock::now();

        window_.ProcessEvents();
        eventSystem_.ProcessEventQueue(
            [&](const Event &event)
            {
                OnEvent(event);
            });
        OnUpdate();
        OnRender();
        window_.SwapBuffers();

        frametime_ = GetDurationSeconds(frameStart, FrameClock::now());
    }

    OnClose();
}

double Nova::Application::GetTime() const noexcept
{
    return GetDurationSeconds(appStart_, FrameClock::now());
}
