#pragma once
#include <Nova/dotnet/Memory.hpp>

namespace Nova
{
    class AssemblyInfo
	{
	public:
		AssemblyInfo() = default;

        AssemblyInfo(AssemblyInfo&&) noexcept = default;

		constexpr const std::string_view GetName() const noexcept { return name_.Get(); }

		constexpr const std::string_view GetFullName() const noexcept { return fullName_.Get(); }
		
		constexpr const std::string_view GetFilepath() const noexcept { return filepath_.Get(); }
		
		constexpr int32_t GetID() const noexcept { return id_; }

	private:
		LocalMemory<char> name_;
		LocalMemory<char> fullName_;
		LocalMemory<char> filepath_;
		int32_t id_;
	};
}