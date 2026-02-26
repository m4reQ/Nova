#pragma once
#include <glm/vec3.hpp>

struct DirectionalLightComponent
{
    glm::vec3 Color;
    float Intensity;
    glm::vec3 Direction;
};

struct PointLightComponent
{
    glm::vec3 Color;
    float Intensity;
    float Radius;
};