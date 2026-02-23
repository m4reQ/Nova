#pragma once
#include <memory>
#include <list>
#include <array>

namespace Nova
{
	struct ArenaRegion
	{
		size_t CurrentSize;
		std::array<uintptr_t, 8192 / sizeof(uintptr_t)> Data;

		constexpr size_t GetCapacity() const noexcept { return Data.max_size(); }
	};

	class Arena
	{
	public:
		Arena() = default;

		void Reset() noexcept;
		void *Put(const void *obj, size_t objSize);
		size_t GetSize() const noexcept;

	private:
		std::list<ArenaRegion> m_Regions;
	};
}