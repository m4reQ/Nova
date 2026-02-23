#include <Nova/dotnet/Host.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Build.hpp>
#include <Nova/core/Memory.hpp>
#include <dotnet/hostfxr.h>
#include <dotnet/coreclr_delegates.h>
#include <dotnet/error_codes.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <locale>
#include <codecvt>

using namespace Nova;

// ManagedHost
typedef void (*ManagedLogCallback)(int level, DotnetANSIString msg);
typedef void (*ManagedExcCallback)(DotnetANSIString msg, bool isFatal);
typedef void (*ManagedHostInitializeFn)(ManagedLogCallback logCallback, ManagedExcCallback excCallback);

// AssemblyLoader
typedef AssemblyInfo (*ManagedLoadAssemblyFromFilepathFn)(DotnetUTF8String filepath);
typedef DotnetANSIString (*ManagedGetAssemblyNameFn)(uint32_t assemblyID);
typedef DotnetANSIString (*ManagedGetAssemblyFullNameFn)(uint32_t assemblyID);
typedef HLOCAL (*ManagedGetAssemblyTypesFn)(int32_t assemblyID);

// TypeInterface
typedef DotnetANSIString (*ManagedGetTypeNameFn)(uint32_t typeID);
typedef DotnetANSIString (*ManagedGetTypeFullNameFn)(uint32_t typeID);
typedef DotnetANSIString (*ManagedGetTypeAssemblyQualifiedNameFn)(uint32_t typeID);
typedef bool (*ManagedUploadInternalCallsFn)(uint32_t typeID, InternalCallUpload *uploadData, uint32_t uploadsCount);
typedef bool (*ManagedInvokeStaticMethodFn)(int32_t methodHash, void *resultPtr, const void **argsPtr);

// GarbageCollector
typedef void(*ManagedCollectGarbageFn)(int32_t generation, uint32_t collectionMode, bool blocking, bool compacting);
typedef void(*ManagedWaitForPendingFinalizersFn)(void);
typedef void(*ManagedFreeGCHandleFn)(uint32_t handle);

constexpr const char *c_HostfxrDllName = "hostfxr.dll";
constexpr const wchar_t* c_NovaHostFilepath = L"./NovaHost.dll";

static hostfxr_handle s_HostfxrHandle;
static HMODULE s_HostfxrLibrary;
static hostfxr_initialize_for_runtime_config_fn HostfxrInitializeForRuntimeConfig;
static hostfxr_close_fn HostfxrClose;
static hostfxr_get_runtime_delegate_fn HostfxrGetRuntimeDelegate;
static hostfxr_set_error_writer_fn HostfxrSetErrorWriter;
static load_assembly_fn HostfxrLoadAssembly;
static load_assembly_and_get_function_pointer_fn HostfxrLoadAssemblyAndGetFunctionPointer;
static ManagedHostInitializeFn ManagedHostInitialize;
static ManagedLoadAssemblyFromFilepathFn ManagedLoadAssemblyFromFilepath;
static ManagedGetAssemblyNameFn ManagedGetAssemblyName;
static ManagedGetAssemblyFullNameFn ManagedGetAssemblyFullName;
static ManagedGetAssemblyTypesFn ManagedGetAssemblyTypes;
static ManagedGetTypeNameFn ManagedGetTypeName;
static ManagedGetTypeFullNameFn ManagedGetTypeFullName;
static ManagedGetTypeAssemblyQualifiedNameFn ManagedGetTypeAssemblyQualifiedName;
static ManagedUploadInternalCallsFn ManagedUploadInternalCalls;
static ManagedInvokeStaticMethodFn ManagedInvokeStaticMethod;
static ManagedCollectGarbageFn ManagedCollectGarbage;
static ManagedWaitForPendingFinalizersFn ManagedWaitForPendingFinalizers;
static ManagedFreeGCHandleFn ManagedFreeGCHandle;

#ifdef NV_WINDOWS
static std::filesystem::path GetHostfxrBasePath()
{
	NV_PROFILE_FUNC;

	TaskMemory<wchar_t> programFilesPath;
	const auto result = SHGetKnownFolderPath(
		FOLDERID_ProgramFiles,
		0,
		nullptr,
		programFilesPath.GetRef());
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to retrieve program files path.");

	return std::filesystem::path(programFilesPath.Get()) / "dotnet" / "host" / "fxr";
}

