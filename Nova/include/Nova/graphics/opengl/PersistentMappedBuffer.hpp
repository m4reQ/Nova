#pragma once
#include <Nova/core/Utility.hpp>
#include <Nova/graphics/opengl/GL.hpp>
#include <utility>
#include <span>

namespace Nova
{
    enum class BufferAccessFlags : GLenum
    {
        None = 0,
        Readable = GL_MAP_READ_BIT,
        Writable = GL_MAP_WRITE_BIT,
    };

    NV_DEFINE_BITWISE_OPERATORS(BufferAccessFlags);

    class PersistentMappedBuffer
    {
    public:
        PersistentMappedBuffer() = default;

        PersistentMappedBuffer(const PersistentMappedBuffer &other);

        constexpr PersistentMappedBuffer(PersistentMappedBuffer &&other) noexcept
            : id_(std::exchange(other.id_, 0)),
              dataBase_(other.dataBase_),
              dataCurrent_(other.dataCurrent_),
              size_(other.size_) { }

        PersistentMappedBuffer(GLsizeiptr size, BufferAccessFlags accessFlags, const void *data = nullptr);

        ~PersistentMappedBuffer() noexcept;

        GLsizeiptr Commit(bool wholeBuffer = false) noexcept;

        void Commit(GLintptr offset, GLsizeiptr length) noexcept;

        void Discard() noexcept;

        void Write(const void* data, GLsizeiptr dataSize) noexcept;

        template <typename T>
        void Write(const std::span<T> data) noexcept
        {
            Write(data.data(), data.size_bytes());
        }

        void Bind(BufferBindTarget target) const noexcept;

        void Bind(BufferBaseTarget target, GLuint index) const noexcept;

        void Bind(BufferBaseTarget target, GLuint index, GLintptr offset, GLsizeiptr size) const noexcept;

        void SetDebugName(const std::string_view debugName) const noexcept;

        constexpr GLuint GetID() const noexcept { return id_; }

        constexpr GLsizeiptr GetSize() const noexcept { return size_; }

        constexpr GLsizeiptr GetDataSize() const noexcept { return (std::byte*)dataCurrent_ - (std::byte*)dataBase_; }

        constexpr void* GetDataPtr() noexcept { return dataCurrent_; }

        constexpr const void* GetDataPtr() const noexcept { return dataCurrent_; }

        template <typename T>
        constexpr T* GetDataPtr() noexcept { return reinterpret_cast<T*>(dataCurrent_); }

        template <typename T>
        constexpr const T* GetDataPtr() const noexcept { return reinterpret_cast<const T*>(dataCurrent_); }

        constexpr void* GetBasePtr(GLsizeiptr offsetBytes = 0) noexcept { return (uint8_t*)dataBase_ + offsetBytes; }

        constexpr const void* GetBasePtr(GLsizeiptr offsetBytes = 0) const noexcept { return (const uint8_t*)dataBase_ + offsetBytes; }

        template <typename T>
        constexpr T* GetBasePtr(GLsizeiptr offsetBytes = 0) noexcept { return (T*)((uint8_t*)dataBase_ + offsetBytes); }

        template <typename T>
        constexpr const T* GetBasePtr(GLsizeiptr offsetBytes = 0) const noexcept { return (const T*)((const uint8_t*)dataBase_ + offsetBytes); }

        PersistentMappedBuffer& operator=(const PersistentMappedBuffer& other);

        constexpr PersistentMappedBuffer& operator=(PersistentMappedBuffer&& other) noexcept
        {
            id_ = std::exchange(other.id_, 0);
            size_ = other.size_;
            dataBase_ = other.dataBase_;
            dataCurrent_ = other.dataCurrent_;

            return *this;
        }

    private:
        GLsizeiptr size_;
        void* dataBase_;
        void* dataCurrent_;
        GLuint id_;
    };
}