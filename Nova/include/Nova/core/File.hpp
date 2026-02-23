#pragma once
#include <filesystem>
#include <utility>
#include <string_view>

namespace Nova::File
{
	std::pair<std::unique_ptr<uint8_t[]>, size_t> Read(
		const std::filesystem::path& filepath,
		const std::string_view mode);

	std::pair<std::unique_ptr<char[]>, size_t> ReadText(
		const std::filesystem::path& filepath);

	std::pair<std::unique_ptr<uint8_t[]>, size_t> ReadBinary(
		const std::filesystem::path& filepath);
}