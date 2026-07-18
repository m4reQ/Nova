#pragma once
#include <Nova/core/Input.hpp>
#include <Nova/graphics/Rect.hpp>
#include <glm/vec2.hpp>
#include <glfw/glfw3.h>
#include <span>

namespace Nova
{
	enum class EventType
	{
		WindowResize,
		WindowMove,
		WindowClose,
		WindowFocus,
		MouseMove,
		MouseButtonDown,
		MouseButtonUp,
		MouseScroll,
		KeyDown,
		KeyUp,
		FileDrop,
	};

	class Event
	{
	public:
		virtual ~Event() noexcept = default;

		virtual EventType GetEventType() const noexcept = 0;

		virtual const std::string_view GetName() const noexcept = 0;
	};

	class WindowResizeEvent final : public Event
	{
	public:
		WindowResizeEvent() = default;

		constexpr WindowResizeEvent(int width, int height) noexcept
			: width_(width), height_(height) {}

		EventType GetEventType() const noexcept override { return EventType::WindowResize; }

		const std::string_view GetName() const noexcept override { return "WindowResize"; }

		constexpr int GetWidth() const noexcept { return width_; }

		constexpr int GetHeight() const noexcept { return height_; }

		constexpr glm::ivec2 GetSize() const noexcept { return glm::ivec2{width_, height_}; }

		constexpr Rect<int> GetRect() const noexcept { return Rect{0, 0, width_, height_}; }

	private:
		int width_;
		int height_;
	};

	class WindowMoveEvent final : public Event
	{
	public:
		WindowMoveEvent() = default;

		constexpr WindowMoveEvent(int x, int y) noexcept
			: x_(x), y_(y) {}

		EventType GetEventType() const noexcept override { return EventType::WindowMove; }

		const std::string_view GetName() const noexcept override { return "WindowMove"; }

		constexpr int GetX() const noexcept { return x_; }

		constexpr int GetY() const noexcept { return y_; }

		constexpr glm::ivec2 GetPosition() const noexcept { return glm::ivec2{x_, y_}; }

	private:
		int x_;
		int y_;
	};

	class WindowCloseEvent final : public Event
	{
	public:
		WindowCloseEvent() = default;

		constexpr WindowCloseEvent(double time) noexcept
			: time_(time) {}

		EventType GetEventType() const noexcept override { return EventType::WindowClose; }

		const std::string_view GetName() const noexcept override { return "WindowClose"; }

		constexpr double GetTime() const noexcept { return time_; }

	private:
		double time_;
	};

	class WindowFocusEvent final : public Event
	{
	public:
		WindowFocusEvent() = default;

		constexpr WindowFocusEvent(bool isFocused) noexcept
			: isFocused_(isFocused) {}

		EventType GetEventType() const noexcept override { return EventType::WindowFocus; }

		const std::string_view GetName() const noexcept override { return "WindowFocus"; }

		constexpr bool IsFocused() const noexcept { return isFocused_; }

	private:
		bool isFocused_;
	};

	class MouseMoveEvent final : public Event
	{
	public:
		MouseMoveEvent() = default;

		constexpr MouseMoveEvent(double x, double y, double deltaX, double deltaY) noexcept
			: x_(x), y_(y), deltaX_(deltaX), deltaY_(deltaY) {}

		EventType GetEventType() const noexcept override { return EventType::MouseMove; }

		const std::string_view GetName() const noexcept override { return "MouseMove"; }

		constexpr double GetX() const noexcept { return x_; }

		constexpr double GetY() const noexcept { return y_; }

		constexpr glm::dvec2 GetPosition() const noexcept { return glm::dvec2{x_, y_}; }

		constexpr double GetDeltaX() const noexcept { return deltaX_; }

		constexpr double GetDeltaY() const noexcept { return deltaX_; }

		constexpr glm::dvec2 GetDelta() const noexcept { return glm::dvec2{deltaX_, deltaY_}; }

	private:
		double x_;
		double y_;
		double deltaX_;
		double deltaY_;
	};

	class MouseButtonDownEvent final : public Event
	{
	public:
		MouseButtonDownEvent() = default;

		constexpr MouseButtonDownEvent(Button button) noexcept
			: button_(button) {}

		EventType GetEventType() const noexcept override { return EventType::MouseButtonDown; }

		const std::string_view GetName() const noexcept override { return "MouseButtonDown"; }

		constexpr Button GetButton() const noexcept { return button_; }

	private:
		Button button_;
	};

	class MouseButtonUpEvent final : public Event
	{
	public:
		MouseButtonUpEvent() = default;

		constexpr MouseButtonUpEvent(Button button) noexcept
			: button_(button) {}

		EventType GetEventType() const noexcept override { return EventType::MouseButtonUp; }

		const std::string_view GetName() const noexcept override { return "MouseButtonUp"; }

		constexpr Button GetButton() const noexcept { return button_; }

	private:
		Button button_;
	};

	class MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent() = default;

		constexpr MouseScrollEvent(double v, double h) noexcept
			: v_(v), h_(h) {}

		EventType GetEventType() const noexcept override { return EventType::MouseScroll; }

		const std::string_view GetName() const noexcept override { return "MouseScroll"; }

		constexpr double GetVertical() const noexcept { return v_; }

		constexpr double GetHorizontal() const noexcept { return h_; }

	private:
		double v_;
		double h_;
	};

	class KeyDownEvent final : public Event
	{
	public:
		KeyDownEvent() = default;

		constexpr KeyDownEvent(Key key, bool isRepeated) noexcept
			: key_(key),
			  isRepeated_(isRepeated) {}

		EventType GetEventType() const noexcept override { return EventType::KeyDown; }

		const std::string_view GetName() const noexcept override { return "KeyDown"; }

		constexpr Key GetKey() const noexcept { return key_; }

		constexpr bool IsRepeated() const noexcept { return isRepeated_; }

	private:
		Key key_;
		bool isRepeated_;
	};

	class KeyUpEvent final : public Event
	{
	public:
		KeyUpEvent() = default;

		constexpr KeyUpEvent(Key key) noexcept
			: key_(key) {}

		EventType GetEventType() const noexcept override { return EventType::KeyUp; }

		const std::string_view GetName() const noexcept override { return "KeyUp"; }

		constexpr Key GetKey() const noexcept { return key_; }

	private:
		Key key_;
	};

	class FileDropEvent final : public Event
	{
	public:
		FileDropEvent() = default;

		constexpr FileDropEvent(const std::span<const char *> paths) noexcept
			: paths_(paths) {}

		constexpr FileDropEvent(int pathsCount, const char *paths[]) noexcept
			: paths_(paths, pathsCount) {}

		EventType GetEventType() const noexcept override { return EventType::FileDrop; }

		const std::string_view GetName() const noexcept override { return "FileDrop"; }

		constexpr size_t GetPathsCount() const noexcept { return paths_.size(); }

		constexpr std::span<const char *> GetPaths() const noexcept { return paths_; }

		constexpr const std::string_view GetPath(size_t index) const noexcept { return paths_[index]; }

	private:
		std::span<const char *> paths_;
	};
}