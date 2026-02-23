#include <Nova/graphics/opengl/VertexArray.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/core/Build.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>

using namespace Nova;

static constexpr const std::string_view AttribTypeToString(AttributeType type)
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

VertexArray::VertexArray(VertexArray&& other) noexcept
	: GLObject::GLObject(std::exchange(other.m_ID, 0))
{
	RegisterObject(GL_VERTEX_ARRAY, m_ID);
}

VertexArray::VertexArray(GLuint id) noexcept
	: GLObject::GLObject(id)
{
	RegisterObject(GL_VERTEX_ARRAY, m_ID);
}

VertexArray::VertexArray(std::span<const VertexInput> layout)
	: GLObject::GLObject(GL::CreateVertexArray())
{
	NV_PROFILE_FUNC;

	for (const auto &vertexInput : layout)
		AddVertexInput(vertexInput);
	
	RegisterObject(GL_VERTEX_ARRAY, m_ID);
}

VertexArray::VertexArray(
	std::span<const VertexInput> layout,
	const Buffer& elementBuffer)
	: VertexArray(layout)
{
	GL::VertexArrayElementBuffer(m_ID, (GLuint)elementBuffer.GetID());
}

VertexArray::VertexArray(std::initializer_list<VertexInput> layout)
	: VertexArray(std::span<const VertexInput>(layout.begin(), layout.end())) { }

VertexArray::VertexArray(
	std::initializer_list<VertexInput> layout,
	const Buffer& elementBuffer)
	: VertexArray(std::span<const VertexInput>(layout.begin(), layout.end()), elementBuffer) {  }

void VertexArray::AddVertexInput(const VertexInput& vertexInput)
{
	AddVertexInput(
		vertexInput.Stride,
		vertexInput.Descriptors,
		vertexInput.BufferID.value_or(BufferID(0)),
		vertexInput.Offset,
		vertexInput.InstanceDivisor);
}

void VertexArray::AddVertexInput(
	GLuint stride,
	const std::vector<VertexDescriptor>& descriptors,
	BufferID bufferID,
	GLint offset,
	GLint instanceDivisor)
{
	NV_PROFILE_FUNC;

	GLuint bindingIndex;
	const auto& bindingEntry = m_BufferBindings.find((GLuint)bufferID);
	if (bindingEntry == m_BufferBindings.end())
	{
		NV_LOG_WARNING("Vertex array {} is using automatic buffer binding resolution for buffer {}. To speed this up consider selecting buffer binding before using BindVertexBuffer or manually specify the binding.", (GLuint)m_ID, (GLuint)bufferID);

		bindingIndex = FindNextFreeBindingIndex();
		m_BufferBindings[(GLuint)bufferID] = bindingIndex;
		m_UsedBufferBindings.emplace_back(bindingIndex);
	}
	else
	{
		bindingIndex = bindingEntry->second;
	}

	GL::VertexArrayVertexBuffer(
		m_ID,
		bindingIndex,
		(GLuint)bufferID,
		offset,
		stride);
	GL::VertexArrayBindingDivisor(
		m_ID,
		bindingIndex,
		instanceDivisor);

	NV_LOG_TRACE("VertexArray({}) Bound vertex buffer {} at index {}.", (GLuint)m_ID, (GLuint)bufferID, bindingIndex);

	GLuint attribOffset = 0;
	for (const auto& descriptor : descriptors)
	{
		NV_PROFILE_SCOPE("::AddVertexDescriptor");

		for (GLuint row = 0; row < descriptor.Rows; row++)
		{
			const auto attribIndex = descriptor.AttributeIndex + row;
			const auto offset = attribOffset + (row * descriptor.GetRowSize());

			GL::EnableVertexArrayAttrib(m_ID, attribIndex);
			GL::VertexArrayAttribBinding(m_ID, attribIndex, bindingIndex);

			switch (descriptor.AttributeType)
			{
			case AttributeType::Double:
				glVertexArrayAttribLFormat(
					(GLuint)m_ID,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					offset);
				break;
			case AttributeType::Fixed:
			case AttributeType::Float:
			case AttributeType::HalfFloat:
				glVertexArrayAttribFormat(
					(GLuint)m_ID,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					descriptor.IsNormalized,
					offset);
				break;
			default:
				glVertexArrayAttribIFormat(
					(GLuint)m_ID,
					attribIndex,
					descriptor.Count,
					(GLenum)descriptor.AttributeType,
					offset);
				break;
			}
		}

		NV_LOG_TRACE(
			"VertexArray({}) Added vertex descriptor for buffer binding {}.\n\tIndex: {}\n\tOffset: {}\n\tType: {}\n\tCount: {}\n\tRows: {}\n\tNormalized: {}",
			(GLuint)m_ID,
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

	GL::BindVertexArray(m_ID);
}

void VertexArray::BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset)
{
	const auto& bindingEntry = m_BufferBindings.find((GLuint)buffer.GetID());
	if (bindingEntry != m_BufferBindings.end())
	{
		m_UsedBufferBindings.erase(
			std::remove(
				m_UsedBufferBindings.begin(),
				m_UsedBufferBindings.end(),
				bindingEntry->second));

		// NV_LOG_WARNING("Buffer with ID {} is already bound to binding index {}. Buffer will be rebound.", (GLuint)buffer.GetID(), bindingEntry->second);
	}
	
	m_BufferBindings[(GLuint)buffer.GetID()] = bindingIndex;
	m_UsedBufferBindings.emplace_back(bindingIndex);
	GL::VertexArrayVertexBuffer(m_ID, bindingIndex, (GLuint)buffer.GetID(), offset, stride);
}

void VertexArray::BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset, GLuint instanceDivisor)
{
	BindVertexBuffer(buffer, bindingIndex, stride, offset);
	GL::VertexArrayBindingDivisor(m_ID, bindingIndex, instanceDivisor);
}

void VertexArray::BindElementBuffer(const Buffer &buffer) const noexcept
{
	GL::VertexArrayElementBuffer(m_ID, (GLuint)buffer.GetID());
}

void VertexArray::Delete() noexcept
{
	glDeleteVertexArrays(1, &m_ID);
	m_ID = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
	m_ID = std::exchange(other.m_ID, 0);
	return *this;
}

GLuint VertexArray::FindNextFreeBindingIndex()
{
	GLint maxBindingsCount = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &maxBindingsCount);

	for (GLuint i = 0; i < maxBindingsCount; i++)
	{
		const auto bindingUnused = std::find(
			m_UsedBufferBindings.begin(),
			m_UsedBufferBindings.end(),
			i) == m_UsedBufferBindings.end();
		if (bindingUnused)
			return i;
	}

	throw std::runtime_error("All buffer bindings for this VAO are used.");
}