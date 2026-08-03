#include <Nova/platform/windows/Process.hpp>
#include <Nova/debug/Profile.hpp>
#include <Psapi.h>
#include <TlHelp32.h>
#include <stdexcept>

static HANDLE GetCurrentProcessHandle()
{
    NV_PROFILE_FUNC;

    auto pseudo = GetCurrentProcess();
    HANDLE handle{};
    if (!DuplicateHandle(
            pseudo,
            pseudo,
            pseudo,
            &handle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS))
        throw std::runtime_error("Failed to get current process handle.");

    return handle;
}

Nova::Process::Process()
    : Process(GetCurrentProcessHandle())
{
}

Nova::Process::Process(HANDLE handle) noexcept
    : handle_(
          handle,
          [](auto x)
          {
              if (x != reinterpret_cast<HANDLE>(-1))
                  CloseHandle(x);
          })
{
}

unsigned int Nova::Process::GetID() const
{
    NV_PROFILE_FUNC;

    return GetProcessId(handle_.Get());
}

Nova::ProcessMemoryInfo Nova::Process::GetMemoryInfo() const
{
    NV_PROFILE_FUNC;

    PROCESS_MEMORY_COUNTERS_EX info{};
    if (GetProcessMemoryInfo(
            handle_.Get(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&info),
            sizeof(PROCESS_MEMORY_COUNTERS_EX)) == 0)
        throw std::runtime_error("Failed to get process memory info.");

    return {
        .virtualUsedBytes = info.PrivateUsage,
        .physicalUsedBytes = info.WorkingSetSize,
    };
}
