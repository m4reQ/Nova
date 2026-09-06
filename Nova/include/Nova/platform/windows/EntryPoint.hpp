#include <Nova/core/Application.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/platform/windows/StartupData.hpp>
#include <Nova/platform/windows/DLL.hpp>
#include <Nova/platform/windows/Error.hpp>
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

extern void RunApplication(const Nova::StartupData &startupData);

static std::vector<std::string> GetArgs(int argc, char **argv) noexcept
{
    std::vector<std::string> args;
    args.reserve(argc);

    for (auto i = 0; i < argc; i++)
        args.emplace_back(argv[i]);

    return args;
}

#ifdef NV_CONSOLE
int main(int argc, char **argv)
{
    // TODO Move this out of main
    NV_LOG_INITIALIZE("./NovaLog.txt");
    NV_LOG_INFO("Using working directory \"{}\".", std::filesystem::current_path().string());
    NV_PROFILE_SET_ENABLED(true);
    NV_PROFILE_BEGIN_SESSION("./NovaProfileSession.json");
    NV_PROFILE_SET_CURRENT_THREAD_NAME("MainThread");
    NV_PROFILE_SET_CURRENT_THREAD_INDEX(0);

#ifndef NV_DEBUG
    try
    {
#endif
        const Nova::StartupData startupData{
            .exeInstance = GetModuleHandleA(nullptr),
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(argc, argv),
            .showCommand = SW_SHOWDEFAULT,
        };
        RunApplication(startupData);
#ifndef NV_DEBUG
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
#endif

    return 0;
}
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdLine, int cmdShow)
{
    // TODO Move this out of main
    NV_LOG_INITIALIZE("./NovaLog.txt");
    NV_LOG_INFO("Using working directory \"{}\".", std::filesystem::current_path().string());
    NV_PROFILE_SET_ENABLED(true);
    NV_PROFILE_BEGIN_SESSION("./NovaProfileSession.json");
    NV_PROFILE_SET_CURRENT_THREAD_NAME("MainThread");
    NV_PROFILE_SET_CURRENT_THREAD_INDEX(0);

#ifndef NV_DEBUG
    try
    {
#endif
        const Nova::StartupData startupData{
            .exeInstance = hInst,
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(__argc, __argv),
            .showCommand = cmdShow,
        };
        RunApplication(startupData);
#ifndef NV_DEBUG
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
#endif

    return 0;
}
#endif