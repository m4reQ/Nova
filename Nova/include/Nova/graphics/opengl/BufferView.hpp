#pragma once
#include <glad/gl.h>
#include <type_traits>
#include <utility>

namespace Nova
{
    template <typename T>
    class BufferView
    {
    public:
        constexpr BufferView(GLuint bufferID, void* data, GLsizeiptr size, GLintptr offset) noexcept
            : data_(data),
              size_(size),
              offset_(offset),
              bufferID_(bufferID) { }

        BufferView(const BufferView&) = delete;

        constexpr BufferView(BufferView<T>&& other) noexcept
            : data_(std::exchange(other.data_, nullptr)),
              size_(other.size_),
              offset_(other.offset_),
              bufferID_(other.bufferID_) { }

#ifdef NV_BUFFER_VIEW_AUTO_COMMIT
        ~BufferView() noexcept
        {
            if (data_)
                Commit();
        }
#endif

        void Commit() const noexcept
        {
            if constexpr (std::is_same_v<T, void>)
                glFlushMappedNamedBufferRange(bufferID_, offset_, size_);
            else
                glFlushMappedNamedBufferRange(bufferID_, offset_, size_ * sizeof(T));
        }

        constexpr GLuint GetBufferID() const noexcept { return bufferID_; }

        constexpr std::span<T> GetView() noexcept { return std::span(data_, size_); }

        constexpr std::span<const T> GetView() const noexcept { return std::span(data_, size_); }

        constexpr T& operator[](GLsizeiptr i) noexcept { return *(data_ + i); }

        constexpr const T& operator[](GLsizeiptr i) const noexcept { return *(data_ + i); }

        constexpr T* begin() noexcept { return data_; }
        
        constexpr const T* begin() const noexcept { return data_; }

        constexpr T* end() noexcept { return data_ + size_; }

        constexpr const T* end() const noexcept { return data_ + size_; }

        constexpr GLsizeiptr size() const noexcept { return size_; }

        constexpr GLsizeiptr size_bytes() const noexcept
        {
            if constexpr (std::is_same_v<T, void>)
                return size_;
            else
                return size_ * sizeof(T);
        }

        constexpr T* data() noexcept { return data_; }

        constexpr const T* data() const noexcept { return data_; }

        constexpr BufferView<T>& operator=(const BufferView<T>& other) = delete;

        constexpr BufferView<T>& operator=(BufferView<T>&& other) noexcept
        {
            data_ = std::exchange(other.data_, nullptr);
            size_ = other.size_;
            offset_ = other.offset_;
            bufferID_ = other.bufferID_;

            return *this;
        }

    private:
        T* data_;
        GLsizeiptr size_;
        GLintptr offset_;
        GLuint bufferID_;
    };
}