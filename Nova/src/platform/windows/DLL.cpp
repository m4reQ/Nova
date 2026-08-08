#include <Nova/platform/windows/DLL.hpp>

static HMODULE sModule;

HMODULE Nova::DLL::GetHandle() noexcept
{
    return sModule;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD, LPVOID)
{
    sModule = module;

    return TRUE;
}