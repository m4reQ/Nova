#include <Nova/platform/windows/StartupData.hpp>
#include <Nova/platform/windows/DLL.hpp>
#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>

extern void Run(const Nova::StartupData &args);

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
        Run({
            .exeInstance = GetModuleHandle(nullptr),
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(argc, argv),
            .showCommand = SW_SHOW,
        });
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
        Run({
            .exeInstance = hInst,
            .libInstance = Nova::DLL::GetHandle(),
            .args = GetArgs(__argc, __argv),
            .showCommand = cmdShow,
        });
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif