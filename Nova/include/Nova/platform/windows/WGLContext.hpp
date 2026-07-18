#pragma once
#include <Nova/utils/AutoRelease.hpp>
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

        WGLContext(HGLRC context) noexcept;

        WGLContext(WGLContext &&) noexcept = default;

        void MakeCurrent(HDC deviceContext) const;

        void LoadModern(HDC deviceContext) const;

        constexpr HGLRC Get() const noexcept { return context_.Get(); }

        constexpr HGLRC Reset() noexcept { return context_.Reset(); }

        constexpr operator HGLRC() const noexcept { return Get(); }

        WGLContext &operator=(WGLContext &&) noexcept = default;

    private:
        AutoRelease<HGLRC> context_;
    };
}