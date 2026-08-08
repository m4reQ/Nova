#pragma once
#include <glad/gl.h>

namespace Nova
{
    class Buffer;

    template <typename T>
    class BufferRegion
    {
    public:
        BufferRegion() = delete;

        BufferRegion(const BufferRegion &) = delete;

        BufferRegion(BufferRegion &&) noexcept = default;

        constexpr BufferRegion(Buffer &buffer, GLintptr offsetBytes, GLsizei capacityBytes, T *data) noexcept
            : buffer_(buffer),
              bufferOffset_(offsetBytes),
              capacity_(capacityBytes),
              base_(data),
              current_(data) {}

        constexpr void Increment() noexcept { current_++; }

        constexpr bool IsEmpty() const noexcept { return current_ == base_; }

        constexpr bool IsFull() const noexcept { return GetSize() == GetCapacity(); }

        constexpr GLsizei GetCapacity() const noexcept { return capacity_ / sizeof(T); }

        constexpr GLsizei GetCapacityBytes() const noexcept { return capacity_; }

        constexpr GLsizei GetSize() const noexcept
        {
            const auto size = current_ - base_;
            assert_fits_in<GLsizei>(size);
            return static_cast<GLsizei>(size);
        }

        constexpr GLsizei GetSizeBytes() const noexcept
        {
            const auto size = (current_ - base_) * sizeof(T);
            assert_fits_in<GLsizei>(size);
            return static_cast<GLsizei>(size);
        }

        constexpr GLintptr GetBufferOffset() const noexcept { return bufferOffset_; }

        constexpr const Buffer &GetBuffer() const noexcept { return buffer_; }

        constexpr Buffer &GetBuffer() noexcept { return buffer_; }

        constexpr const T *GetBasePtr() const noexcept { return base_; }

        constexpr T *GetBasePtr() noexcept { return base_; }

        constexpr const T *GetCurrentPtr() const noexcept { return current_; }

        constexpr T *GetCurrentPtr() noexcept { return current_; }

        GLsizei Commit() noexcept
        {
            const auto commitedSize = GetSize();
            buffer_.Commit(bufferOffset_, GetSizeBytes());
            current_ = base_;

            return commitedSize;
        }

        void Copy(std::span<const T> data)
        {
            std::memcpy(current_, data.data(), data.size_bytes());
            current_ += data.size();
        }

        constexpr void Reset() noexcept { current_ = base_; }

        BufferRegion &operator=(const BufferRegion &) = delete;

        BufferRegion &operator=(BufferRegion &&) noexcept = default;

    private:
        Buffer &buffer_;
        GLintptr bufferOffset_;
        GLsizei capacity_;
        T *base_;
        T *current_;
    };
}