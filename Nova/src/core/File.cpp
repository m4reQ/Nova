#include <Nova/core/File.hpp>
#include <Nova/core/Memory.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

std::pair<std::unique_ptr<uint8_t[]>, size_t> File::ReadWhole(
	const std::filesystem::path &filepath,
	const std::string_view mode)
{
	NV_PROFILE_FUNC;

	constexpr size_t c_ReadChunkSize = 4096;

	FILE *file = fopen(filepath.string().c_str(), mode.data());
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

std::pair<std::unique_ptr<char[]>, size_t> File::ReadWholeText(
	const std::filesystem::path &filepath)
{
	auto [data, size] = ReadWhole(filepath, "rS");
	return std::make_pair(unique_ptr_cast<char[]>(std::move(data)), size);
}

std::pair<std::unique_ptr<uint8_t[]>, size_t> File::ReadWholeBinary(
	const std::filesystem::path &filepath)
{
	return ReadWhole(filepath, "rbS");
}

File::File(const std::string_view filepath, const std::string_view mode)
{
	const auto result = fopen_s(&file_, filepath.data(), mode.data());
	if (result)
		throw std::runtime_error("Failed to open file.");
}

File::File(const std::filesystem::path &filepath, const std::string_view mode)
	: File(std::string_view(filepath.string()), mode) {}

File::~File() noexcept
{
	fclose(file_);
}

size_t File::Read(void *data, size_t elementSize, size_t elementsCount) const noexcept
{
	return fread(data, elementSize, elementsCount, file_);
}

size_t File::ReadChecked(void *data, size_t elementSize, size_t elementsCount) const
{
	const auto readSize = Read(data, elementSize, elementsCount);
	if (readSize != elementsCount)
		throw std::runtime_error("Failed to read file");

	return readSize;
}

int File::Seek(long offset, FileSeekOrigin origin)
{
	return fseek(file_, offset, (int)origin);
}

bool File::GetLine(std::string &out) const
{
	out.clear();

	char buffer[4096];

	while (fgets(buffer, sizeof(buffer), file_))
	{
		out += buffer;

		if (!out.empty() && out.back() == '\n')
			return true;
	}

	return !out.empty();
}