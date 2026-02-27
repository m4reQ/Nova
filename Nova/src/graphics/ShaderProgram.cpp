#include <Nova/graphics/opengl/ShaderProgram.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/core/Utility.hpp>
#include <Nova/core/File.hpp>
#include <iostream>
#include <fstream>
#include <array>
#include <limits>

using namespace Nova;

struct ResourceProps
{
	GLint Location;
	GLint NameLength;

	static constexpr const std::array<GLenum, 2> Names{GL_LOCATION, GL_NAME_LENGTH};
};

static ProgramBinary RetrieveProgramBinary(GLuint programID)
{
	NV_PROFILE_FUNC;

	GLint binarySize = 0;
	glGetProgramiv(programID, GL_PROGRAM_BINARY_LENGTH, &binarySize);

	GLenum binaryFormat = 0;
	auto binary = std::make_unique<std::byte[]>(binarySize);
	glGetProgramBinary(
		programID,
		binarySize,
		nullptr,
		&binaryFormat,
		binary.get());

	return ProgramBinary{
		.Binary = std::move(binary),
		.Size = (size_t)binarySize,
		.Format = binaryFormat,
	};
}

static void CheckProgramLinkStatus(GLuint program)
{
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	if (!linkStatus)
	{
		GLsizei linkMessageLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &linkMessageLength);

		std::string linkMessage(linkMessageLength, '\0');
		glGetProgramInfoLog(program, linkMessageLength, nullptr, linkMessage.data());

		std::cerr << linkMessage << std::endl;

		throw std::runtime_error("Failed to link shader program.");
	}
}

static void CleanUpAttachedShaders(GLuint program) noexcept
{
	NV_PROFILE_FUNC;

	GLint stagesCount = 0;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &stagesCount);

	const auto attachedShaders = std::make_unique<GLuint[]>(stagesCount);
	glGetAttachedShaders(program, stagesCount, nullptr, attachedShaders.get());

	for (size_t i = 0; i < stagesCount; i++)
	{
		const auto shader = attachedShaders[i];
		glDetachShader(program, shader);
		glDeleteShader(shader);
	}
}

static void NormalizeArrayResourceName(std::string &name)
{
	const auto bracketLocation = name.find('[');
	if (bracketLocation != -1)
	{
		name.resize(bracketLocation);
	}
}

static void RetrieveProgramInterface(
	GLuint program,
	ProgramInterface1 interface,
	const std::span<ProgramResourceProps, 2> propNames,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>>& storage)
{
	NV_PROFILE_FUNC;

	const auto uniformBlocksCount = GL::GetProgramInterface(
		program,
		(ProgramInterface2)interface,
		ProgramInterfacePName::ActiveResources);

	for (auto i = 0; i < uniformBlocksCount; i++)
	{
		std::array<GLint, 2> props{};

		GL::GetProgramResource(
			program,
			(ProgramInterface2)interface,
			i,
			propNames,
			props);

		if (props[0] == -1)
			continue;

		auto resourceName = GL::GetProgramResourceName(
			program,
			interface,
			i,
			props[1]);
		NormalizeArrayResourceName(resourceName);

		storage.insert({ std::move(resourceName), props[0] });
	}
}

static void RetrieveStorageBlocksInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>>& storage)
{
	std::array<ProgramResourceProps, 2> propNames{ ProgramResourceProps::BufferBinding, ProgramResourceProps::NameLength };
	RetrieveProgramInterface(
		program,
		ProgramInterface1::ShaderStorageBlock,
		propNames,
		storage);
}

static void RetrieveUniformBlocksInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>>& storage)
{
	std::array<ProgramResourceProps, 2> propNames{ ProgramResourceProps::BufferBinding, ProgramResourceProps::NameLength };
	RetrieveProgramInterface(
		program,
		ProgramInterface1::UniformBlock,
		propNames,
		storage);
}

static void RetrieveInputInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>>& storage)
{
	std::array<ProgramResourceProps, 2> propNames{ ProgramResourceProps::Location, ProgramResourceProps::NameLength };
	RetrieveProgramInterface(
		program,
		ProgramInterface1::ProgramInput,
		propNames,
		storage);
	RetrieveProgramInterface(
		program,
		ProgramInterface1::Uniform,
		propNames,
		storage);
}

static std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> RetrieveProgramInterface(GLuint program)
{
	NV_PROFILE_FUNC;

	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> resources;

	RetrieveInputInterface(program, resources);
	RetrieveUniformBlocksInterface(program, resources);
	RetrieveStorageBlocksInterface(program, resources);

	return resources;
}

