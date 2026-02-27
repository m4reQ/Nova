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
    void OnAttach(entt::registry& scene, entt::entity parentEntity) override
    {
        camera_ = scene.try_get<CameraComponent>(parentEntity);
        lightTransform_ = scene.try_get<TransformComponent>(parentEntity);
    }

    void OnMouseMove(const Nova::MouseMoveEvent& event) noexcept
    {
        if (!isMouseCaptured_)
            return;

        cameraYaw_ += (float)(-event.GetDeltaX() * MouseSensitivity);

        cameraPitch_ += (float)(event.GetDeltaY() * MouseSensitivity);
        cameraPitch_ = glm::clamp(cameraPitch_, -1.57f, 1.57f);

        camera_->Front = glm::normalize(
            glm::vec3(
                glm::cos(cameraYaw_) * glm::cos(cameraPitch_),
                glm::sin(cameraPitch_),
                glm::sin(cameraYaw_) * glm::cos(cameraPitch_)));
    }

    void OnMouseScroll(const Nova::MouseScrollEvent& event) noexcept
    {
        if (!isMouseCaptured_)
            return;

        camera_->Data.Perspective.FOV = glm::clamp(
            camera_->Data.Perspective.FOV - (float)glm::radians(event.GetVertical() * ScrollSensitivity),
            0.018f,
            1.57f);
    }

    void OnKey(const Nova::KeyEvent& event) noexcept
    {
        if (event.IsPressed() && event.GetKey() == Nova::Key::_1)
        {
            isMouseCaptured_ = !isMouseCaptured_;
            Nova::Window::SetCursorCapture(isMouseCaptured_);
        }
    }

    void OnEvent(const Nova::Event& event) override
    {
        switch (event.GetEventType())
        {
        case Nova::EventType::MouseMove:
            OnMouseMove((const Nova::MouseMoveEvent&)event);
            break;
        case Nova::EventType::MouseScroll:
            OnMouseScroll((const Nova::MouseScrollEvent&)event);
            break;
        case Nova::EventType::Key:
            OnKey((const Nova::KeyEvent&)event);
            break;
        }
    }

    void OnUpdate(double frametime) override
    {
        if (!isMouseCaptured_)
            return;

        auto delta = glm::zero<glm::vec3>();

        if (Nova::Input::IsKeyDown(Nova::Key::W))
            delta += camera_->Front;
        if (Nova::Input::IsKeyDown(Nova::Key::S))
            delta -= camera_->Front;
        if (Nova::Input::IsKeyDown(Nova::Key::D))
            delta += glm::normalize(
                glm::cross(
                    camera_->Front,
                    camera_->Up));
        if (Nova::Input::IsKeyDown(Nova::Key::A))
            delta -= glm::normalize(
                glm::cross(
                    camera_->Front,
                    camera_->Up));
        if (Nova::Input::IsKeyDown(Nova::Key::Space))
            delta += camera_->Up;
        if (Nova::Input::IsKeyDown(Nova::Key::LeftShift))
            delta -= camera_->Up;

        camera_->Position += delta * CameraSpeed * (float)frametime;
        lightTransform_->Position = camera_->Position;
    }

private:
    CameraComponent* camera_;
    TransformComponent* lightTransform_;

    float cameraPitch_ = 0.0f;
    float cameraYaw_ = glm::radians(-90.0f);
    bool isMouseCaptured_ = false;
};