#include "Camera.hpp"

void Camera::Move(glm::vec3 delta)
{
	position_ += delta;
}

void Camera::Move(float dx, float dy, float dz)
{
	Move({ dx, dy, dz });
}

void Camera::MoveTo(glm::vec3 position)
{
	position_ = position;
}

void Camera::MoveTo(float x, float y, float z)
{
	MoveTo({ x, y, z });
}

void Camera::SetFOVDegrees(float fov)
{
	SetFOVRadians(glm::radians(fov));
}

void Camera::SetFOVRadians(float fov)
{
	fov_ = fov;
}

void Camera::SetAspectRatio(float aspectRatio)
{
	aspectRatio_ = aspectRatio;
}

void Camera::SetAspectRatio(float width, float height)
{
	aspectRatio_ = width / height;
}

void Camera::SetRotationDegrees(float yaw, float pitch)
{
	SetRotationRadians(glm::radians(yaw), glm::radians(pitch));
}

void Camera::SetRotationRadians(float yaw, float pitch)
{
	pitch = glm::clamp(pitch, -1.57f, 1.57f);
	front_ = glm::normalize(
		glm::vec3(
			glm::cos(yaw) * glm::cos(pitch),
			glm::sin(pitch),
			glm::sin(yaw) * glm::cos(pitch)));
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(fov_, aspectRatio_, 0.01f, 5.0f);
}