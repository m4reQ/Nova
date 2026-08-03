#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Nova
{
    struct TransformComponent
    {
        glm::vec3 Position = glm::zero<glm::vec3>();
        glm::vec3 Scale = glm::one<glm::vec3>();
        glm::quat Rotation;
    };
}