static HMODULE LoadDynamicLibrary(const std::filesystem::path& path)
{
	auto result = LoadLibraryW(path.wstring().c_str());
	if (result == nullptr)
		throw std::runtime_error("Failed to load dynamic library.");

	return result;
}
#else
#error "Unsupported platform"
#endif

static constexpr const std::string_view HostfxrStatusToString(StatusCode status) noexcept
{
	switch (status)
	{
		case Success:
			return "Success";
		case Success_HostAlreadyInitialized:
			return "Success_HostAlreadyInitialized";
		case Success_DifferentRuntimeProperties:
			return "Success_DifferentRuntimeProperties";
		case InvalidArgFailure:
			return "InvalidArgFailure";
		case CoreHostLibLoadFailure:
			return "CoreHostLibLoadFailure";
		case CoreHostLibMissingFailure:
			return "CoreHostLibMissingFailure";
		case CoreHostEntryPointFailure:
			return "CoreHostEntryPointFailure";
		case CurrentHostFindFailure:
			return "CurrentHostFindFailure";
		case CoreClrResolveFailure:
			return "CoreClrResolveFailure";
		case CoreClrBindFailure:
			return "CoreClrBindFailure";
		case CoreClrInitFailure:
			return "CoreClrInitFailure";
		case CoreClrExeFailure:
			return "CoreClrExeFailure";
		case ResolverInitFailure:
			return "ResolverInitFailure";
		case ResolverResolveFailure:
			return "ResolverResolveFailure";
		case LibHostInitFailure:
			return "LibHostInitFailure";
		case LibHostInvalidArgs:
			return "LibHostInvalidArgs";
		case InvalidConfigFile:
			return "InvalidConfigFile";
		case AppArgNotRunnable:
			return "AppArgNotRunnable";
		case AppHostExeNotBoundFailure:
			return "AppHostExeNotBoundFailure";
		case FrameworkMissingFailure:
			return "FrameworkMissingFailure";
		case HostApiFailed:
			return "HostApiFailed";
		case HostApiBufferTooSmall:
			return "HostApiBufferTooSmall";
		case AppPathFindFailure:
			return "AppPathFindFailure";
		case SdkResolveFailure:
			return "SdkResolveFailure";
		case FrameworkCompatFailure:
			return "FrameworkCompatFailure";
		case FrameworkCompatRetry:
			return "FrameworkCompatRetry";
		case BundleExtractionFailure:
			return "BundleExtractionFailure";
		case BundleExtractionIOError:
			return "BundleExtractionIOError";
		case LibHostDuplicateProperty:
			return "LibHostDuplicateProperty";
		case HostApiUnsupportedVersion:
			return "HostApiUnsupportedVersion";
		case HostInvalidState:
			return "HostInvalidState";
		case HostPropertyNotFound:
			return "HostPropertyNotFound";
		case HostIncompatibleConfig:
			return "HostIncompatibleConfig";
		case HostApiUnsupportedScenario:
			return "HostApiUnsupportedScenario";
		case HostFeatureDisabled:
			return "HostFeatureDisabled";
		default:
			return "UnknownStatus";
	}

	NV_UNREACHABLE;
}

static std::filesystem::path GetHostfxrPath(const std::string_view version)
{
	NV_PROFILE_FUNC;

	const auto searchPath = GetHostfxrBasePath();
	if (!std::filesystem::exists(searchPath))
		throw std::runtime_error("Hostfxr directory does not exist.");

	for (const auto &dir : std::filesystem::recursive_directory_iterator(searchPath))
	{
		if (!dir.is_directory())
			continue;

		if (dir.path().string().find(version) == std::string::npos)
			continue;

		const auto result = dir.path() / c_HostfxrDllName;
		if (!std::filesystem::exists(result))
			throw std::runtime_error("Found hostfxr directory candidate but no hostfxr dll inside.");
		
		NV_LOG_TRACE("Using hostfxr dll: {}", result.string());
		return result;
	}

	throw std::runtime_error("Failed to find hostfxr path.");
}

