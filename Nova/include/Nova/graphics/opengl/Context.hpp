#pragma once
#include <Nova/graphics/WindowData.hpp>
#ifdef NV_WINDOWS
#include <Nova/platform/windows/GLContextData.hpp>
#else
#error "Unsupported platform!"
#endif

namespace Nova
{
    class GLContext
    {
    public:
        GLContext() = default;

        GLContext(const WindowData &windowData, unsigned int versionMajor, unsigned int versionMinor);

        GLContext(const GLContext &) = delete;

        GLContext(GLContext &&) noexcept = default;

        ~GLContext() noexcept;

        void MakeCurrent();

        void LoadGL();

        GLContext &operator=(const GLContext &) = delete;

        GLContext &operator=(GLContext &&) noexcept = default;

    private:
        GLContextData data_;
    };
}