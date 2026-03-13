#pragma once
#include <Nova/core/Layer.hpp>
#include <Nova/assets/Model.hpp>
#include <Nova/assets/Image.hpp>
#include <entt/entt.hpp>
#include "Camera.hpp"

class MainLayer final : public Nova::Layer
{
public:
	MainLayer();

	void OnUpdate(double frametime) override;
	void OnRender() override;
	bool OnEvent(const Nova::Event &event) override;

private:
	void OnResizeEvent(const Nova::WindowResizeEvent &event) noexcept;
	void OnKeyEvent(const Nova::KeyEvent &event) noexcept;
	void OnMouseMoveEvent(const Nova::MouseMoveEvent &event) noexcept;
	void OnMouseScrollEvent(const Nova::MouseScrollEvent &event) noexcept;

	entt::registry entities_;
	std::shared_ptr<Nova::Model> model_;
	std::shared_ptr<Nova::Image> image_;
	std::shared_ptr<Nova::Image> modelIconImage_;
	bool cursorCaptured_ = false;
	entt::entity mainCameraEntity_ = (entt::entity)-1;

	glm::vec4 fogColor_ = {1.0f, 1.0f, 1.0f, 1.0f};
};