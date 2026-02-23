#pragma once
#include <string>
#include <filesystem>

namespace Nova
{
    struct DotnetSettings
    {
        const std::string Version = "9.0";
        const std::filesystem::path RuntimeConfigPath = "Nova.runtimeconfig.json";
    };
}