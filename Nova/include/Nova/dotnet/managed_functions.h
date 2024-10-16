#pragma once
#include <Nova/dotnet/interop/interop_string.h>
#include <stddef.h>

typedef void (*NvManagedLogCallback)(int level, NvInteropString msg);
typedef void (*NvManagedExcCallback)(NvInteropString msg, bool isFatal);

typedef void (*NvManagedHostInitializeFn)(NvManagedLogCallback logCallback, NvManagedExcCallback excCallback);

typedef int32_t (*NvManagedLoadAssemblyFromFilepathFn)(NvInteropString filepath, NvInteropString loadContext);
typedef void (*NvManagedUnloadContextFn)(NvInteropString contextName);
typedef void (*NvManagedReloadContextFn)(NvInteropString contextName);
typedef int32_t (*NvManagedGetAssemblyIDFn)(NvInteropString assemblyName, NvInteropString contextName);
typedef NvInteropString (*NvManagedGetAssemblyNameFn)(int32_t assemblyId);
typedef NvInteropString (*NvManagedGetAssemblyFullNameFn)(int32_t assemblyId);

// ManagedHost
extern NvManagedHostInitializeFn NvManagedHostInitialize;

// AssemblyLoader
extern NvManagedLoadAssemblyFromFilepathFn NvManagedLoadAssemblyFromFilepath;
extern NvManagedUnloadContextFn NvManagedUnloadContext;
extern NvManagedReloadContextFn NvManagedReloadContext;
extern NvManagedGetAssemblyIDFn NvManagedGetAssemblyID;
extern NvManagedGetAssemblyNameFn NvManagedGetAssemblyName;
extern NvManagedGetAssemblyFullNameFn NvManagedGetAssemblyFullName;
