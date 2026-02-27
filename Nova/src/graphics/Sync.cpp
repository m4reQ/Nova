#include <Nova/graphics/opengl/Sync.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

static constexpr bool CheckSyncWaitResult(GLenum result) noexcept
{
    return result == GL_ALREADY_SIGNALED
        || result == GL_CONDITION_SATISFIED;
}

static constexpr bool SyncWaitFailed(GLenum result) noexcept
{
    return result == GL_WAIT_FAILED;
}

static GLenum WaitClientSyncInfinite(GLsync sync) noexcept
{
    GLenum result = GL_UNSIGNALED;

    do
    {
        result = glClientWaitSync(
            sync,
            GL_SYNC_FLUSH_COMMANDS_BIT,
            SyncInfiniteTimeoutThresholdNs);
    } while (result != GL_ALREADY_SIGNALED && result != GL_CONDITION_SATISFIED && result != GL_WAIT_FAILED);

    return result;
}

Sync::~Sync() noexcept
{
    for (const auto sync : syncHandles_)
        glDeleteSync(sync);
}

void Sync::Set() noexcept
{
    NV_PROFILE_FUNC;

    const auto deleteIndex = (currentSyncIndex_ + 1) % SyncHandlesCount;
    if (syncHandles_[deleteIndex]) {
        NV_PROFILE_SCOPE("::DeleteSyncHandle");

        glDeleteSync(syncHandles_[deleteIndex]);
        syncHandles_[deleteIndex] = nullptr;
    }

    currentSyncIndex_ = deleteIndex;

    {
        NV_PROFILE_SCOPE("::FenceSync");
        syncHandles_[currentSyncIndex_] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
}

bool Sync::WaitClient(size_t timeoutNs) noexcept
{
    NV_PROFILE_FUNC;

    const auto sync = Get();

    if (!sync)
        return true;

    const auto result = timeoutNs == SyncTimeoutInfinite
        ? WaitClientSyncInfinite(sync)
        : glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeoutNs);
    return CheckSyncWaitResult(result);
}

void Sync::WaitServer() noexcept
{
    NV_PROFILE_FUNC;

    const auto sync = Get();

    if (sync)
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
}

bool Sync::IsSignaled() const noexcept
{
    NV_PROFILE_FUNC;

    const auto sync = Get();
    
    if (!sync)
        return true;

    GLint status = GL_UNSIGNALED;
    glGetSynciv(sync, GL_SYNC_STATUS, 1, nullptr, &status);

    return status == GL_SIGNALED;
}