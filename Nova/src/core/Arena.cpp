#include <Nova/memory/Arena.hpp>
#include <stdexcept>

using namespace Nova;

size_t Arena::GetSize() const noexcept
{
	size_t size = 0;
	for (const ArenaRegion &region : m_Regions)
		size += region.CurrentSize;
	
	return size;
}

void Arena::Reset() noexcept
{
	m_Regions.clear();
}

void* Arena::Put(const void *obj, size_t objSize)
{
	const size_t sizeInPages = (objSize + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

	for (ArenaRegion &region : m_Regions)
	{
		if (region.CurrentSize + sizeInPages <= region.GetCapacity())
		{
			void* result = std::memcpy(&region.Data[region.CurrentSize], obj, objSize);
			region.CurrentSize += sizeInPages;
			
			return result;
		}
	}

	ArenaRegion &region = m_Regions.emplace_back();
	if (sizeInPages > region.GetCapacity())
		throw std::runtime_error("Object cannot fit into arena region.");

	void* result = std::memcpy(&region.Data[region.CurrentSize], obj, objSize);
	region.CurrentSize += sizeInPages;

	return result;
}