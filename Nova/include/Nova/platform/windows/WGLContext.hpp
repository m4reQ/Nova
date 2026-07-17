#pragma once
#include <utility>
#include <span>
#include <Windows.h>

namespace Nova
{
    class WGLContext
    {
    public:
        WGLContext() = default;

        WGLContext(HDC deviceContext);

        WGLContext(HDC deviceContext, std::span<const int> attribs);

        constexpr WGLContext(HGLRC context)
            : context_(context) {}

        WGLContext(const WGLContext &) = delete;

        WGLContext(WGLContext &&) noexcept = default;

        ~WGLContext() noexcept;

        void MakeCurrent(HDC deviceContext) const;

        void LoadModern(HDC deviceContext) const;

        constexpr HGLRC Get() const noexcept { return context_; }

        constexpr HGLRC Reset() noexcept { return std::exchange(context_, nullptr); }

        WGLContext &operator=(const WGLContext &) = delete;

        WGLContext &operator=(WGLContext &&) noexcept = default;

        constexpr operator HGLRC() const noexcept { return Get(); }

    private:
        HGLRC context_ = nullptr;
    };
}