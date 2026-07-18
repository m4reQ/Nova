#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace Nova
{
    struct StartupData
    {
        HINSTANCE exeInstance;
        HINSTANCE libInstance;
        std::vector<std::string> args;
        int showCommand;
    };
}