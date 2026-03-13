#pragma once
#include <Nova/input/Input.hpp>
#include <Nova/graphics/Window.hpp>
#include <Nova/ecs/components/CameraComponent.hpp>
#include <Nova/ecs/components/TransformComponent.hpp>
#include <Nova/ecs/components/LightComponent.hpp>
#include <Nova/ecs/components/ScriptController.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

constexpr float CameraSpeed = 0.3f;
constexpr double MouseSensitivity = 0.001f;
constexpr float ScrollSensitivity = 1.0f;

class CameraController final : public ScriptController
{
public:
    void OnAttach(entt::registry &scene, entt::entity parentEntity) override;

    void OnEvent(const Nova::Event &event) override;

    void OnUpdate(double frametime) override;

private:
    Nova::CameraComponent *camera_;
    Nova::TransformComponent *lightTransform_;

    float cameraPitch_ = 0.0f;
    float cameraYaw_ = glm::radians(-90.0f);
    bool isMouseCaptured_ = false;

    void OnKey(const Nova::KeyEvent &event) noexcept;

    void OnMouseScroll(const Nova::MouseScrollEvent &event) noexcept;

    void OnMouseMove(const Nova::MouseMoveEvent &event) noexcept;
};