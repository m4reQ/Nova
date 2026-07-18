#pragma once
#include <Nova/graphics/opengl/GL.hpp>
#include <glm/glm.hpp>

namespace Nova
{
    struct TextureUploadInfo
    {
        glm::ivec3 Size;
        glm::ivec3 Offset;
        size_t DataOffset;
        GLint Mipmap;
        PixelFormat PixelFormat;
        PixelType PixelType;
    };
}