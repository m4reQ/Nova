#pragma once
#include <filesystem>
#include <utility>
#include <string_view>
#include <cstdio>
#include <span>

namespace Nova
{
	enum class FileSeekOrigin : int
	{
		Current = SEEK_CUR,
		Begin = SEEK_SET,
		End = SEEK_END,
	};

	class File
	{
	public:
		static std::vector<char> ReadWholeText(const std::filesystem::path &filepath);

		static std::vector<std::byte> ReadWholeBinary(const std::filesystem::path &filepath);

		static void WriteBinary(const std::filesystem::path &filepath, std::span<const std::byte> binary);

		File() noexcept = default;

		constexpr File(File &&other) noexcept
			: file_(std::exchange(other.file_, nullptr)) {}

		File(const std::string_view filepath, const std::string_view mode);

		File(const std::filesystem::path &filepath, const std::string_view mode);

		~File() noexcept;

		size_t Read(void *data, size_t elementSize, size_t elementsCount) const noexcept;

		size_t ReadChecked(void *data, size_t elementSize, size_t elementsCount) const;

		template <typename T>
		size_t Read(T *data, size_t elementsCount = 1) { return Read(data, sizeof(T), elementsCount); }

		template <typename T>
		size_t ReadChecked(T *data, size_t elementsCount = 1) { return ReadChecked(data, sizeof(T), elementsCount); }

		std::vector<std::byte> ReadToEndBinary() const noexcept;

		std::vector<char> ReadToEndText() const noexcept;

		bool GetLine(std::string &out) const;

		int Seek(long offset, FileSeekOrigin origin);

		constexpr const FILE *GetHandle() const noexcept { return file_; }

		constexpr FILE *GetHandle() noexcept { return file_; }

		constexpr File &operator=(File &&other) noexcept
		{
			file_ = std::exchange(other.file_, nullptr);

			return *this;
		}

	private:
		FILE *file_ = nullptr;
	};
}