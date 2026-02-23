#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/dotnet/AssemblyInfo.hpp>
#include <Nova/dotnet/Type.hpp>
#include <span>
#include <filesystem>
#include <string_view>
#include <string>
#include <unordered_map>
#include <Nova/core/Utility.hpp>

namespace Nova
{
	class NV_API DotnetAssembly
	{
	public:
		DotnetAssembly() = default;

		DotnetAssembly(const std::filesystem::path& filepath);

		DotnetAssembly(const std::span<uint8_t> data);

		constexpr const AssemblyInfo& GetInfo() const noexcept { return info_; }

		const DotnetType& GetType(const std::string_view name);

	private:
		AssemblyInfo info_;
		std::unordered_map<std::string_view, DotnetType, StringHash, std::equal_to<>> types_;
	};
}
