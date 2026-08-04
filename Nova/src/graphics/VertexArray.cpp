#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/Build.hpp>

using namespace Nova;

static constexpr const std::string_view AttribTypeToString(AttributeType type) noexcept
{
	switch (type)
	{
	case AttributeType::Byte:
		return "Byte";
	case AttributeType::UnsignedByte:
		return "UnsignedByte";
	case AttributeType::Short:
		return "Short";
	case AttributeType::UnsignedShort:
		return "UnsignedShort";
	case AttributeType::Int:
		return "Int";
	case AttributeType::UnsignedInt:
		return "UnsignedInt";
	case AttributeType::Fixed:
		return "Fixed";
	case AttributeType::Float:
		return "Float";
	case AttributeType::HalfFloat:
		return "HalfFloat";
	case AttributeType::Double:
		return "Double";
	case AttributeType::Int2101010Rev:
		return "Int2101010Rev";
	case AttributeType::UnsignedInt2101010Rev:
		return "UnsignedInt2101010Rev";
	case AttributeType::UnsignedInt10F11F11FRev:
		return "UnsignedInt10F11F11FRev";
	}

	NV_UNREACHABLE;
}

VertexArray::VertexArray(VertexArray &&other) noexcept
	: bufferBindings_(std::move(other.bufferBindings_)),
	  usedBufferBindings_(std::move(other.usedBufferBindings_)),
	  id_(std::exchange(other.id_, 0)) {}

VertexArray::VertexArray(GLuint id) noexcept
	: id_(id) {}

VertexArray::VertexArray(std::span<const VertexInput> layout)
	: id_(GL::CreateVertexArray())
{
	NV_PROFILE_FUNC;

	for (const auto &vertexInput : layout)
		AddVertexInput(vertexInput);
}

VertexArray::VertexArray(
	std::span<const VertexInput> layout,
	const Buffer &elementBuffer)
	: VertexArray(layout)
{
	GL::VertexArrayElementBuffer(id_, elementBuffer.GetID());
}

VertexArray::VertexArray(std::initializer_list<VertexInput> layout)
	: VertexArray(std::span<const VertexInput>(layout.begin(), layout.end())) {}

VertexArray::VertexArray(
	std::initializer_list<VertexInput> layout,
	const Buffer &elementBuffer)
	: VertexArray(std::span<const VertexInput>(layout.begin(), layout.end()), elementBuffer) {}

void VertexArray::AddVertexInput(const VertexInput &vertexInput)
{
	AddVertexInput(
		vertexInput.Stride,
		vertexInput.Descriptors,
		vertexInput.BufferID.value_or(0),
		vertexInput.Offset,
		vertexInput.InstanceDivisor);
}

void VertexArray::AddVertexInput(
	GLuint stride,
	const std::vector<VertexDescriptor> &descriptors,
	GLuint bufferID,
	GLint offset,
	GLint instanceDivisor)
{
	NV_PROFILE_FUNC;

	GLuint bindingIndex;
	const auto &bindingEntry = bufferBindings_.find(bufferID);
	if (bindingEntry == bufferBindings_.end())
	{
		NV_LOG_WARNING("Vertex array {} is using automatic buffer binding resolution for buffer {}. To speed this up consider selecting buffer binding before using BindVertexBuffer or manually specify the binding.", (GLuint)id_, (GLuint)bufferID);

		bindingIndex = FindNextFreeBindingIndex();
		bufferBindings_[bufferID] = bindingIndex;
		usedBufferBindings_.emplace_back(bindingIndex);
	}
	else
	{
		bindingIndex = bindingEntry->second;
	}

	GL::VertexArrayVertexBuffer(
		id_,
		bindingIndex,
		bufferID,
		offset,
		stride);
	GL::VertexArrayBindingDivisor(
		id_,
		bindingIndex,
		instanceDivisor);

	NV_LOG_TRACE("VertexArray({}) Bound vertex buffer {} at index {}.", (GLuint)id_, (GLuint)bufferID, bindingIndex);

	GLuint attribOffset = 0;
	for (const auto &descriptor : descriptors)
	{
		NV_PROFILE_SCOPE("::AddVertexDescriptor");

		for (GLuint row = 0; row < descriptor.Rows; row++)
		{
			const auto attribIndex = descriptor.AttributeIndex + row;
			const auto offset = attribOffset + (row * descriptor.GetRowSize());

			GL::EnableVertexArrayAttrib(id_, attribIndex);
			GL::VertexArrayAttribBinding(id_, attribIndex, bindingIndex);

			switch (descriptor.AttributeType)
			{
			case AttributeType::Double:
				glVertexArrayAttribLFormat(
					id_,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					offset);
				break;
			case AttributeType::Fixed:
			case AttributeType::Float:
			case AttributeType::HalfFloat:
				glVertexArrayAttribFormat(
					id_,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					descriptor.IsNormalized,
					offset);
				break;
			default:
				glVertexArrayAttribIFormat(
					id_,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					offset);
				break;
			}
		}

		NV_LOG_TRACE(
			"VertexArray({}) Added vertex descriptor for buffer binding {}.\n\tIndex: {}\n\tOffset: {}\n\tType: {}\n\tCount: {}\n\tRows: {}\n\tNormalized: {}",
			id_,
			bindingIndex,
			descriptor.AttributeIndex,
			attribOffset,
			AttribTypeToString(descriptor.AttributeType),
			descriptor.Count,
			descriptor.Rows,
			descriptor.IsNormalized ? "True" : "False");

		attribOffset += descriptor.GetSize();
	}
}

void VertexArray::Use() const noexcept
{
	NV_PROFILE_FUNC;

	GL::BindVertexArray(id_);
}

void VertexArray::BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset)
{
	const auto &bindingEntry = bufferBindings_.find((GLuint)buffer.GetID());
	if (bindingEntry != bufferBindings_.end())
	{
		usedBufferBindings_.erase(
			std::remove(
				usedBufferBindings_.begin(),
				usedBufferBindings_.end(),
				bindingEntry->second));

		// NV_LOG_WARNING("Buffer with ID {} is already bound to binding index {}. Buffer will be rebound.", (GLuint)buffer.GetID(), bindingEntry->second);
	}

	bufferBindings_[(GLuint)buffer.GetID()] = bindingIndex;
	usedBufferBindings_.emplace_back(bindingIndex);
	GL::VertexArrayVertexBuffer(id_, bindingIndex, (GLuint)buffer.GetID(), offset, stride);
}

void VertexArray::BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset, GLuint instanceDivisor)
{
	BindVertexBuffer(buffer, bindingIndex, stride, offset);
	GL::VertexArrayBindingDivisor(id_, bindingIndex, instanceDivisor);
}

void VertexArray::BindElementBuffer(const Buffer &buffer) const noexcept
{
	GL::VertexArrayElementBuffer(id_, (GLuint)buffer.GetID());
}

void VertexArray::Delete() noexcept
{
	glDeleteVertexArrays(1, &id_);
	id_ = 0;
}

GLuint VertexArray::FindNextFreeBindingIndex()
{
	GLint maxBindingsCount = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &maxBindingsCount);

	for (GLuint i = 0; i < maxBindingsCount; i++)
	{
		const auto bindingUnused = std::find(
									   usedBufferBindings_.begin(),
									   usedBufferBindings_.end(),
									   i) == usedBufferBindings_.end();
		if (bindingUnused)
			return i;
	}

	throw std::runtime_error("All buffer bindings for this VAO are used.");
}