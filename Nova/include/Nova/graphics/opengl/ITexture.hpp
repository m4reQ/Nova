#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <glm/glm.hpp>

namespace Nova
{
    class ITexture
    {
    public:
        virtual ~ITexture() noexcept = default;

        virtual GLuint GetID() const noexcept = 0;

        virtual GLsizei GetWidth() const noexcept = 0;

        virtual GLsizei GetHeight() const noexcept = 0;

        virtual GLsizei GetDepth() const noexcept = 0;

        virtual glm::ivec3 GetSize() const noexcept = 0;

        virtual GLsizei GetMipmaps() const noexcept = 0;

        virtual InternalFormat GetFormat() const noexcept = 0;

        virtual TextureTarget GetTarget() const noexcept = 0;
    };
}