#pragma once
#include <Nova/graphics/WindowData.hpp>
#ifdef NV_WINDOWS
#include <Nova/platform/windows/GLContextData.hpp>
#else
#error "Unsupported platform!"
#endif
#include <utility>

namespace Nova
{
    class GLContext
    {
    public:
        GLContext() = default;

        GLContext(const WindowData &windowData, unsigned int versionMajor, unsigned int versionMinor);

        GLContext(const GLContext &) = delete;

        constexpr GLContext(GLContext &&other) noexcept
            : data_(std::exchange(other.data_, {})) {}

        ~GLContext() noexcept;

        void MakeCurrent();

        void LoadGL();

        double GetRefreshRate();

        GLContext &operator=(const GLContext &) = delete;

        constexpr GLContext &operator=(GLContext &&other) noexcept
        {
            data_ = std::exchange(other.data_, {});
            return *this;
        }

    private:
        GLContextData data_;
    };
}