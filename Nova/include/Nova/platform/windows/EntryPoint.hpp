#include <Nova/core/Application.hpp>
#include <Nova/platform/windows/StartupData.hpp>
#include <Nova/platform/windows/DLL.hpp>
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

extern std::unique_ptr<Nova::Application> CreateApplication(const Nova::StartupData &startupData);

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
    try
    {
        const Nova::StartupData startupData{
            .exeInstance = GetModuleHandle(nullptr),
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(argc, argv),
            .showCommand = SW_SHOW,
        };
        auto application = CreateApplication(startupData);
        application->Run();
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdLine, int cmdShow)
{
    try
    {
        const Nova::StartupData startupData{
            .exeInstance = hInst,
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(__argc, __argv),
            .showCommand = cmdShow,
        };
        auto application = CreateApplication(startupData);
        application->Run();
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif