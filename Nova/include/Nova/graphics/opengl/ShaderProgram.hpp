#pragma once
#include <Nova/graphics/opengl/ID.hpp>
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
#include <variant>

namespace Nova
{
	enum class ShaderType
	{
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Compute = GL_COMPUTE_SHADER,
		TesselationControl = GL_TESS_CONTROL_SHADER,
		TesselationEvaluation = GL_TESS_EVALUATION_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
	};

	struct ProgramBinary
	{
		std::unique_ptr<std::byte[]> Binary;
		size_t Size;
		GLenum Format;
	};

	enum class DependencyType
	{
		Source = 1,
		BinaryStage = 2,
		BinaryProgram = 3,
	};

	/// @brief Specifies a dependency of specific shader stage with source code.
	struct SourceDependency
	{
		ShaderType ShaderType;
	};

	/// @brief Specifies a dependency of specific shader stage with binary.
	struct BinaryStageDependency
	{
		ShaderType ShaderType;
		GLenum BinaryFormat;
	};

	/// @brief Specifies a depenedncy of whole shader program with this program's binary
	struct BinaryProgramDependency
	{
		GLenum BinaryFormat;
	};

	/// @brief Specifies information about shader program creation dependency
	struct ProgramDependency
	{
		std::filesystem::file_time_type Timestamp;
		std::filesystem::path Filepath;
		uintmax_t FileSize;
		std::variant<SourceDependency, BinaryStageDependency, BinaryProgramDependency> Data;
	};

	struct ShaderSpecializeInfo
	{
		std::string_view EntryPoint;
		std::span<const GLuint> ConstantIndices;
		std::span<const GLuint> ConstantValues;
	};

	/// @brief Used for configuring shader output variables.
	/// Equivalent of GLSLs `layout(location=<Location>) out ... <Name>`.
	struct OutputLocation
	{
		std::string Name;
		GLuint Location;
	};

	struct SourceShaderStage
	{
		ShaderType Type;
		std::string_view Source;
	};

	struct BinaryShaderStage
	{
		ShaderType Type;
		GLenum BinaryFormat;
		std::span<const std::byte> Binary;
	};

	struct SourceFileShaderStage
	{
		ShaderType Type;
		std::filesystem::path Filepath;
	};

	struct BinaryFileShaderStage
	{
		ShaderType Type;
		GLenum BinaryFormat;
		std::filesystem::path Filepath;
	};

	struct SPIRVShaderStage
	{
		ShaderType Type;
		std::span<const std::byte> Binary;
		std::optional<ShaderSpecializeInfo> SpecializeInfo = std::nullopt;
	};

	struct SPIRVFileShaderStage
	{
		ShaderType Type;
		std::filesystem::path Filepath;
		std::optional<ShaderSpecializeInfo> SpecializeInfo = std::nullopt;
	};

	using ShaderStage = std::variant<
		SourceShaderStage,
		BinaryShaderStage,
		SourceFileShaderStage,
		BinaryFileShaderStage,
		SPIRVShaderStage,
		SPIRVFileShaderStage>;

	class ShaderProgram
	{
	public:
		static void ReleaseShaderCompiler() noexcept;

		static bool IsShaderBinarySupported() noexcept;

		static bool IsProgramBinarySupported() noexcept;

		ShaderProgram() = default;

		ShaderProgram(const ShaderProgram &) = delete;

		ShaderProgram(ShaderProgram &&) noexcept = default;

		ShaderProgram(GLID id);

		ShaderProgram(std::span<const std::byte> binary, GLenum binaryFormat);

		ShaderProgram(std::filesystem::path binaryFile, GLenum binaryFormat);

		ShaderProgram(std::span<const ShaderStage> stages, std::span<const OutputLocation> outputs = {});

		ShaderProgram(std::initializer_list<ShaderStage> stages, std::initializer_list<OutputLocation> outputs = {});

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

		constexpr const std::vector<ProgramDependency> &GetDependencies() const noexcept { return dependencies_; }

		std::optional<GLuint> TryGetResourceLocation(const std::string_view name) const;

		std::pair<std::vector<std::byte>, GLenum> GetBinary() const;

		ShaderProgram &operator=(const ShaderProgram &) = delete;

		ShaderProgram &operator=(ShaderProgram &&) noexcept = default;

	private:
		std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> resources_;
		std::vector<ProgramDependency> dependencies_;
		GLID id_;
	};
}
