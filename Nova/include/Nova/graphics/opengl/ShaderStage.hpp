#pragma once
#include <glad/gl.h>
#include <string_view>
#include <utility>
#include <span>
#include <filesystem>

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

	struct ShaderSpecializeInfo
	{
		std::string_view EntryPoint;
		std::span<const GLuint> ConstantIndices;
		std::span<const GLuint> ConstantValues;
	};

	class ShaderStage
	{
	public:
		static ShaderStage FromGLSL(
			ShaderType type,
			const std::string_view source);

		static ShaderStage FromGLSL(
			ShaderType type,
			const std::filesystem::path &filepath);

		static ShaderStage FromBinary(
			ShaderType type,
			GLenum binaryType,
			std::span<const std::byte> binary);

		static ShaderStage FromBinary(
			ShaderType type,
			GLenum binaryType,
			const std::filesystem::path &filepath);

		static ShaderStage FromSPIRV(
			ShaderType type,
			std::span<const std::byte> binary);

		static ShaderStage FromSPIRV(
			ShaderType type,
			std::span<const std::byte> binary,
			const ShaderSpecializeInfo &specializeInfo);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::filesystem::path &filepath);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::filesystem::path &filepath,
			const ShaderSpecializeInfo &specializeInfo);

		ShaderStage() = default;

		constexpr ShaderStage(GLuint id) noexcept
			: id_(id) {}

		~ShaderStage() noexcept;

		constexpr GLuint GetID() const noexcept { return id_; }

	private:
		GLuint id_ = 0;
	};
}