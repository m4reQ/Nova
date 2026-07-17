#pragma once
#include <Windows.h>

namespace Nova
{
    class WGLContext
    {
    public:
        WGLContext() = default;

        WGLContext(HDC deviceContext);

        constexpr WGLContext(HGLRC context)
            : context_(context) {}

        WGLContext(const WGLContext &) = delete;

        ~WGLContext() noexcept;

        void MakeCurrent(HDC deviceContext) const;

        constexpr HGLRC Get() const noexcept { return context_; }

        constexpr HGLRC Reset() noexcept
        {
            auto old = context_;
            context_ = nullptr;
            return old;
        }

        WGLContext &operator=(const WGLContext &) = delete;

        constexpr operator HGLRC() const noexcept { return Get(); }

    private:
        HGLRC context_ = nullptr;
    };
}