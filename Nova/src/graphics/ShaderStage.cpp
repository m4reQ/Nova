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
	const ShaderSpecializeInfo& info)
{
	NV_PROFILE_FUNC;

	std::vector<GLuint> constantIndices;
	constantIndices.reserve(info.SpecializeConstants.size());

	std::vector<GLuint> constantValues;
	constantValues.reserve(info.SpecializeConstants.size());

	for (const auto& [index, value] : info.SpecializeConstants)
	{
		constantIndices.emplace_back(index);
		constantValues.emplace_back(value);
	}

	glSpecializeShaderARB(
		stageID,
		info.EntryPoint.data(),
		(GLuint)info.SpecializeConstants.size(),
		constantIndices.data(),
		constantValues.data());
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
	glShaderSource(id, 1, &data, (GLint*)&length);
	glCompileShader(id);

	CheckShaderStatus(id);

	return ShaderStage(id);
}

ShaderStage ShaderStage::FromGLSL(
	ShaderType type,
	const std::filesystem::path& filepath)
{
	NV_PROFILE_FUNC;

	const auto [source, size] = File::ReadText(filepath);
	return FromGLSL(type, std::string_view(source.get(), size));
}

ShaderStage ShaderStage::FromBinary(
	ShaderType type,
	GLenum binaryType,
	const std::span<uint8_t> binary)
{
	return FromBinary(
		type,
		binaryType,
		binary.data(),
		binary.size_bytes());
}

ShaderStage ShaderStage::FromBinary(
	ShaderType type,
	GLenum binaryType,
	const uint8_t* binary,
	size_t binarySize)
{
	NV_PROFILE_FUNC;

	if (!check_fits_in<GLsizei>(binarySize))
		throw std::overflow_error("Binary size exceeds max size accepted by OpenGL.");

	const auto id = glCreateShader((GLenum)type);
	glShaderBinary(
		1,
		&id,
		binaryType,
		binary,
		(GLsizei)binarySize);

	CheckShaderStatus(id);

	return ShaderStage(id);
}

ShaderStage ShaderStage::FromBinary(
	ShaderType type,
	GLenum binaryType,
	const std::filesystem::path& filepath)
{
	NV_PROFILE_FUNC;

	const auto [binary, size] = File::ReadBinary(filepath);
	return FromBinary(
		type,
		binaryType,
		binary.get(),
		size);
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::span<uint8_t> binary)
{
	return FromSPIRV(type, binary.data(), binary.size_bytes());
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::span<uint8_t> binary,
	const ShaderSpecializeInfo& specializeInfo)
{
	NV_PROFILE_FUNC;

	const auto stage = FromSPIRV(type, binary);
	SpecializeShaderStage(stage.m_ID, specializeInfo);

	return stage;
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::uint8_t* binary,
	size_t binarySize)
{
	NV_PROFILE_FUNC;

	return FromBinary(type, GL_SPIR_V_BINARY, binary, binarySize);
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::uint8_t* binary,
	size_t binarySize,
	const ShaderSpecializeInfo& specializeInfo)
{
	NV_PROFILE_FUNC;

	const auto stage = FromSPIRV(type, binary, binarySize);
	SpecializeShaderStage(stage.m_ID, specializeInfo);

	return stage;
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::filesystem::path& filepath)
{
	NV_PROFILE_FUNC;

	const auto [binary, size] = File::ReadBinary(filepath);
	return FromSPIRV(type, binary.get(), size);
}

ShaderStage ShaderStage::FromSPIRV(
	ShaderType type,
	const std::filesystem::path& filepath,
	const ShaderSpecializeInfo& specializeInfo)
{
	NV_PROFILE_FUNC;

	const auto stage = FromSPIRV(type, filepath);
	SpecializeShaderStage(stage.m_ID, specializeInfo);

	return stage;
}