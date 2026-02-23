#include <Nova/graphics/opengl/PersistentMappedBuffer.hpp>
#include <Nova/graphics/opengl/GL.hpp>

using namespace Nova;

PersistentMappedBuffer::PersistentMappedBuffer(const PersistentMappedBuffer& other)
    : id_(GL::CreateBuffer())
{
    GL::CopyNamedBufferSubData(other.id_, id_, 0, 0, other.GetSize());
}

PersistentMappedBuffer::PersistentMappedBuffer(GLsizeiptr size, BufferAccessFlags accessFlags, const void *data)
    : id_(GL::CreateBuffer()),
      size_(size)
{
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
    glFlushMappedNamedBufferRange(id_, 0, GetDataSize());
    dataCurrent_ = dataBase_;
}

void PersistentMappedBuffer::Discard() noexcept
{
    dataCurrent_ = dataBase_;
}

void PersistentMappedBuffer::Write(std::span<const uint8_t> data) noexcept
{
    Write(data.data(), (GLsizeiptr)data.size_bytes());
}

void PersistentMappedBuffer::Write(const void* data, GLsizeiptr dataSize) noexcept
{
    std::memcpy(dataCurrent_, data, dataSize);
    dataCurrent_ = (uint8_t*)dataCurrent_ + dataSize;
}