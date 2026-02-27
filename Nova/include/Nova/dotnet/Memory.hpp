#pragma once
#include <Nova/core/Build.hpp>
#include <Windows.h>
#include <objbase.h>
#include <utility>
#include <memory>
#include <span>

namespace Nova
{
    template <typename T = void>
    class LocalMemory
    {
    public:
        LocalMemory() = default;

        LocalMemory(size_t size, uint32_t flags)
            : data_(LocalAlloc(flags, size)) { }

        LocalMemory(const LocalMemory& other) noexcept
        {
            data_ = LocalAlloc(other.GetFlags(), other.GetSize());
            std::memcpy(data_, other.data_, other.GetSize());
        }

        constexpr LocalMemory(LocalMemory&& other) noexcept
            : data_(std::exchange(other.data_, nullptr)) { }

        constexpr LocalMemory(HLOCAL data) noexcept
            : data_(data) { }
        
        ~LocalMemory() noexcept
        {
            LocalFree(data_);
        }

        constexpr LocalMemory& operator=(const LocalMemory& other)
        {
            data_ = LocalAlloc(other.GetFlags(), other.GetSize());
            std::memcpy(data_, other.data_, other.GetSize());

            return *this;
        }

        constexpr LocalMemory& operator=(LocalMemory&& other) noexcept
        {
            data_ = std::exchange(other.data_, nullptr);
            
            return *this;
        }

        size_t GetSize() const noexcept { return IsValid() ? LocalSize(data_) : 0; }

        uint32_t GetFlags() const noexcept { return LocalFlags(data_); }

        bool IsValid() const noexcept { return GetFlags() != LMEM_INVALID_HANDLE; }

        constexpr bool IsNull() const noexcept { return data_ == nullptr; }

        constexpr T* Get() noexcept { return static_cast<T*>(data_); }

        constexpr const T* Get() const noexcept { return static_cast<T*>(data_); }

        constexpr T** GetRef() noexcept { return reinterpret_cast<T**>(&data_); }

        constexpr const T** GetRef() const noexcept { return (const T**)&data_; }

        constexpr std::span<T> GetView() noexcept
        {
            // TODO Check if data is aligned for T
            // TODO Should also check if size can contain at least n full objects
            return std::span<T>(Get(), GetSize() / sizeof(T));
        }

        constexpr std::span<const T> GetView() const noexcept
        {
            // TODO Check if data is aligned for T
            // TODO Should also check if size can contain at least n full objects
            return std::span<const T>(Get(), GetSize() / sizeof(T));
        }

    private:
        HLOCAL data_ = nullptr;
    };

    template <typename T = void>
    class TaskMemory
    {
    public:
        TaskMemory() noexcept = default;
        
        constexpr TaskMemory(T* data) noexcept
            : data_(data) { }
        
        constexpr TaskMemory(TaskMemory<T>&& other) noexcept
            : data_(std::exchange(other.data_, nullptr)) { }

        constexpr TaskMemory& operator=(TaskMemory<T>&& other) noexcept
        {
            data_ = std::exchange(other.data_, nullptr);
            return *this;
        }

        TaskMemory(size_t count) noexcept
        {
            data_ = CoTaskMemAlloc(count * sizeof(T));
            NV_CHECK(data_ != nullptr, "CoTaskMemAlloc failed.");
        }

        ~TaskMemory() noexcept
        {
            CoTaskMemFree(data_);
        }

        constexpr T* Get() noexcept { return data_; }
        
        constexpr const T* Get() const noexcept { return data_; }

        constexpr T** GetRef() noexcept { return &data_; }

        constexpr const T** GetRef() const noexcept { return &data_; }

    private:
        T* data_ = nullptr;
    };
}