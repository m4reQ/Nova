#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <utility>

namespace Nova
{
    constexpr size_t SyncTimeoutInfinite = -1;
    constexpr size_t SyncTimeoutImmediate = 0;
    constexpr size_t SyncInfiniteTimeoutThresholdNs = 100;

    class Sync
    {
    public:
        Sync() = default;

        constexpr Sync(const Sync&) = delete;

        constexpr Sync(Sync&& other) noexcept
            : sync_(std::exchange(other.sync_, nullptr)) { }

        ~Sync() noexcept;

        void Set() noexcept;

        bool WaitClient(size_t timeoutNs = SyncTimeoutImmediate) noexcept;

        void WaitServer() noexcept;

        bool IsSignaled() const noexcept;

        constexpr GLsync Get() const noexcept { return sync_; }

        constexpr Sync& operator=(Sync&& other) noexcept
        {
            sync_ = std::exchange(other.sync_, nullptr);
            return *this;
        }

        constexpr operator GLsync() const noexcept { return Get(); }

    private:
        GLsync sync_ = nullptr;
    };
}