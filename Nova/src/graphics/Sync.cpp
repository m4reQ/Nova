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
    glDeleteSync(sync_);
}

void Sync::Set() noexcept
{
    NV_PROFILE_FUNC;

    glDeleteSync(sync_);
    sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool Sync::WaitClient(size_t timeoutNs) noexcept
{
    NV_PROFILE_FUNC;

    if (!sync_)
        return true;

    const auto result = timeoutNs == SyncTimeoutInfinite
        ? WaitClientSyncInfinite(sync_)
        : glClientWaitSync(sync_, GL_SYNC_FLUSH_COMMANDS_BIT, timeoutNs);
    return CheckSyncWaitResult(result);
}

void Sync::WaitServer() noexcept
{
    NV_PROFILE_FUNC;

    if (sync_)
        glWaitSync(sync_, 0, GL_TIMEOUT_IGNORED);
}

bool Sync::IsSignaled() const noexcept
{
    NV_PROFILE_FUNC;
    
    if (!sync_)
        return true;

    GLint status = GL_UNSIGNALED;
    glGetSynciv(sync_, GL_SYNC_STATUS, 1, nullptr, &status);

    return status == GL_SIGNALED;
}