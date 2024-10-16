#include <Nova/dotnet/host.h>
#include <Nova/dotnet/managed_functions.h>
#include <Nova/core/build.h>
#include <Nova/core/exception.h>
#include <dotnet/coreclr_delegates.h>
#include <dotnet/hostfxr.h>
#include <dotnet/error_codes.h>
#include <stdio.h>
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>

#define IS_INITIALIZED (s_HostfxrLibrary && s_HostfxrHandle)
#define ASSERT_INITIALIZED NV_ASSERT(IS_INITIALIZED, "Dotnet host is not initialized.")
#define LOAD_NOVAHOST_FUNC(assemblyPath, typeName, funcName) LoadManagedFunctionPtr(assemblyPath, L"NovaHost."##typeName L", NovaHost", funcName)

// hostfxr function pointers
static hostfxr_initialize_for_runtime_config_fn HostfxrInitializeForRuntimeConfig;
static hostfxr_close_fn HostfxrClose;
static hostfxr_get_runtime_delegate_fn HostfxrGetRuntimeDelegate;
static hostfxr_set_error_writer_fn HostfxrSetErrorWriter;

static load_assembly_fn HostfxrLoadAssembly;
static load_assembly_and_get_function_pointer_fn HostfxrLoadAssemblyAndGetFunctionPointer;

// managed function pointers
// ManagedHost
NvManagedHostInitializeFn NvManagedHostInitialize;

// AssemblyLoader
NvManagedLoadAssemblyFromFilepathFn NvManagedLoadAssemblyFromFilepath;
NvManagedUnloadContextFn NvManagedUnloadContext;
NvManagedReloadContextFn NvManagedReloadContext;
NvManagedGetAssemblyIDFn NvManagedGetAssemblyID;
NvManagedGetAssemblyNameFn NvManagedGetAssemblyName;
NvManagedGetAssemblyFullNameFn NvManagedGetAssemblyFullName;

static bool s_IsInitialized;
static hostfxr_handle s_HostfxrHandle;
static HMODULE s_HostfxrLibrary;

