#pragma once
#include <glm/vec3.hpp>

struct TransformComponent
{
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::vec3 Rotation;
};