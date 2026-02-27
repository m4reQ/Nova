#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <utility>
#include <array>

namespace Nova
{
    constexpr size_t SyncHandlesCount = 3;
    constexpr size_t SyncTimeoutInfinite = -1;
    constexpr size_t SyncTimeoutImmediate = 0;
    constexpr size_t SyncInfiniteTimeoutThresholdNs = 1000;

    class Sync
    {
    public:
        Sync() = default;

        constexpr Sync(const Sync&) = delete;

        constexpr Sync(Sync&& other) noexcept
            : syncHandles_(std::exchange(other.syncHandles_, {0})) { }

        ~Sync() noexcept;

        void Set() noexcept;

        bool WaitClient(size_t timeoutNs = SyncTimeoutImmediate) noexcept;

        void WaitServer() noexcept;

        bool IsSignaled() const noexcept;

        constexpr GLsync Get() const noexcept { return syncHandles_[currentSyncIndex_]; }

        constexpr Sync& operator=(Sync&& other) noexcept
        {
            syncHandles_ = std::exchange(other.syncHandles_, {0});
            return *this;
        }

        constexpr operator GLsync() const noexcept { return Get(); }

    private:
        std::array<GLsync, SyncHandlesCount> syncHandles_ = {0};
        size_t currentSyncIndex_ = 0;
    };
}