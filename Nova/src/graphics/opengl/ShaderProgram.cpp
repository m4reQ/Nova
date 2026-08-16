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

static void CleanUpAttachedShaders(GLuint program) noexcept
{
	NV_PROFILE_FUNC;

	GLint stagesCount = 0;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &stagesCount);

	const auto attachedShaders = std::make_unique<GLuint[]>(stagesCount);
	glGetAttachedShaders(program, stagesCount, nullptr, attachedShaders.get());

	for (auto i = 0zu; i < stagesCount; i++)
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
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> &storage)
{
	NV_PROFILE_FUNC;

	const auto uniformBlocksCount = GL::GetProgramInterface(
		program,
		static_cast<ProgramInterface2>(interface),
		ProgramInterfacePName::ActiveResources);

	for (auto i = 0; i < uniformBlocksCount; i++)
	{
		std::array<GLint, 2> props{};

		GL::GetProgramResource(
			program,
			static_cast<ProgramInterface2>(interface),
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

		storage.insert({std::move(resourceName), props[0]});
	}
}

static void RetrieveStorageBlocksInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> &storage)
{
	NV_PROFILE_FUNC;

	std::array<ProgramResourceProps, 2> propNames{ProgramResourceProps::BufferBinding, ProgramResourceProps::NameLength};
	RetrieveProgramInterface(
		program,
		ProgramInterface1::ShaderStorageBlock,
		propNames,
		storage);
}

static void RetrieveUniformBlocksInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> &storage)
{
	NV_PROFILE_FUNC;

	std::array<ProgramResourceProps, 2> propNames{ProgramResourceProps::BufferBinding, ProgramResourceProps::NameLength};
	RetrieveProgramInterface(
		program,
		ProgramInterface1::UniformBlock,
		propNames,
		storage);
}

