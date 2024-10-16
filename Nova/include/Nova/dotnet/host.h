#pragma once
#include <Nova/core/build.h>
#include <Nova/dotnet/assembly.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    const wchar_t *runtimePropertiesFilepath;
    const wchar_t *managedAssemblyFilepath;
    const wchar_t *dotnetVersion;
} NvDotnetSettings;

void _NvDotnetHostInitialize(const NvDotnetSettings *settings);
void _NvDotnetHostShutdown(void);

NV_API bool NvDotnetHostLoadAssembly(const wchar_t *assemblyFilepath, const char *loadContextName, NvDotnetAssembly *out);
NV_API void NvDotnetHostUnloadContext(const char *contextName);