void ShaderProgram::ReleaseShaderCompiler() noexcept
{
	glReleaseShaderCompiler();
}

bool ShaderProgram::IsShaderBinarySupported() noexcept
{
	static auto s_SupportChecked = false;
	static auto s_IsSupported = false;
	if (s_SupportChecked)
	{
		return s_IsSupported;
	}

	GLint formatsCount = 0;
	glGetIntegerv(GL_SHADER_BINARY_FORMATS, &formatsCount);
	s_IsSupported = formatsCount != 0;

	return s_IsSupported;
}

bool ShaderProgram::IsProgramBinarySupported() noexcept
{
	static auto s_SupportChecked = false;
	static auto s_IsSupported = false;
	if (s_SupportChecked)
	{
		return s_IsSupported;
	}

	GLint formatsCount = 0;
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &formatsCount);
	s_IsSupported = formatsCount != 0;

	return s_IsSupported;
}

void ShaderProgram::Delete() noexcept
{
	NV_PROFILE_FUNC;

	glDeleteProgram(id_);
	id_ = 0;
}

void ShaderProgram::Use() const
{
	NV_PROFILE_FUNC;

	glUseProgram(id_);
}

ShaderProgram ShaderProgram::FromBinary(GLenum binaryFormat, const std::span<uint8_t> binary)
{
	NV_PROFILE_FUNC;

	return FromBinary(binaryFormat, binary.data(), binary.size_bytes());
}

ShaderProgram ShaderProgram::FromBinary(
	GLenum binaryFormat,
	const uint8_t* binary,
	size_t binarySize)
{
	NV_PROFILE_FUNC;

	if (!check_fits_in<GLsizei>(binarySize))
		throw std::overflow_error("Binary size exceeds max allowed by OpenGL.");

	ShaderProgram program;
	program.id_ = glCreateProgram();
	glProgramBinary(
		program.id_,
		binaryFormat,
		binary,
		(GLsizei)binarySize);

	CheckProgramLinkStatus(program.id_);

	program.resources_ = RetrieveProgramInterface(program.id_);

	return program;
}

ShaderProgram ShaderProgram::FromBinary(
	GLenum binaryFormat,
	const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	const auto [data, size] = File::ReadBinary(filepath);
	return FromBinary(binaryFormat, data.get(), size);
}

ShaderProgram::ShaderProgram(
	const ShaderStage *stages,
	size_t stagesCount)
{
	NV_PROFILE_FUNC;

	id_ = glCreateProgram();

	for (const auto &stage : std::span(stages, stagesCount))
		glAttachShader(id_, stage.GetID());

	{
		NV_PROFILE_SCOPE("::LinkShaderProgram");
		glLinkProgram(id_);
	}

	CleanUpAttachedShaders(id_);
	CheckProgramLinkStatus(id_);

	resources_ = RetrieveProgramInterface(id_);
}

std::pair<const std::span<std::byte>, GLenum> ShaderProgram::GetBinary()
{
	NV_PROFILE_FUNC;

	if (savedBinary_.has_value())
	{
		const auto &savedBinary = savedBinary_.value();
		return std::make_pair(
			std::span<std::byte>(savedBinary.Binary.get(), savedBinary.Size),
			savedBinary.Format);
	}

	if (!ShaderProgram::IsProgramBinarySupported())
		throw std::runtime_error("Current context doesn't support retrieving shader program binary.");

	savedBinary_ = RetrieveProgramBinary(id_);

	return ShaderProgram::GetBinary();
}

GLuint ShaderProgram::GetResourceLocation(const std::string_view name) const
{
	NV_PROFILE_FUNC;

	const auto location = TryGetResourceLocation(name);
	if (location)
		return location.value();

	throw std::runtime_error("Failed to find resource with given ID.");
}

std::optional<GLuint> ShaderProgram::TryGetResourceLocation(const std::string_view name) const
{
	NV_PROFILE_FUNC;

	const auto it = resources_.find(name);
	if (it == resources_.end())
		return std::nullopt;

	return it->second;
}

void ShaderProgram::SetUniform(const std::string_view name, float value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform1f(id_, GetResourceLocation(name), value);
}

void ShaderProgram::SetUniform(const std::string_view name, int32_t value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform1i(id_, GetResourceLocation(name), value);
}

void ShaderProgram::SetUniform(const std::string_view name, uint32_t value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform1ui(id_, GetResourceLocation(name), value);
}

void ShaderProgram::SetUniform(const std::string_view name, const glm::vec3& value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform3f(id_, GetResourceLocation(name), value.x, value.y, value.z);
}