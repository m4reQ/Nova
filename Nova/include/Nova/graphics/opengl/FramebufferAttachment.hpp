#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <glm/vec2.hpp>

namespace Nova
{
    class IFramebufferAttachment
    {
    public:
        virtual ~IFramebufferAttachment() noexcept = default;

        virtual GLuint GetID() const noexcept = 0;

        virtual GLsizei GetWidth() const noexcept = 0;

        virtual GLsizei GetHeight() const noexcept = 0;

        virtual InternalFormat GetFormat() const noexcept = 0;
    };
}