static void RetrieveInputInterface(
	GLuint program,
	std::unordered_map<std::string, GLuint, StringHash, std::equal_to<>> &storage)
{
	NV_PROFILE_FUNC;

	std::array<ProgramResourceProps, 2> propNames{ProgramResourceProps::Location, ProgramResourceProps::NameLength};
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

static void SetProgramBinary(GLuint program, std::span<const std::byte> binary, GLenum binaryFormat)
{
	NV_PROFILE_FUNC;

	assert_fits_in<GLsizei>(binary.size_bytes());
	glProgramBinary(
		program,
		binaryFormat,
		binary.data(),
		static_cast<GLsizei>(binary.size_bytes()));
}

static GLuint CreateShaderStage(ShaderType type, const std::string_view source)
{
	NV_PROFILE_FUNC;

	const auto stage = glCreateShader(static_cast<GLenum>(type));
	const auto sourceData = source.data();
	const auto length = source.size();
	assert_fits_in<GLint>(length);

	const auto _length = static_cast<GLint>(source.size());
	glShaderSource(stage, 1, &sourceData, &_length);
	glCompileShader(stage);

	return stage;
}

static GLuint CreateShaderStage(const SourceShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	return CreateShaderStage(stageInfo.Type, stageInfo.Source);
}

static GLuint CreateShaderStage(const SourceFileShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	const auto source = File::ReadWholeText(stageInfo.Filepath);
	return CreateShaderStage(stageInfo.Type, std::string_view(source.data(), source.size()));
}

static GLuint CreateShaderStage(ShaderType type, std::span<const std::byte> binary, GLenum binaryFormat)
{
	NV_PROFILE_FUNC;

	if (!ShaderProgram::IsShaderBinarySupported())
		throw std::runtime_error("Binary shader stages are not supported by the current OpenGL context.");

	assert_fits_in<GLsizei>(binary.size_bytes());

	const auto stage = glCreateShader(static_cast<GLenum>(type));
	glShaderBinary(
		1,
		&stage,
		binaryFormat,
		binary.data(),
		static_cast<GLsizei>(binary.size_bytes()));

	return stage;
}

static GLuint CreateShaderStage(const BinaryShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	return CreateShaderStage(stageInfo.Type, stageInfo.Binary, stageInfo.BinaryFormat);
}

static GLuint CreateShaderStage(const BinaryFileShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	const auto binary = File::ReadWholeBinary(stageInfo.Filepath);
	return CreateShaderStage(stageInfo.Type, binary, stageInfo.BinaryFormat);
}

static GLuint CreateShaderStage(ShaderType type, std::span<const std::byte> binary, const std::optional<ShaderSpecializeInfo> &specializeInfo)
{
	NV_PROFILE_FUNC;

	assert_fits_in<GLsizei>(binary.size_bytes());

	const auto stage = glCreateShader(static_cast<GLenum>(type));
	glShaderBinary(
		1,
		&stage,
		GL_SHADER_BINARY_FORMAT_SPIR_V,
		binary.data(),
		static_cast<GLsizei>(binary.size_bytes()));

	if (specializeInfo.has_value())
	{
		const auto &specializeInfo_ = specializeInfo.value();
		if (specializeInfo_.ConstantIndices.size() != specializeInfo_.ConstantValues.size())
			throw std::runtime_error("Specialize contants indices and values count must match.");

		assert_fits_in<GLuint>(specializeInfo_.ConstantIndices.size());

		glSpecializeShader(
			stage,
			specializeInfo_.EntryPoint.data(),
			static_cast<GLuint>(specializeInfo_.ConstantIndices.size()),
			specializeInfo_.ConstantIndices.data(),
			specializeInfo_.ConstantValues.data());
	}

	return stage;
}

static GLuint CreateShaderStage(const SPIRVShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	return CreateShaderStage(stageInfo.Type, stageInfo.Binary, stageInfo.SpecializeInfo);
}

static GLuint CreateShaderStage(const SPIRVFileShaderStage &stageInfo)
{
	NV_PROFILE_FUNC;

	const auto binary = File::ReadWholeBinary(stageInfo.Filepath);
	return CreateShaderStage(stageInfo.Type, binary, stageInfo.SpecializeInfo);
}

static ProgramDependency DependencyFromStage(const BinaryFileShaderStage &stage) noexcept
{
	return ProgramDependency{
		.Timestamp = std::filesystem::last_write_time(stage.Filepath),
		.Filepath = stage.Filepath,
		.FileSize = std::filesystem::file_size(stage.Filepath),
		.Data = BinaryStageDependency{
			.ShaderType = stage.Type,
			.BinaryFormat = stage.BinaryFormat,
		}};
}

static ProgramDependency DependencyFromStage(const SourceFileShaderStage &stage) noexcept
{
	return ProgramDependency{
		.Timestamp = std::filesystem::last_write_time(stage.Filepath),
		.Filepath = stage.Filepath,
		.FileSize = std::filesystem::file_size(stage.Filepath),
		.Data = BinaryStageDependency{
			.ShaderType = stage.Type,
		}};
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

Nova::ShaderProgram::~ShaderProgram() noexcept
{
	if (id_)
		glDeleteProgram(id_);
}

void ShaderProgram::Use() const
{
	NV_PROFILE_FUNC;

	glUseProgram(id_);
}

ShaderProgram::ShaderProgram(GLID id)
	: id_(id),
	  resources_(RetrieveProgramInterface(id)) {}

ShaderProgram::ShaderProgram(std::span<const std::byte> binary, GLenum binaryFormat)
{
	NV_PROFILE_FUNC;

	if (!IsProgramBinarySupported())
		throw std::runtime_error("Binary shader programs are not supported by the current OpenGL context.");

	id_ = glCreateProgram();

	SetProgramBinary(id_, binary, binaryFormat);
	CheckProgramLinkStatus(id_);
	resources_ = RetrieveProgramInterface(id_);
}

ShaderProgram::ShaderProgram(std::filesystem::path binaryFile, GLenum binaryFormat)
{
	NV_PROFILE_FUNC;

	if (!IsProgramBinarySupported())
		throw std::runtime_error("Binary shader programs are not supported by the current OpenGL context.");

	id_ = glCreateProgram();

	SetProgramBinary(id_, File::ReadWholeBinary(binaryFile), binaryFormat);
	CheckProgramLinkStatus(id_);
	resources_ = RetrieveProgramInterface(id_);

	// TODO Maybe initialize dependencies_ with single dependency in initializer list
	dependencies_.emplace_back(
		ProgramDependency{
			.Timestamp = std::filesystem::last_write_time(binaryFile),
			.Filepath = binaryFile,
			.FileSize = std::filesystem::file_size(binaryFile),
			.Data = BinaryProgramDependency{
				.BinaryFormat = binaryFormat,
			}});
}

ShaderProgram::ShaderProgram(std::span<const ShaderStage> stages, std::span<const OutputLocation> outputs)
	: id_(glCreateProgram())
{
	NV_PROFILE_FUNC;

	for (const auto &stageInfo : stages)
	{
		const auto stage = std::visit(
			[](const auto &info)
			{
				return CreateShaderStage(info);
			},
			stageInfo);
		CheckShaderStatus(stage);
		glAttachShader(id_, stage);
	}

	for (const auto &output : outputs)
		glBindFragDataLocation(id_, output.Location, output.Name.c_str());

	glLinkProgram(id_);

	CleanUpAttachedShaders(id_);
	CheckProgramLinkStatus(id_);

	resources_ = RetrieveProgramInterface(id_);

	// TODO Maybe initialize dependencies_ with all dependencies in initializer list
	for (const auto &stage : stages)
	{
		ProgramDependency dependency;

		if (std::holds_alternative<BinaryFileShaderStage>(stage))
			dependency = DependencyFromStage(std::get<BinaryFileShaderStage>(stage));
		else if (std::holds_alternative<SourceFileShaderStage>(stage))
			dependency = DependencyFromStage(std::get<SourceFileShaderStage>(stage));

		dependencies_.emplace_back(std::move(dependency));
	}
}

ShaderProgram::ShaderProgram(std::initializer_list<ShaderStage> stages, std::initializer_list<OutputLocation> outputs)
	: ShaderProgram(std::span(stages), std::span(outputs)) {}

std::pair<std::vector<std::byte>, GLenum> ShaderProgram::GetBinary() const
{
	NV_PROFILE_FUNC;

	if (!ShaderProgram::IsProgramBinarySupported())
		throw std::runtime_error("Current OpenGL context doesn't support retrieving shader program binary.");

	GLint binarySize = 0;
	glGetProgramiv(id_, GL_PROGRAM_BINARY_LENGTH, &binarySize);

	GLenum binaryFormat = 0;
	std::vector<std::byte> binary(binarySize);
	glGetProgramBinary(
		id_,
		binarySize,
		nullptr,
		&binaryFormat,
		binary.data());

	return std::make_pair(std::move(binary), binaryFormat);
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

void ShaderProgram::SetUniform(const std::string_view name, const glm::vec3 &value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform3f(id_, GetResourceLocation(name), value.x, value.y, value.z);
}

void ShaderProgram::SetUniform(const std::string_view name, const glm::vec2 &value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform2f(id_, GetResourceLocation(name), value.x, value.y);
}

void ShaderProgram::SetUniform(const std::string_view name, const glm::vec4 &value) const
{
	NV_PROFILE_FUNC;
	glProgramUniform4f(id_, GetResourceLocation(name), value.x, value.y, value.z, value.w);
}

void ShaderProgram::SetUniform(const std::string_view name, GLuint64 value) const
{
	NV_PROFILE_FUNC;
	glProgramUniformHandleui64ARB(id_, GetResourceLocation(name), value);
}

void ShaderProgram::SetDebugName(const std::string_view name) const noexcept
{
	GL::ObjectLabel(ObjectIdentifier::Program, id_, name);
}