#pragma once
#include <string>
#include <string_view>

namespace Nova::Unicode
{
    std::wstring MultibyteToWChar(const std::string_view str);
    std::string WCharToMultibyte(const std::wstring_view str);
}