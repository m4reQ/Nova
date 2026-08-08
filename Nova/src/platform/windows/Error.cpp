#include <Nova/platform/windows/Error.hpp>
#include <Windows.h>
#include <array>
#include <format>

constexpr auto maxMessageBufferSize = 1024zu * 32zu; // 32 kB

static const char *StringToOwnedPointer(const std::string &str) noexcept
{
    auto ptr = new char[str.size()];
    strcpy(ptr, str.c_str());

    return ptr;
}

std::string Nova::Win32::GetLastError()
{
    const auto lastError = ::GetLastError();
    if (lastError == NO_ERROR)
        return {};

    std::array<char, maxMessageBufferSize> messageBuffer;
    const auto messageLength = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        lastError,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        messageBuffer.data(),
        maxMessageBufferSize,
        nullptr);

    if (messageLength == 0)
        return "<failed to get error message>";

    return std::format("({:#x}) {}", lastError, std::string_view(messageBuffer.data(), messageLength));
}

// NOTE This leaks the string
Nova::Win32::Exception::Exception(const std::string_view message)
    : std::exception(
          StringToOwnedPointer(
              std::format(
                  "{}: {}",
                  message,
                  GetLastError()))) {}
