#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <fstream>
#include <thread>
#include <mutex>

using namespace Nova;

constexpr const std::string_view c_ProfileCounterFormat = ",{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"args\":{{\"value\":{}}}}}";
constexpr const std::string_view c_ProfileFrameFormat = ",{{\"name\":\"{}\",\"ph\":\"X\",\"ts\":{},\"dur\":{},\"tid\":{}}}";
constexpr const std::string_view c_ProfileEventFormat = ",{{\"name\":\"{}\",\"ph\":\"i\",\"ts\":{},\"tid\":{},\"s\":\"g\"}}";
constexpr const std::string_view c_ProfileSessionHeader = "{\"traceEvents\":[{}";
constexpr const std::string_view c_ProfileSessionFooter = "]}";

static bool s_IsEnabled;
static std::mutex s_ProfileFileLock;
static std::ofstream s_ProfileFile;
static std::filesystem::path s_ProfileFilepath;
static ProfileTimer::time_point s_ProfileSessionStart;
static bool s_AtexitCallbackRegistered;

static double DurationToMicroseconds(ProfileTimer::duration dur)
{
    return std::chrono::duration<double, std::micro>(dur).count();
}

static void WriteToProfileFile(const std::string_view data)
{
    const std::lock_guard lock(s_ProfileFileLock);
    s_ProfileFile << data;
}

_ProfileFrame::~_ProfileFrame() noexcept
{
    Profile::_WriteProfileFrame(m_FunctionName, m_StartTimestamp);
}

void Profile::SetEnabled(bool isEnabled) noexcept
{
    s_IsEnabled = isEnabled;
}

void Profile::BeginSession(const std::filesystem::path &profileFilepath)
{
    if (!s_IsEnabled)
    {
        return;
    }

    if (!s_AtexitCallbackRegistered)
    {
        std::atexit(
            []()
            {
                EndSession();
            });
        s_AtexitCallbackRegistered = true;
    }

    if (s_ProfileFile.is_open())
    {
        NV_LOG_INFO("Profile session is already running on file: {}. Closing existing session...", s_ProfileFilepath.string());
        EndSession();
    }

    s_ProfileFilepath = profileFilepath;
    s_ProfileSessionStart = ProfileTimer::now();
    s_ProfileFile = std::ofstream(profileFilepath);

    if (!s_ProfileFile.is_open())
    {
        throw std::runtime_error("Couldn't open profile file.");
    }

    WriteToProfileFile(c_ProfileSessionHeader);
}

void Profile::EndSession() noexcept
{
    if (!s_IsEnabled)
    {
        return;
    }

    if (!IsSessionRunning())
    {
        NV_LOG_INFO("No profile session is currently running.");
        return;
    }

    WriteToProfileFile(c_ProfileSessionFooter);
    s_ProfileFile.close();
}

bool Profile::IsEnabled() noexcept
{
    return s_IsEnabled;
}

bool Profile::IsSessionRunning() noexcept
{
    return s_ProfileFile.is_open();
}

void Profile::_WriteProfileFrame(const std::string_view functionName, ProfileTimer::time_point startTs) noexcept
{
    if (!s_IsEnabled || !IsSessionRunning())
    {
        return;
    }

    const auto duration = ProfileTimer::now() - startTs;
    const auto startTsSessionOffset = startTs - s_ProfileSessionStart;

    WriteToProfileFile(
        std::format(
            c_ProfileFrameFormat,
            functionName,
            DurationToMicroseconds(startTsSessionOffset),
            DurationToMicroseconds(duration),
            std::this_thread::get_id()));
}

void Profile::_WriteProfileEvent(const std::string_view eventName) noexcept
{
    if (!s_IsEnabled || !IsSessionRunning())
    {
        return;
    }

    const auto nowSessionOffset = ProfileTimer::now() - s_ProfileSessionStart;

    WriteToProfileFile(
        std::format(
            c_ProfileEventFormat,
            eventName,
            DurationToMicroseconds(nowSessionOffset),
            std::this_thread::get_id()));
}

void Profile::_WriteProfileCounter(const std::string_view counterName, float value) noexcept
{
    if (!s_IsEnabled || !IsSessionRunning())
    {
        return;
    }

    const auto nowSessionOffset = ProfileTimer::now() - s_ProfileSessionStart;

    WriteToProfileFile(
        std::format(
            c_ProfileCounterFormat,
            counterName,
            DurationToMicroseconds(nowSessionOffset),
            value));
}
