#pragma once
#include <glm/glm.hpp>

namespace Nova
{
    struct ModelVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TextureCoords;
    };
}