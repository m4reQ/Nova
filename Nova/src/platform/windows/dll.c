#include <Nova/core/loader.h>
#include <Nova/core/build.h>
#include <Windows.h>

static HINSTANCE s_LibraryInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
        s_LibraryInstance = hinstDLL;

    return TRUE;
}

void *NvLoaderGetLibraryInstance(void)
{
    NV_ASSERT(s_LibraryInstance, "Nova DLL instance was not acquired");
    return s_LibraryInstance;
}