static wchar_t *GetHostfxrPath(const wchar_t *requiredVersion)
{
    wchar_t *programFilesPath = NULL;
    const HRESULT status = SHGetKnownFolderPath(
        &FOLDERID_ProgramFiles,
        0,
        NULL,
        &programFilesPath);
    if (!SUCCEEDED(status))
        // NvExceptionThrow("Failed to retrieve program files directory filepath, status code: %d.\n", status);
        NvExceptionThrow("Failed to retrieve program files directory filepath.\n");

    wchar_t pathBuffer[MAX_PATH];
    wchar_t *originalPath = PathCombineW(pathBuffer, programFilesPath, L"dotnet\\host\\fxr");
    NV_ASSERT(originalPath != NULL, "Failed to concatentate path.");

    CoTaskMemFree(programFilesPath);

    if (!PathFileExistsW(originalPath))
        NvExceptionThrow("Hostfxr directory does not exist.\n");

    size_t originalPathLength = wcslen(originalPath);
    if (originalPathLength + sizeof(L"\\*") > MAX_PATH)
        NvExceptionThrow("Hostfxr path is too long.\n");

    WIN32_FIND_DATAW findData;
    HANDLE findHandle = FindFirstFileW(wcscat(originalPath, L"\\*"), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
        NvExceptionThrow("Hostfxr directory doesn't contain any files.\n");

    wchar_t candidatePath[MAX_PATH];
    size_t versionStrLen = wcslen(requiredVersion);
    bool found = false;

    do
    {
        *(originalPath + originalPathLength) = L'\0';

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
        {
            printf("Skipping hostfxr path candidate \"ls\": path is not a directory.\n", findData.cFileName);
            continue;
        }

        if (originalPathLength + wcslen(findData.cFileName) + sizeof(L"\\hostfxr.dll") > MAX_PATH)
        {
            printf("Skipping hostfxr path candidate \"%ls\": filepath is too long.\n", findData.cFileName);
            continue;
        }

        if (wcsncmp(findData.cFileName, requiredVersion, versionStrLen) != 0)
        {
            printf("Skipping hostfxr path candidate \"%ls\": dotnet version do not match the requied (%ls)\n", findData.cFileName, requiredVersion);
            continue;
        }

        PathCombineW(candidatePath, originalPath, findData.cFileName);
        PathCombineW(candidatePath, candidatePath, L"hostfxr.dll");
        if (!PathFileExistsW(candidatePath))
        {
            printf("Skipping hostfxr path candidate \"%ls\": directory doesn't contain hostfxr.dll.\n", findData.cFileName);
            continue;
        }

        found = true;
        break;
    } while (FindNextFileW(findHandle, &findData));

    FindClose(findHandle);

    if (!found)
        NvExceptionThrow("Failed to find hostfxr.dll path.\n");

    return wcsdup(candidatePath);
}

static void *LoadHostfxrFunctionPtr(const char *funcName)
{
    NV_ASSERT(s_HostfxrLibrary, "Hostfxr library is not loaded.");

    void *ptr = GetProcAddress(s_HostfxrLibrary, funcName);
    if (!ptr)
        NvExceptionThrow("Failed to load hostfxr function pointer.\n");

    return ptr;
}

static void *LoadManagedFunctionPtr(const wchar_t *assemblyPath, const wchar_t *typeName, const wchar_t *funcName)
{
    NV_ASSERT(assemblyPath, "Assembly path cannot be NULL");
    NV_ASSERT(typeName, "Type name cannot be NULL");
    NV_ASSERT(funcName, "Function name cannot be NULL");

    void *ptr = NULL;
    const status = HostfxrLoadAssemblyAndGetFunctionPointer(
        assemblyPath,
        typeName,
        funcName,
        UNMANAGEDCALLERSONLY_METHOD,
        NULL,
        &ptr);
    if (status != Success)
        NvExceptionThrow("Failed to load host managed function pointer.\n");

    NV_ASSERT(ptr, "Managed function pointer should not be NULL.");
    return ptr;
}

static void *LoadRuntimeDelegate(enum hostfxr_delegate_type type)
{
    NV_ASSERT(s_HostfxrHandle, ".NET host is not initialized.");

    void *ptr = NULL;
    uint32_t status = HostfxrGetRuntimeDelegate(s_HostfxrHandle, type, &ptr);
    if (status != Success)
        NvExceptionThrow("Failed to retrieve .NET host delegate.\n");

    NV_ASSERT(ptr, "Retrieved delegate pointer should not be NULL.");
    return ptr;
}

static void HostfxrErrorCallback(const wchar_t *message)
{
    NvExceptionThrow("Hostfxr error.");
}

static void InitializeHostfxr(const wchar_t *hostfxrPath, const wchar_t *configPath)
{
    s_HostfxrLibrary = LoadLibraryW(hostfxrPath);
    if (!s_HostfxrLibrary)
        NvExceptionThrow("Failed to load hostfxr.dll.\n");

    HostfxrSetErrorWriter = LoadHostfxrFunctionPtr("hostfxr_set_error_writer");
    HostfxrInitializeForRuntimeConfig = LoadHostfxrFunctionPtr("hostfxr_initialize_for_runtime_config");
    HostfxrGetRuntimeDelegate = LoadHostfxrFunctionPtr("hostfxr_get_runtime_delegate");
    HostfxrClose = LoadHostfxrFunctionPtr("hostfxr_close");

    HostfxrSetErrorWriter(HostfxrErrorCallback);

    int32_t status = HostfxrInitializeForRuntimeConfig(configPath, NULL, &s_HostfxrHandle);
    if (status != Success &&
        status != Success_HostAlreadyInitialized &&
        status != Success_DifferentRuntimeProperties)
        // NvExceptionThrow("Failed to initialize .NET runtime. Status: 0x%x.\n", status);
        NvExceptionThrow("Failed to initialize .NET runtime.\n");

    NV_ASSERT(s_HostfxrHandle, "Hostfxr handle should not be NULL");

    HostfxrLoadAssembly = LoadRuntimeDelegate(hdt_load_assembly);
    HostfxrLoadAssemblyAndGetFunctionPointer = LoadRuntimeDelegate(hdt_load_assembly_and_get_function_pointer);
}

static void InitializeManagedEnvironment(const wchar_t *assemblyPath, NvManagedLogCallback logCallback, NvManagedExcCallback excCallback)
{
    NvManagedHostInitialize = LOAD_NOVAHOST_FUNC(assemblyPath, L"ManagedHost", L"Initialize");
    NvManagedLoadAssemblyFromFilepath = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"LoadAssemblyFromFilepath");
    NvManagedUnloadContext = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"UnloadContext");
    NvManagedReloadContext = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"ReloadContext");
    NvManagedGetAssemblyID = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"GetAssemblyID");
    NvManagedGetAssemblyName = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"GetAssemblyName");
    NvManagedGetAssemblyFullName = LOAD_NOVAHOST_FUNC(assemblyPath, L"AssemblyLoader", L"GetAssemblyFullName");

    NvManagedHostInitialize(logCallback, excCallback);
}

