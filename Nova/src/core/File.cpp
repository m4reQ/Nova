#include <Nova/core/File.hpp>
#include <Nova/core/Memory.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

std::pair<std::unique_ptr<uint8_t[]>, size_t> File::Read(
	const std::filesystem::path& filepath,
	const std::string_view mode)
{
	NV_PROFILE_FUNC;

	constexpr size_t c_ReadChunkSize = 4096;

	FILE* file = fopen(filepath.string().c_str(), mode.data());
	if (file == nullptr)
		throw std::runtime_error("Failed to open file.");

	auto data = std::make_unique<uint8_t[]>(c_ReadChunkSize);
	size_t bufferSize = c_ReadChunkSize;
	size_t dataSize = 0;
	while (true)
	{
		NV_PROFILE_SCOPE("::ReadFileChunk");

		size_t bytesRead = fread(&data[dataSize], sizeof(std::byte), c_ReadChunkSize, file);
		dataSize += bytesRead;

		if (bytesRead < c_ReadChunkSize)
			break;

		{
			NV_PROFILE_SCOPE("::ReallocateChunk");
			auto newData = std::make_unique<uint8_t[]>(dataSize + c_ReadChunkSize);
			std::memcpy(newData.get(), data.get(), dataSize);
			data = std::move(newData);
		}
	}

	fclose(file);

	return std::make_pair(std::move(data), dataSize);
}

std::pair<std::unique_ptr<char[]>, size_t> File::ReadText(
	const std::filesystem::path& filepath)
{
	auto [data, size] = Read(filepath, "rS");
	return std::make_pair(unique_ptr_cast<char[]>(std::move(data)), size);
}

std::pair<std::unique_ptr<uint8_t[]>, size_t> File::ReadBinary(
	const std::filesystem::path& filepath)
{
	return Read(filepath, "rbS");
}