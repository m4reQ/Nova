#pragma once
#include <Nova/graphics/opengl/Texture.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace Nova
{
    struct Material
    {
        glm::vec4 Color;
        float SpecularIntensity;
        float Shininess;
        std::shared_ptr<Texture> AlbedoTexture;
    };
}