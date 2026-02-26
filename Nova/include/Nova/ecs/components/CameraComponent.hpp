#pragma once
#include <glm/vec3.hpp>

enum class CameraType
{
    Perspective,
    Orthographic,
};

struct PerspectiveData
{
    float FOV;
    float AspectRatio;
    float ZNear;
    float ZFar;
};

struct OrthographicData
{
    float Left;
    float Right;
    float Bottom;
    float Top;
};

struct CameraComponent
{
    union
    {
        PerspectiveData Perspective;
        OrthographicData Ortho;
    } Data;
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    CameraType Type;

    static constexpr CameraComponent CreatePerspective(
        float fov,
        float aspectRatio,
        float zNear,
        float zFar) noexcept
    {
        CameraComponent component;
        component.Position = {0.0f, 0.0f, 0.0f};
        component.Front = {0.0f, 0.0f, -1.0f};
        component.Up = {0.0f, 1.0f, 0.0f};
        component.Type = CameraType::Perspective;
        component.Data.Perspective = PerspectiveData {
            .FOV = fov,
            .AspectRatio = aspectRatio,
            .ZNear = zNear,
            .ZFar = zFar,
        };

        return component;
    }
};