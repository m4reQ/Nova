#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

Buffer::Buffer(const Buffer &other)
{
    NV_PROFILE_FUNC;

    id_ = GL::CreateBuffer();
    GL::CopyNamedBufferSubData(other.id_, id_, 0, 0, other.GetSize());
}

Buffer::Buffer(GLsizeiptr size, BufferAccessFlags accessFlags, const void *data)
    : size_(size)
{
    NV_PROFILE_FUNC;

    id_ = GL::CreateBuffer();

    auto storageFlags = static_cast<BufferStorageFlags>(accessFlags);
    if (accessFlags != BufferAccessFlags::None)
        Flag::Set(storageFlags, BufferStorageFlags::MapPersistentBit);

    GL::NamedBufferStorage(id_, size, data, storageFlags);

    auto mapFlags = static_cast<BufferMapFlags>(accessFlags);
    if (accessFlags != BufferAccessFlags::None)
    {
        Flag::Set(mapFlags, BufferMapFlags::MapPersistentBit | BufferMapFlags::MapFlushExplicitBit | BufferMapFlags::MapUnsynchronizedBit);
        dataBase_ = GL::MapNamedBufferRange(id_, 0, size, mapFlags);
        dataCurrent_ = dataBase_;
    }
}

Buffer::~Buffer() noexcept
{
    if (id_)
        GL::DeleteBuffer(id_);
}

GLsizeiptr Buffer::Commit(bool wholeBuffer) noexcept
{
    NV_PROFILE_FUNC;

    const auto flushedDataSize = wholeBuffer ? GetSize() : GetDataSize();

    if (flushedDataSize > 0)
        glFlushMappedNamedBufferRange(id_, 0, flushedDataSize);

    dataCurrent_ = dataBase_;

    return flushedDataSize;
}

void Buffer::Commit(GLintptr offset, GLsizeiptr length) noexcept
{
    NV_PROFILE_FUNC;
    glFlushMappedNamedBufferRange(id_, offset, length);
}

void Buffer::Discard() noexcept
{
    dataCurrent_ = dataBase_;
}

void Buffer::Write(const void *data, GLsizeiptr dataSize) noexcept
{
    NV_PROFILE_FUNC;

    std::memcpy(dataCurrent_, data, dataSize);
    dataCurrent_ = (uint8_t *)dataCurrent_ + dataSize;
}

void Buffer::Bind(BufferBindTarget target) const noexcept
{
    NV_PROFILE_FUNC;

    GL::BindBuffer(target, id_);
}

void Buffer::Bind(BufferBaseTarget target, GLuint index) const noexcept
{
    NV_PROFILE_FUNC;

    GL::BindBufferBase(target, index, id_);
}

void Buffer::Bind(BufferBaseTarget target, GLuint index, GLintptr offset, GLsizeiptr size) const noexcept
{
    NV_PROFILE_FUNC;

    GL::BindBufferRange(target, index, id_, offset, size);
}

void Buffer::SetDebugName(const std::string_view debugName) const
{
    NV_PROFILE_FUNC;
    GL::ObjectLabel(ObjectIdentifier::Buffer, id_, debugName);
}