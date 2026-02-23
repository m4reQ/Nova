#pragma once
#include <Nova/input/Button.hpp>
#include <Nova/input/Modifier.hpp>
#include <Nova/input/Key.hpp>
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
		MouseButton,
		MouseScroll,
		Key,
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
			: width_(width), height_(height) { }

		EventType GetEventType() const noexcept override { return EventType::WindowResize; }
		
		const std::string_view GetName() const noexcept override { return "WindowResize"; }

		constexpr int GetWidth() const noexcept { return width_; }

		constexpr int GetHeight() const noexcept { return height_; }

		constexpr glm::ivec2 GetSize() const noexcept { return glm::ivec2{ width_, height_ }; }

		constexpr Rect<int> GetRect() const noexcept { return Rect{ 0, 0, width_, height_ }; }

	private:
		int width_;
		int height_;
	};

	class WindowMoveEvent final : public Event
	{
	public:
		WindowMoveEvent() = default;

		constexpr WindowMoveEvent(int x, int y) noexcept
			: x_(x), y_(y) { }

		EventType GetEventType() const noexcept override { return EventType::WindowMove; }

		const std::string_view GetName() const noexcept override { return "WindowMove"; }

		constexpr int GetX() const noexcept { return x_; }

		constexpr int GetY() const noexcept { return y_; }

		constexpr glm::ivec2 GetPosition() const noexcept { return glm::ivec2{ x_, y_ }; }

	private:
		int x_;
		int y_;
	};

	class WindowCloseEvent final : public Event
	{
	public:
		WindowCloseEvent() = default;

		constexpr WindowCloseEvent(double time) noexcept
			: time_(time) { }

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
			: isFocused_(isFocused) { }

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

		constexpr MouseMoveEvent(double x, double y, double lastX, double lastY) noexcept
			: x_(x), y_(y), lastX_(lastX), lastY_(lastY) { }

		EventType GetEventType() const noexcept override { return EventType::MouseMove; }

		const std::string_view GetName() const noexcept override { return "MouseMove"; }

		constexpr double GetX() const noexcept { return x_; }

		constexpr double GetY() const noexcept { return y_; }

		constexpr double GetLastX() const noexcept { return lastX_; }

		constexpr double GetLastY() const noexcept { return lastY_; }

		constexpr double GetDeltaX() const noexcept { return lastX_ - x_; }

		constexpr double GetDeltaY() const noexcept { return lastY_ - y_; }
		
		constexpr glm::dvec2 GetPosition() const noexcept { return glm::dvec2{ x_, y_ }; }

		constexpr glm::dvec2 GetLastPosition() const noexcept { return glm::dvec2{ lastX_, lastY_ }; }

		constexpr glm::dvec2 GetDelta() const noexcept { return glm::dvec2{ GetDeltaX(), GetDeltaY() }; }

	private:
		double x_;
		double y_;
		double lastX_;
		double lastY_;
	};

	class MouseButtonEvent final : public Event
	{
	public:
		MouseButtonEvent() = default;

		constexpr MouseButtonEvent(Button button, bool isPressed, Modifier modifiers) noexcept
			: button_(button), isPressed_(isPressed), modifiers_(modifiers) { }

		EventType GetEventType() const noexcept override { return EventType::MouseButton; }

		const std::string_view GetName() const noexcept override { return "MouseButton"; }

		constexpr Button GetButton() const noexcept { return button_; }
		
		constexpr bool IsPressed() const noexcept { return isPressed_; }
		
		constexpr bool IsReleased() const noexcept { return !isPressed_; }

		constexpr Modifier GetModifiers() const noexcept { return modifiers_; }

		constexpr bool IsModifierActive(Modifier mod) const noexcept { return ((int)modifiers_ & (int)mod) == (int)mod; }

	private:
		Button button_;
		Modifier modifiers_;
		bool isPressed_;
	};

	class MouseScrollEvent final : public Event
	{
	public:
		MouseScrollEvent() = default;

		constexpr MouseScrollEvent(double v, double h) noexcept
			: v_(v), h_(h) { }

		EventType GetEventType() const noexcept override { return EventType::MouseScroll; }

		const std::string_view GetName() const noexcept override { return "MouseScroll"; }

		constexpr double GetVertical() const noexcept { return v_; }

		constexpr double GetHorizontal() const noexcept { return h_; }

	private:
		double v_;
		double h_;
	};

	class KeyEvent final : public Event
	{
	public:
		KeyEvent() = default;

		constexpr KeyEvent(Key key, int action, Modifier mods) noexcept
			: key_(key), action_(action), mods_(mods) { }

		EventType GetEventType() const noexcept override { return EventType::Key; }

		const std::string_view GetName() const noexcept override { return "Key"; }

		constexpr Key GetKey() const noexcept { return key_; }
		
		constexpr bool IsPressed() const noexcept { return action_ == GLFW_PRESS; }
		
		constexpr bool IsReleased() const noexcept { return action_ == GLFW_RELEASE; }
		
		constexpr bool IsRepeated() const noexcept { return action_ == GLFW_REPEAT; }
		
		constexpr Modifier GetModifiers() const noexcept { return mods_; }
		
		constexpr bool IsModifierActive(Modifier mod) const noexcept { return ((int)mods_ & (int)mod) == (int)mod; }

	private:
		Key key_;
		Modifier mods_;
		int action_;
	};

	class FileDropEvent final : public Event
	{
	public:
		FileDropEvent() = default;

		constexpr FileDropEvent(const std::span<const char*> paths) noexcept
			: paths_(paths) { }

		constexpr FileDropEvent(int pathsCount, const char* paths[]) noexcept
			: paths_(paths, pathsCount) { }

		EventType GetEventType() const noexcept override { return EventType::FileDrop; }

		const std::string_view GetName() const noexcept override { return "FileDrop"; }

		constexpr size_t GetPathsCount() const noexcept { return paths_.size(); }

		constexpr std::span<const char*> GetPaths() const noexcept { return paths_; }

		constexpr const std::string_view GetPath(size_t index) const noexcept { return paths_[index]; }

	private:
		std::span<const char*> paths_;
	};
}