static void* LoadHostFxrFunctionPtr(const std::string_view funcName)
{
	auto ptr = GetProcAddress(s_HostfxrLibrary, funcName.data());
	NV_CHECK(ptr, "Failed to load hostfxr library function.");

	NV_LOG_TRACE("Loaded hostfxr library function \"{}\" ({}).", funcName, (void*)ptr);

	return ptr;
}

static void* LoadRuntimeDelegate(hostfxr_handle handle, hostfxr_delegate_type type)
{
	void *ptr = nullptr;
	const auto status = HostfxrGetRuntimeDelegate(handle, type, &ptr);
	NV_CHECK(status == Success, "Failed to load hostfxr runtime delegate.");

	return ptr;
}

static void* LoadManagedFunctionPtr(const std::wstring_view typeName, const std::wstring_view methodName)
{
	NV_PROFILE_FUNC;

	void* ptr = nullptr;
	const auto status = HostfxrLoadAssemblyAndGetFunctionPointer(
		c_NovaHostFilepath,
		typeName.data(),
		methodName.data(),
		(const wchar_t *)UNMANAGEDCALLERSONLY_METHOD,
		nullptr,
		&ptr);
	if (status != StatusCode::Success)
	{
		NV_LOG_ERROR(
			"Failed to load NovaHost managed function: {} ({:x}).",
			HostfxrStatusToString(static_cast<StatusCode>(status)),
			status);
		throw std::runtime_error("Failed to load NovaHost managed function.");
	}

	return ptr;
}

static void LoadManagedFunctionPointers()
{
	NV_PROFILE_FUNC;

	NV_LOG_TRACE("Loading managed host functions from assembly file \"{}\"...", std::filesystem::absolute(c_NovaHostFilepath).string());

	ManagedHostInitialize = static_cast<ManagedHostInitializeFn>(LoadManagedFunctionPtr(L"NovaHost.ManagedHost, NovaHost", L"Initialize"));
	ManagedLoadAssemblyFromFilepath = static_cast<ManagedLoadAssemblyFromFilepathFn>(LoadManagedFunctionPtr(L"NovaHost.AssemblyLoader, NovaHost", L"LoadAssemblyFromFilepath"));
	ManagedGetAssemblyTypes = static_cast<ManagedGetAssemblyTypesFn>(LoadManagedFunctionPtr(L"NovaHost.AssemblyLoader, NovaHost", L"GetAssemblyTypes"));
	// ManagedGetAssemblyName = static_cast<ManagedGetAssemblyNameFn>(LoadManagedFunctionPtr(L"NovaHost.AssemblyLoader, NovaHost", L"GetAssemblyName"));
	// ManagedGetAssemblyFullName = static_cast<ManagedGetAssemblyFullNameFn>(LoadManagedFunctionPtr(L"NovaHost.AssemblyLoader, NovaHost", L"GetAssemblyFullName"));
	// ManagedGetAssemblyTypes = static_cast<ManagedGetAssemblyTypesFn>(LoadManagedFunctionPtr(L"NovaHost.AssemblyLoader, NovaHost", L"GetAssemblyTypes"));
	// ManagedGetTypeName = static_cast<ManagedGetTypeNameFn>(LoadManagedFunctionPtr(L"NovaHost.TypeInterface, NovaHost", L"GetTypeName"));
	// ManagedGetTypeFullName = static_cast<ManagedGetTypeFullNameFn>(LoadManagedFunctionPtr(L"NovaHost.TypeInterface, NovaHost", L"GetTypeFullName"));
	// ManagedGetTypeAssemblyQualifiedName = static_cast<ManagedGetTypeAssemblyQualifiedNameFn>(LoadManagedFunctionPtr(L"NovaHost.TypeInterface, NovaHost", L"GetTypeAssemblyQualifiedName"));
	// ManagedUploadInternalCalls = static_cast<ManagedUploadInternalCallsFn>(LoadManagedFunctionPtr(L"NovaHost.TypeInterface, NovaHost", L"UploadInternalCalls"));
	// ManagedInvokeStaticMethod = static_cast<ManagedInvokeStaticMethodFn>(LoadManagedFunctionPtr(L"NovaHost.TypeInterface, NovaHost", L"InvokeStaticMethod"));
	// ManagedCollectGarbage = static_cast<ManagedCollectGarbageFn>(LoadManagedFunctionPtr(L"NovaHost.GarbageCollector, NovaHost", L"CollectGarbage"));
	// ManagedWaitForPendingFinalizers = static_cast<ManagedWaitForPendingFinalizersFn>(LoadManagedFunctionPtr(L"NovaHost.GarbageCollector, NovaHost", L"WaitForPendingFinalizers"));
	// ManagedFreeGCHandle = static_cast<ManagedFreeGCHandleFn>(LoadManagedFunctionPtr(L"NovaHost.GarbageCollector, NovaHost", L"FreeGCHandle"));
}

