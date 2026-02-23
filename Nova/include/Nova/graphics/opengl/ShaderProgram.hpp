#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/ShaderStage.hpp>
#include <glm/vec3.hpp>
#include <span>
#include <string_view>
#include <optional>
#include <filesystem>
#include <utility>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Nova
{
	struct ProgramBinary
	{
		std::unique_ptr<std::byte[]> Binary;
		size_t Size;
		GLenum Format;
	};

	class ShaderProgram
	{
	public:
		static void ReleaseShaderCompiler() noexcept;

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const std::span<uint8_t> binary);

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const uint8_t* binary,
			size_t binarySize);

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const std::filesystem::path& filepath);

		static bool IsShaderBinarySupported() noexcept;

		static bool IsProgramBinarySupported() noexcept;

		ShaderProgram() = default;

		ShaderProgram(const ShaderProgram&) = delete;

		ShaderProgram(ShaderProgram&& other) noexcept
			: resources_(std::move(other.resources_)),
			  savedBinary_(std::move(other.savedBinary_)),
			  id_(other.id_) { }

		ShaderProgram(const ShaderStage* stages, size_t stagesCount);

		template <HasArrayInterface<ShaderStage> T>
		ShaderProgram(const T& stages)
			: ShaderProgram(stages.data(), stages.size()) { }

		template <HasConstArrayInterface<ShaderStage> T>
		ShaderProgram(const T& stages)
			: ShaderProgram(stages.data(), stages.size()) { }

		ShaderProgram(std::initializer_list<ShaderStage> stages)
			: ShaderProgram(std::span(stages)) { }

		void Use() const;

		void Delete() noexcept;

		void SetUniform(const std::string_view name, float value) const;

		void SetUniform(const std::string_view name, int32_t value) const;

		void SetUniform(const std::string_view name, uint32_t value) const;

		void SetUniform(const std::string_view name, const glm::vec3& value) const;

		GLuint GetResourceLocation(const std::string_view name) const;

		std::optional<GLuint> TryGetResourceLocation(const std::string_view name) const;

		std::pair<const std::span<std::byte>, GLenum> GetBinary();

		ShaderProgram& operator=(ShaderProgram&& other) noexcept
		{
			resources_ = std::move(other.resources_);
			savedBinary_ = std::move(other.savedBinary_);
			id_ = std::exchange(other.id_, 0);

			return *this;
		}

	private:
		std::unordered_map<
			std::string,
			GLuint,
			StringHash,
			std::equal_to<>> resources_;
		std::optional<ProgramBinary> savedBinary_ = std::nullopt;
		GLuint id_;
	};
}
