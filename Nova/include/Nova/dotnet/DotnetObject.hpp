#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/dotnet/DotnetType.hpp>

namespace Nova
{
	class NV_API DotnetObject
	{
	public:
		DotnetObject() = delete;
		DotnetObject(const DotnetObject &) = delete;
		
		DotnetObject(DotnetObject &&other) noexcept;

		void Destroy() noexcept;

		constexpr DotnetType &GetType() const noexcept { return *m_Type; }
	private:
		void *m_Handle;
		DotnetType *m_Type;
	};
}