static void InitializeManagedEnvironment()
{
	NV_PROFILE_FUNC;

	ManagedHostInitialize(
		[](int level, const DotnetANSIString msg)
		{
			NV_LOG_TRACE(".NET: {}", msg.GetView());
		},
		[](const DotnetANSIString msg, bool isFatal)
		{
			NV_LOG_ERROR(msg.GetView());
		});

	NV_LOG_TRACE("Initialized .NET managed Nova environment.");
}

static void InitializeHostfxrEnvironment(const std::filesystem::path &configPath)
{
	NV_PROFILE_FUNC;

	NV_LOG_TRACE("Using .NET runtime config file \"{}\".", std::filesystem::absolute(configPath).string());

	HostfxrSetErrorWriter(
		[](const auto msg)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			const auto msgConv = converter.to_bytes(msg);

			NV_LOG_ERROR("Hostfxr error occurred: {}", msgConv);
		});

	const auto status = (StatusCode)HostfxrInitializeForRuntimeConfig(configPath.c_str(), nullptr, &s_HostfxrHandle);
	if (status != Success &&
		status != Success_HostAlreadyInitialized &&
		status != Success_DifferentRuntimeProperties)
	{
		NV_LOG_ERROR("Failed to initialize hostfxr environment. Status: {}", HostfxrStatusToString(status));
		throw std::runtime_error("Failed to initialize hostfxr.");
	}

	NV_LOG_TRACE("Hostfxr initialized.");
}

static void LoadHostfxrDelegates() noexcept
{
	NV_PROFILE_FUNC;

	HostfxrLoadAssembly = static_cast<load_assembly_fn>(LoadRuntimeDelegate(s_HostfxrHandle, hdt_load_assembly));
	HostfxrLoadAssemblyAndGetFunctionPointer = static_cast<load_assembly_and_get_function_pointer_fn>(LoadRuntimeDelegate(s_HostfxrHandle, hdt_load_assembly_and_get_function_pointer));

	NV_LOG_TRACE("Loaded hostfxr runtime delegates.");
}

static void LoadHostfxrLibraryFunctions()
{
	NV_PROFILE_FUNC;

	HostfxrSetErrorWriter = static_cast<hostfxr_set_error_writer_fn>(LoadHostFxrFunctionPtr("hostfxr_set_error_writer"));
	HostfxrInitializeForRuntimeConfig = static_cast<hostfxr_initialize_for_runtime_config_fn>(LoadHostFxrFunctionPtr("hostfxr_initialize_for_runtime_config"));
	HostfxrGetRuntimeDelegate = static_cast<hostfxr_get_runtime_delegate_fn>(LoadHostFxrFunctionPtr("hostfxr_get_runtime_delegate"));
	HostfxrClose = static_cast<hostfxr_close_fn>(LoadHostFxrFunctionPtr("hostfxr_close"));

	NV_LOG_TRACE("Loaded hostfxr library functions.");
}

static void LoadHostfxrLibrary(const std::filesystem::path& filepath)
{
	NV_PROFILE_FUNC;

	s_HostfxrLibrary = LoadDynamicLibrary(filepath);
	NV_LOG_TRACE("Loaded hostfxr dll.");
}