static void ValidateSettings(const NvDotnetSettings *settings)
{
    NV_ASSERT(settings, "Settings ptr cannot be NULL");

    if (!settings->dotnetVersion)
        NvExceptionThrow(".NET requied version string cannot be NULL\n");

    if (!PathFileExistsW(settings->runtimePropertiesFilepath))
        // NvExceptionThrow(".NET runtime config file \"%ls\" does not exist.\n", settings->runtimePropertiesFilepath)
        NvExceptionThrow(".NET runtime config file does not exist.\n");

    if (!PathFileExistsW(settings->managedAssemblyFilepath))
        // NvExceptionThrow(stderr, ".NET managed assembly file \"%ls\" does not exist.\n", settings->managedAssemblyFilepath);
        NvExceptionThrow(".NET managed assembly file does not exist.\n");
}

static void ManagedLogCallback(int lvl, NvInteropString message)
{
    printf("[.NET] Managed Host -> %s.\n", (const char *)message.data);
}

static void ManagedExcCallback(NvInteropString excMessage, bool isFatal)
{
    if (isFatal)
        NvExceptionThrow((const char *)excMessage.data);

    fprintf(stderr, "[.NET] Managed Host -> Error:\n%s.\n", (const char *)excMessage.data);
}

static char *DupStringAndFree(NvInteropString string)
{
    char *result = NvInteropStringDup(&string);
    NvInteropStringFree(&string);

    return result;
}

void _NvDotnetHostInitialize(const NvDotnetSettings *settings)
{
    if (s_IsInitialized)
        return;

    ValidateSettings(settings);

    wchar_t *hostfxrPath = GetHostfxrPath(settings->dotnetVersion);
    InitializeHostfxr(hostfxrPath, settings->runtimePropertiesFilepath);
    free(hostfxrPath);

    InitializeManagedEnvironment(settings->managedAssemblyFilepath, ManagedLogCallback, ManagedExcCallback);
}

void _NvDotnetHostShutdown(void)
{
    if (s_HostfxrHandle)
        HostfxrClose(s_HostfxrHandle);

    if (s_HostfxrLibrary)
        FreeLibrary(s_HostfxrLibrary);

    s_HostfxrHandle = NULL;
    s_HostfxrLibrary = NULL;
}

bool NvDotnetHostLoadAssembly(const wchar_t *assemblyFilepath, const char *loadContextName, NvDotnetAssembly *out)
{
    NV_ASSERT(out != NULL, "Out assembly cannot be NULL");

    loadContextName = loadContextName ? loadContextName : "default";

    const NvInteropString assemblyFilepathManaged = NvInteropStringCreateStackW(assemblyFilepath);
    const NvInteropString contextNameManaged = NvInteropStringCreateStack(loadContextName);
    const int32_t assemblyId = NvManagedLoadAssemblyFromFilepath(assemblyFilepathManaged, contextNameManaged);
    if (assemblyId == -1)
    {
        fprintf(stderr, "Failed to load assembly from filepath \"%ls\".", assemblyFilepath);
        return false;
    }

    out->id = assemblyId;
    out->name = DupStringAndFree(NvManagedGetAssemblyName(assemblyId));
    out->fullName = DupStringAndFree(NvManagedGetAssemblyFullName(assemblyId));

    return true;
}

void NvDotnetHostUnloadContext(const char *contextName)
{
}
