#include <Nova/graphics/opengl/ShaderStage.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/File.hpp>
#include <Nova/core/Memory.hpp>

using namespace Nova;

static void CheckShaderStatus(GLuint id)
{
	GLint compileSuccess = GL_FALSE;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileSuccess);

	if (!compileSuccess)
	{
		GLint logLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		std::string log(logLength, '\0');
		glGetShaderInfoLog(id, logLength, nullptr, log.data());

		NV_LOG_ERROR("Failed to compile shader stage:\n{}.", log);
		throw std::runtime_error("Failed to compile shader stage.");
	}
}

static void SpecializeShaderStage(
	GLuint stageID,
	const ShaderSpecializeInfo &info)
{
	NV_PROFILE_FUNC;

	if (info.ConstantIndices.size() != info.ConstantValues.size())
		throw std::runtime_error("Specialize constant indices count has to equal specialize constant values count.");

	assert_fits_in<GLuint>(info.ConstantIndices.size());

	glSpecializeShaderARB(
		stageID,
		info.EntryPoint.data(),
		static_cast<GLuint>(info.ConstantIndices.size()),
		info.ConstantIndices.data(),
		info.ConstantValues.data());
}

ShaderStage ShaderStage::FromGLSL(
	ShaderType type,
	const std::string_view source)
{
	NV_PROFILE_FUNC;

	if (!check_fits_in<GLint>(source.size()))
		throw std::overflow_error("Source length exceeds max allowed by OpenGL");

	const auto id = glCreateShader((GLenum)type);

	const auto data = source.data();
	const auto length = source.size();
	glShaderSource(id, 1, &data, (GLint *)&length);
	glCompileShader(id);

	CheckShaderStatus(id);

	return ShaderStage(id);
}

ShaderStage ShaderStage::FromGLSL(
	ShaderType type,
	const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	const auto source = File::ReadWholeText(filepath);
	return FromGLSL(type, std::string_view(source.data(), source.size()));
}

ShaderStage ShaderStage::FromBinary(
	ShaderType type,
	GLenum binaryType,
	std::span<const std::byte> binary)
{
	NV_PROFILE_FUNC;

	assert_fits_in<GLsizei>(binary.size_bytes());

	const auto id = glCreateShader(static_cast<GLenum>(type));
	glShaderBinary(
		1,
		&id,
		binaryType,
		binary.data(),
		static_cast<GLsizei>(binary.size_bytes()));

	CheckShaderStatus(id);

	return ShaderStage(id);
}

ShaderStage ShaderStage::FromBinary(
	ShaderType type,
	GLenum binaryType,
	const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	return FromBinary(
		type,
		binaryType,
		File::ReadWholeBinary(filepath));
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	std::span<const std::byte> binary)
{
	return FromBinary(type, GL_SHADER_BINARY_FORMAT_SPIR_V, binary);
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	std::span<const std::byte> binary,
	const ShaderSpecializeInfo &specializeInfo)
{
	NV_PROFILE_FUNC;

	const auto stage = FromSPIRV(type, binary);
	SpecializeShaderStage(stage.id_, specializeInfo);

	return stage;
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	return FromSPIRV(type, File::ReadWholeBinary(filepath));
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::filesystem::path &filepath,
	const ShaderSpecializeInfo &specializeInfo)
{
	NV_PROFILE_FUNC;

	const auto stage = FromSPIRV(type, filepath);
	SpecializeShaderStage(stage.id_, specializeInfo);

	return stage;
}

Nova::ShaderStage::~ShaderStage() noexcept
{
	if (id_)
		glDeleteShader(id_);
}