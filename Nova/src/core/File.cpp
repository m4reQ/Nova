#include <Nova/core/File.hpp>
#include <Nova/core/Memory.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

constexpr size_t c_ReadChunkSize = 4096;

template <typename T>
static std::vector<T> ReadFileToEnd(FILE *file) noexcept
{
	std::vector<T> buffer(c_ReadChunkSize);
	size_t dataSize = 0;
	while (true)
	{
		size_t bytesRead = fread(
			&buffer[dataSize],
			sizeof(T),
			c_ReadChunkSize,
			file);
		dataSize += bytesRead;

		if (bytesRead < c_ReadChunkSize)
			break;

		buffer.resize(buffer.size() + c_ReadChunkSize);
	}

	buffer.resize(dataSize);

	return buffer;
}

std::vector<char> File::ReadWholeText(const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	FILE *file = fopen(filepath.string().c_str(), "r");
	if (file == nullptr)
		throw std::runtime_error("Failed to open file.");

	const auto result = ReadFileToEnd<char>(file);

	fclose(file);

	return result;
}

std::vector<std::byte> File::ReadWholeBinary(const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	FILE *file = fopen(filepath.string().c_str(), "rb");
	if (file == nullptr)
		throw std::runtime_error("Failed to open file.");

	const auto result = ReadFileToEnd<std::byte>(file);

	fclose(file);

	return result;
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

std::vector<std::byte> File::ReadToEndBinary() const noexcept
{
	return ReadFileToEnd<std::byte>(file_);
}

std::vector<char> File::ReadToEndText() const noexcept
{
	return ReadFileToEnd<char>(file_);
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