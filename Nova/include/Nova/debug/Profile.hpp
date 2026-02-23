#pragma once
#include <filesystem>
#include <string_view>
#include <chrono>

#ifdef NV_DEBUG
#define NV_PROFILE_SET_ENABLED(isEnabled) Nova::Profile::SetEnabled(isEnabled)
#define NV_PROFILE_FUNC const Nova::_ProfileFrame _profileFrame(__FUNCTION__)
#define NV_PROFILE_SCOPE(name) const Nova::_ProfileFrame _profileFrame(name)
#define NV_PROFILE_EVENT(name) Nova::Profile::_WriteProfileEvent(name)
#define NV_PROFILE_COUNTER(name, value) Nova::Profile::_WriteProfileCounter(name, value)
#define NV_PROFILE_BEGIN_SESSION(filepath) Nova::Profile::BeginSession(filepath)
#define NV_PROFILE_END_SESSION() Nova::Profile::EndSession()
#else
#define NV_PROFILE_SET_ENABLED(isEnabled)
#define NV_PROFILE_FUNC
#define NV_PROFILE_SCOPE(name)
#define NV_PROFILE_EVENT(name)
#define NV_PROFILE_COUNTER(name, value)
#define NV_PROFILE_BEGIN_SESSION(filepath)
#define NV_PROFILE_END_SESSION()
#endif

namespace Nova
{
    using ProfileTimer = std::chrono::high_resolution_clock;

    class _ProfileFrame
    {
    public:
        _ProfileFrame(const std::string_view functionName) noexcept
            : m_FunctionName(functionName), m_StartTimestamp(ProfileTimer::now()) {}

        ~_ProfileFrame() noexcept;

    private:
        const std::string_view m_FunctionName;
        const ProfileTimer::time_point m_StartTimestamp;
    };

    namespace Profile
    {
        void SetEnabled(bool isEnabled) noexcept;

        void BeginSession(const std::filesystem::path &profileFilepath);

        void EndSession() noexcept;

        bool IsEnabled() noexcept;

        bool IsSessionRunning() noexcept;

        void _WriteProfileFrame(const std::string_view functionName, ProfileTimer::time_point startTs) noexcept;

        void _WriteProfileEvent(const std::string_view eventName) noexcept;

        void _WriteProfileCounter(const std::string_view counterName, float value) noexcept;
    }
}
