#pragma once
#include <Nova/utils/AutoRelease.hpp>
#include <Windows.h>

namespace Nova
{
    struct ProcessMemoryInfo
    {
        size_t virtualUsedBytes;
        size_t physicalUsedBytes;
    };

    class Process
    {
    public:
        /// @brief Returns the current process.
        Process();

        /// @brief Constructs process object from a given process handle.
        Process(HANDLE handle) noexcept;

        Process(const Process &) = delete;

        Process(Process &&) noexcept = default;

        unsigned int GetID() const;

        ProcessMemoryInfo GetMemoryInfo() const;

        constexpr void *GetHandle() const noexcept { return handle_.Get(); }

        Process &operator=(const Process &) = delete;

        Process &operator=(Process &&) noexcept = default;

    private:
        AutoRelease<HANDLE> handle_;
    };
}