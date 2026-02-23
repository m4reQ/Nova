#pragma once
#include <Nova/dotnet/DotnetSettings.hpp>
#include <Nova/dotnet/AssemblyInfo.hpp>
#include <Nova/dotnet/String.hpp>
#include <Nova/dotnet/TypeInfo.hpp>
#include <dotnet/hostfxr.h>
#include <filesystem>
#include <span>

namespace Nova
{
	enum class CollectionMode : uint32_t
	{
        Default = 0,
        Forced = 1,
        Optimized = 2,
        Aggressive = 3,
	};

	struct InternalCallUpload
	{
		const char *Name;
		const void *FunctionPtr;
	};

	namespace Dotnet
	{
		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void InvokeStaticMethod_(uint32_t methodHash, void* resultPtr, const void** argsPtr);

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void UploadInternalCalls_(uint32_t typeID, const std::span<InternalCallUpload> funcDefs);

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void CollectGarbage_(
			int32_t generation = -1,
			CollectionMode mode = CollectionMode::Default,
			bool blocking = false,
			bool compacting = false) noexcept;

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void WaitForPendingFinalizers_() noexcept;

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void FreeGCHandle_(uint32_t handle) noexcept;

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		AssemblyInfo LoadAssemblyFromFilepath_(const std::filesystem::path& filepath);
		
		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		AssemblyInfo LoadAssemblyFromMemory_(const std::span<uint8_t> data);

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		LocalMemory<TypeInfo> GetAssemblyTypes_(int32_t assemblyID);

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		hostfxr_handle GetHostHandle_() noexcept;

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void* GetHostLibraryHandle_() noexcept;
		
		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void Initialize_(const DotnetSettings& settings);

		/// <summary>
        /// Private API. Don't user directly!
        /// </summary>
		void Shutdown_() noexcept;
	};
}
