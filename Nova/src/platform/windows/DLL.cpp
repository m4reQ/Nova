#include <Nova/platform/windows/DLL.hpp>

static HMODULE sModule;

HMODULE Nova::DLL::GetHandle() noexcept
{
    return sModule;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    sModule = module;

    return TRUE;
}