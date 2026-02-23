#pragma once
#include <Windows.h>
#include <span>

namespace Nova
{
    struct StartupData
    {
        constexpr StartupData(HINSTANCE exeInstance, HINSTANCE libInstance, const char** args, size_t argsCount) noexcept
            : ExeInstance(exeInstance),
              LibInstance(libInstance),
              Args(args, argsCount) { }

        HINSTANCE ExeInstance;
        HINSTANCE LibInstance;
        std::span<const char*> Args;
    };
}