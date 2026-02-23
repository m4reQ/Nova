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
		const std::string_view EntryPoint;
		const std::span<std::pair<GLuint, GLuint>> SpecializeConstants;
	};

	class ShaderStage
	{
	public:
		static ShaderStage FromGLSL(
			ShaderType type,
			const std::string_view source);

		static ShaderStage FromGLSL(
			ShaderType type,
			const std::filesystem::path& filepath);

		static ShaderStage FromBinary(
			ShaderType type,
			GLenum binaryType,
			const std::span<uint8_t> binary);

		static ShaderStage FromBinary(
			ShaderType type,
			GLenum binaryType,
			const uint8_t* binary,
			size_t binarySize);

		static ShaderStage FromBinary(
			ShaderType type,
			GLenum binaryType,
			const std::filesystem::path& filepath);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::span<uint8_t> binary);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::span<uint8_t> binary,
			const ShaderSpecializeInfo& specializeInfo);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::uint8_t* binary,
			size_t binarySize);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::uint8_t* binary,
			size_t binarySize,
			const ShaderSpecializeInfo& specializeInfo);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::filesystem::path& filepath);

		static ShaderStage FromSPIRV(
			ShaderType type,
			const std::filesystem::path& filepath,
			const ShaderSpecializeInfo& specializeInfo);

		ShaderStage() = default;

		constexpr ShaderStage(GLuint id) noexcept
			: m_ID(id) { }

		void Delete() const noexcept { glDeleteShader(m_ID); }

		constexpr GLuint GetID() const noexcept { return m_ID; }

	private:
		GLuint m_ID = 0;
	};
}