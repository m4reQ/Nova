#pragma once
#include <glad/gl.h>
#include <filesystem>
#include <optional>

namespace Nova
{
    struct RendererSettings
    {
        std::optional<std::filesystem::path> ShaderCacheDirectory = std::nullopt;
        GLuint MaxPointLights = 32;
        GLuint MaxDirectionalLights = 2;
        GLuint MaxMaterials = 64;
    };
}