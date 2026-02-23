#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/core/Build.hpp>
#include <Nova/debug/Profile.hpp>
#include <stdexcept>

using namespace Nova;

constexpr static GLenum GetBufferStorageFlags(bool isReadable, bool isWritable, bool isCoherent) noexcept
{
	if (!isReadable && !isWritable)
	{
		return 0;
	}

	return GL_MAP_PERSISTENT_BIT | (isCoherent ? GL_MAP_COHERENT_BIT : 0) | (isReadable ? GL_MAP_READ_BIT : 0) | (isWritable ? GL_MAP_WRITE_BIT : 0);
}

constexpr static GLenum GetBufferMapFlags(bool isReadable, bool isWritable, bool isCoherent, bool isUnsynchronized) noexcept
{
	return GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | (isReadable ? GL_MAP_READ_BIT : 0) | (isWritable ? GL_MAP_WRITE_BIT : 0) | (isCoherent ? GL_MAP_COHERENT_BIT : 0) | (isUnsynchronized ? GL_MAP_UNSYNCHRONIZED_BIT : 0);
}

Buffer::Buffer(
	GLsizeiptr size,
	bool isWritable,
	bool isReadable,
	const void *data)
	: m_Size(size),
	  m_IsReadable(isReadable),
	  m_IsWritable(isWritable)
{
	NV_PROFILE_FUNC;

	glCreateBuffers(1, &m_ID);
	glNamedBufferStorage(m_ID, size, data, GetBufferStorageFlags(isReadable, isWritable, true));

	if (isReadable || isWritable)
	{
		m_BaseDataPtr = glMapNamedBufferRange(m_ID, 0, size, GetBufferMapFlags(isReadable, isWritable, true, false));
		m_DataPtr = m_BaseDataPtr;
	}

	RegisterObject(GL_BUFFER, m_ID);
}

void Buffer::Store(const void *data, std::size_t sizeBytes) noexcept
{
	// TODO Check if data ptr is null (then buffer is not writable)
	std::memcpy(GetDataPtr(), data, sizeBytes);
	m_DataPtr = reinterpret_cast<std::uint8_t *>(m_DataPtr) + sizeBytes;
}

void Buffer::Delete() noexcept
{
	glDeleteBuffers(1, &m_ID);
	m_ID = 0;
}

void Buffer::Bind(BindingTarget target) const
{
	glBindBuffer(static_cast<GLenum>(target), m_ID);
}

void Buffer::Bind(BindingTarget target, GLuint index) const
{
	glBindBufferBase(static_cast<GLenum>(target), index, m_ID);
}

void Buffer::Bind(BindingTarget target, GLuint index, GLintptr offset, GLsizeiptr size) const
{
	glBindBufferRange(static_cast<GLenum>(target), index, m_ID, offset, size);
}

void Buffer::Commit() noexcept
{
	Commit(GetDataSize(), 0);
}

void Buffer::Commit(GLsizeiptr sizeBytes, GLsizeiptr offset) noexcept
{
	glFlushMappedNamedBufferRange(m_ID, offset, sizeBytes);
	m_DataPtr = m_BaseDataPtr;
}

void Buffer::AdvanceDataPtr(GLsizeiptr nBytes) noexcept
{
	// TODO Check for overflow
	m_DataPtr = reinterpret_cast<std::uint8_t *>(m_DataPtr) + nBytes;
}