void Dotnet::Initialize_(const DotnetSettings &settings)
{
	NV_PROFILE_FUNC;
	NV_LOG_TRACE("Initializing .NET host...");

	LoadHostfxrLibrary(GetHostfxrPath(settings.Version));
	LoadHostfxrLibraryFunctions();
	InitializeHostfxrEnvironment(settings.RuntimeConfigPath);
	LoadHostfxrDelegates();
	LoadManagedFunctionPointers();
	InitializeManagedEnvironment();

	NV_LOG_TRACE("Successfully initialized .NET host.");
}

void Dotnet::Shutdown_() noexcept
{
	if (s_HostfxrHandle)
	{
		HostfxrClose(s_HostfxrHandle);
		s_HostfxrHandle = nullptr;
	}

	if (s_HostfxrLibrary)
	{
		FreeLibrary((HMODULE)s_HostfxrLibrary);
		s_HostfxrLibrary = nullptr;
	}

	NV_LOG_TRACE(".NET host shut down.");
}

AssemblyInfo Dotnet::LoadAssemblyFromFilepath_(const std::filesystem::path& filepath)
{
	NV_PROFILE_FUNC;
	NV_CHECK(ManagedLoadAssemblyFromFilepath != nullptr, ".NET host is not initialized.");
	
	NV_LOG_TRACE("Loading .NET assembly from \"{}\"...", filepath.string());

	auto info = ManagedLoadAssemblyFromFilepath(DotnetUTF8String(filepath.c_str(), true));
	if (info.GetID() == 0)
	{
		NV_LOG_ERROR("Failed to load .NET assembly from file \"{}\".", filepath.string());
		throw std::runtime_error("Failed to load .NET assembly.");
	}

	NV_LOG_TRACE("Successfully loaded .NET assembly.");
	NV_LOG_TRACE("\tName: {}", info.GetName());
	NV_LOG_TRACE("\tFull name: {}", info.GetFullName());
	NV_LOG_TRACE("\tFilepath: {}", info.GetFilepath());

	return info;
}

AssemblyInfo Dotnet::LoadAssemblyFromMemory_(const std::span<uint8_t> data)
{
	NV_PROFILE_FUNC;

	return {};
}

hostfxr_handle Dotnet::GetHostHandle_() noexcept
{
	return s_HostfxrHandle;
}

void* Dotnet::GetHostLibraryHandle_() noexcept
{
	return static_cast<void*>(s_HostfxrLibrary);
}

void Dotnet::CollectGarbage_(int32_t generation, CollectionMode mode, bool blocking, bool compacting) noexcept
{
	NV_PROFILE_FUNC;

	ManagedCollectGarbage(generation, static_cast<int32_t>(mode), blocking, compacting);
}

void Dotnet::WaitForPendingFinalizers_() noexcept
{
	NV_PROFILE_FUNC;

	ManagedWaitForPendingFinalizers();
}

void Dotnet::FreeGCHandle_(uint32_t handle) noexcept
{
	NV_PROFILE_FUNC;

	ManagedFreeGCHandle(handle);
}

void Dotnet::UploadInternalCalls_(uint32_t typeID, const std::span<InternalCallUpload> funcDefs)
{
	NV_PROFILE_FUNC;

	NV_CHECK(check_fits_in<uint32_t>(funcDefs.size()), "Too many internal call uploads.");

	if (!ManagedUploadInternalCalls(typeID, funcDefs.data(), static_cast<uint32_t>(funcDefs.size())))
		throw std::runtime_error("Failed to upload internal calls for a .NET type.");
}

void Dotnet::InvokeStaticMethod_(uint32_t methodHash, void* resultPtr, const void** argsPtr)
{
	NV_PROFILE_FUNC;

	if (!ManagedInvokeStaticMethod(methodHash, resultPtr, argsPtr))
		throw std::runtime_error("Failed to invoke static method.");
}

LocalMemory<TypeInfo> Dotnet::GetAssemblyTypes_(int32_t assemblyID)
{
	auto typesBuffer = LocalMemory<TypeInfo>(ManagedGetAssemblyTypes(assemblyID));
	if (typesBuffer.IsNull())
	{
		NV_LOG_ERROR("Failed to retrieve types for assembly with ID: {}.", assemblyID);
		throw std::runtime_error("Failed to retrieve types for specified assembly.");
	}

	return std::move(typesBuffer);
}