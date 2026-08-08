#pragma once
#include <string>
#include <stdexcept>

namespace Nova::Win32
{
    std::string GetLastError();

    class Exception final : public std::exception
    {
    public:
        Exception() = delete;

        Exception(const std::string_view message);
    };
}