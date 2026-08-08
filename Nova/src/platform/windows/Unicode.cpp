#include <Nova/utils/Unicode.hpp>
#include <Windows.h>
#include <stdexcept>

std::wstring Nova::Unicode::MultibyteToWChar(std::string_view str)
{
    if (str.empty())
        return {};

    const auto size = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0);
    if (size == 0)
        throw std::runtime_error("Invalid UTF-8 string");

    std::wstring result(size, L'\0');

    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            str.data(),
            static_cast<int>(str.size()),
            result.data(),
            size) == 0)
        throw std::runtime_error("UTF-8 to UTF-16 conversion failed");

    return result;
}

std::string Nova::Unicode::WCharToMultibyte(std::wstring_view str)
{
    if (str.empty())
        return {};

    const auto size = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (size == 0)
        throw std::runtime_error("Invalid UTF-16 string");

    std::string result(size, '\0');

    if (WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            str.data(),
            static_cast<int>(str.size()),
            result.data(),
            size,
            nullptr,
            nullptr) == 0)
        throw std::runtime_error("UTF-16 to UTF-8 conversion failed");

    return result;
}