#include <Nova/graphics/opengl/PersistentMappedBuffer.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

PersistentMappedBuffer::PersistentMappedBuffer(const PersistentMappedBuffer& other)
{
    NV_PROFILE_FUNC;

    id_ = GL::CreateBuffer();
    GL::CopyNamedBufferSubData(other.id_, id_, 0, 0, other.GetSize());
}

PersistentMappedBuffer::PersistentMappedBuffer(GLsizeiptr size, BufferAccessFlags accessFlags, const void *data)
    : size_(size)
{
    NV_PROFILE_FUNC;

    id_ = GL::CreateBuffer();

    const auto storageFlags = (BufferStorageFlags)accessFlags
        | BufferStorageFlags::MapPersistentBit;
    GL::NamedBufferStorage(id_, size, data, storageFlags);

    const auto mapFlags = (BufferMapFlags)accessFlags
        | BufferMapFlags::MapPersistentBit
        | BufferMapFlags::MapFlushExplicitBit
        | BufferMapFlags::MapUnsynchronizedBit;
    dataBase_ = GL::MapNamedBufferRange(id_, 0, size, mapFlags);
    dataCurrent_ = dataBase_;
}

PersistentMappedBuffer::~PersistentMappedBuffer() noexcept
{
    glDeleteBuffers(1, &id_);
}

GLsizeiptr PersistentMappedBuffer::Commit() noexcept
{
    NV_PROFILE_FUNC;

    const auto flushedDataSize = GetDataSize();
    
    glFlushMappedNamedBufferRange(id_, 0, flushedDataSize);
    dataCurrent_ = dataBase_;

    return flushedDataSize;
}

void PersistentMappedBuffer::Commit(GLintptr offset, GLsizeiptr length) noexcept
{
    NV_PROFILE_FUNC;
    glFlushMappedNamedBufferRange(id_, offset, length);
}

void PersistentMappedBuffer::Discard() noexcept
{
    dataCurrent_ = dataBase_;
}

void PersistentMappedBuffer::Write(const void* data, GLsizeiptr dataSize) noexcept
{
    NV_PROFILE_FUNC;
    
    std::memcpy(dataCurrent_, data, dataSize);
    dataCurrent_ = (uint8_t*)dataCurrent_ + dataSize;
}

void PersistentMappedBuffer::Bind(BufferBindTarget target) const noexcept
{
    NV_PROFILE_FUNC;
    
    GL::BindBuffer(target, id_);
}

void PersistentMappedBuffer::Bind(BufferBaseTarget target, GLuint index) const noexcept
{
    NV_PROFILE_FUNC;
    
    GL::BindBufferBase(target, index, id_);
}

void PersistentMappedBuffer::Bind(BufferBaseTarget target, GLuint index, GLintptr offset, GLsizeiptr size) const noexcept
{
    NV_PROFILE_FUNC;
    
    GL::BindBufferRange(target, index, id_, offset, size);
}

void PersistentMappedBuffer::SetDebugName(const std::string_view debugName) const noexcept
{
    glObjectLabel(GL_BUFFER, id_, debugName.size(), debugName.data());
}