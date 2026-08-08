#pragma once
#include <Nova/graphics/opengl/ShaderStage.hpp>
#include <Nova/core/Utility.hpp>
#include <glm/glm.hpp>
#include <span>
#include <string>
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

	/// @brief Used for configuring shader output variables. Equivalent of GLSLs `layout(location=Location) out ... Name`.
	struct OutputLocation
	{
		std::string Name;
		GLuint Location;
	};

	class ShaderProgram
	{
	public:
		static void ReleaseShaderCompiler() noexcept;

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const std::span<std::byte> binary);

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const std::byte *binary,
			size_t binarySize);

		static ShaderProgram FromBinary(
			GLenum binaryFormat,
			const std::filesystem::path &filepath);

		static bool IsShaderBinarySupported() noexcept;

		static bool IsProgramBinarySupported() noexcept;

		ShaderProgram() = default;

		ShaderProgram(const ShaderProgram &) = delete;

		ShaderProgram(ShaderProgram &&other) noexcept;

		ShaderProgram(std::span<const ShaderStage> stages);

		ShaderProgram(std::span<const ShaderStage> stages, std::span<const OutputLocation> outputs);

		ShaderProgram(std::initializer_list<ShaderStage> stages);

		ShaderProgram(std::initializer_list<ShaderStage> stages, std::initializer_list<OutputLocation> outputs);

		~ShaderProgram() noexcept;

		void Use() const;

		void SetUniform(const std::string_view name, float value) const;

		void SetUniform(const std::string_view name, int32_t value) const;

		void SetUniform(const std::string_view name, uint32_t value) const;

		void SetUniform(const std::string_view name, const glm::vec4 &value) const;

		void SetUniform(const std::string_view name, const glm::vec3 &value) const;

		void SetUniform(const std::string_view name, const glm::vec2 &value) const;

		void SetUniform(const std::string_view name, GLuint64 value) const;

		void SetDebugName(const std::string_view name) const noexcept;

		GLuint GetResourceLocation(const std::string_view name) const;

		std::optional<GLuint> TryGetResourceLocation(const std::string_view name) const;

		std::pair<const std::span<std::byte>, GLenum> GetBinary();

		ShaderProgram &operator=(ShaderProgram &&other) noexcept
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
			std::equal_to<>>
			resources_;
		std::optional<ProgramBinary> savedBinary_ = std::nullopt;
		GLuint id_ = 0;
	};
}
