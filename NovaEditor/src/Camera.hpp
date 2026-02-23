#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/euler_angles.hpp>

class Camera
{
public:
	Camera() = default;

	void Move(glm::vec3 delta);
	void Move(float dx, float dy, float dz);
	void MoveTo(glm::vec3 position);
	void MoveTo(float x, float y, float z);
	void SetFOVDegrees(float fov);
	void SetFOVRadians(float fov);
	void SetAspectRatio(float aspectRatio);
	void SetAspectRatio(float width, float height);
	void SetRotationDegrees(float yaw, float pitch);
	void SetRotationRadians(float yaw, float pitch);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	constexpr glm::vec3 GetPosition() const noexcept { return position_; }
	constexpr glm::vec3 GetFront() const noexcept { return front_; }
	constexpr glm::vec3 GetUp() const noexcept { return up_; }
	constexpr float GetFOVDegrees() const noexcept { return glm::degrees(fov_); }
	constexpr float GetFOVRadians() const noexcept { return fov_; }

private:
	glm::vec3 position_ = { 0.0f, 0.0f, 0.0f };
	glm::vec3 front_ = { 0.0f, 0.0f, -1.0f };
	glm::vec3 up_ = { 0.0f, 1.0f, 0.0f };
	float fov_ = glm::radians(45.0f);
	float aspectRatio_ = 1.0f;
};