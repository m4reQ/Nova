#pragma once
#include <Nova/graphics/opengl/TextureUploadInfo.hpp>
#include <vector>

namespace Nova
{
    struct ImageLoadingData
    {
        std::vector<std::pair<TextureUploadInfo, size_t>> UploadInfos;
        std::vector<void *> Data;
        GLsizei Mipmaps;
        InternalFormat TextureFormat;
        TextureTarget TextureTarget;
        PixelFormat TexturePixelFormat;
        bool GenerateMipmap;
    };

    struct DDSImageLoadingData
    {
        std::vector<std::byte> Data;
        std::vector<TextureUploadInfo> UploadInfos;
        GLsizei Mipmaps;
        InternalFormat TextureFormat;
        TextureTarget TextureTarget;
        PixelFormat TexturePixelFormat;
        bool GenerateMipmap;
    };
}