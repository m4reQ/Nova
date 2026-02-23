#include <Nova/graphics/Window.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/input/Input.hpp>
#include <Nova/core/Application.hpp>
#include <Nova/core/Event.hpp>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <stdexcept>
#include <format>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>

using namespace Nova;

class GLFWImage
{
public:
	GLFWImage(const std::filesystem::path& filepath)
		: shouldFree_(true)
	{
		image_.pixels = stbi_load(filepath.string().c_str(), &image_.width, &image_.height, nullptr, 3);
		if (image_.pixels == nullptr)
			throw std::runtime_error("Failed to load GLFW image from file.");
	}

	constexpr GLFWImage(std::int32_t width, std::int32_t height, std::uint8_t* pixels)
		: image_{ width, height, pixels }, shouldFree_(false) { }

	~GLFWImage() noexcept
	{
		if (shouldFree_)
			stbi_image_free(image_.pixels);
	}

	constexpr const GLFWimage& Get() const noexcept
	{
		return image_;
	}

private:
	GLFWimage image_{ 0 };
	bool shouldFree_{ false };
};

static GLFWwindow *s_WindowHandle;
static GLFWcursor *s_CursorHandle;
static int s_LastWindowPosX;
static int s_LastWindowPosY;
static int s_LastWindowWidth;
static int s_LastWindowHeight;
static FullscreenMode s_CurrentFullscreenMode;
static bool s_IsWindowDefaultDecorated;
static bool s_FirstMouseEnter = true;
static double s_LastMouseX;
static double s_LastMouseY;

static GLFWwindow *CreateWindowFullscreen(const WindowSettings &settings) noexcept
{
	NV_PROFILE_FUNC;

	auto monitor = glfwGetPrimaryMonitor();
	const auto videoMode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

	return glfwCreateWindow(
		videoMode->width,
		videoMode->height,
		settings.Title.data(),
		monitor,
		nullptr);
}

static GLFWwindow* CreateWindowFullscreenBorderless(const WindowSettings& settings)
{
	NV_PROFILE_FUNC;

	auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	return glfwCreateWindow(
		videoMode->width,
		videoMode->height,
		settings.Title.data(),
		nullptr,
		nullptr);
}

static GLFWwindow *CreateWindowWindowed(const WindowSettings &settings) noexcept
{
	NV_PROFILE_FUNC;

	return glfwCreateWindow(settings.Width, settings.Height, settings.Title.data(), nullptr, nullptr);
}

static GLFWcursor *LoadGLFWCursorFromFile(const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	const GLFWImage image(filepath);
	const auto cursor = glfwCreateCursor(&image.Get(), 0, 0);

	if (cursor == nullptr)
		throw std::runtime_error("Failed to create GLFW cursor from image file.");
	
	return cursor;
}

static void InitializeGLFW()
{
	NV_PROFILE_FUNC;

	glfwSetErrorCallback(
		[](int code, const char *msg)
		{
			NV_LOG_ERROR("GLFW Error ({}): {}.", code, msg);
		});

	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW.");
}

bool Window::ShouldClose() noexcept
{
	return glfwWindowShouldClose(s_WindowHandle);
}

void Window::Move(std::int32_t x, std::int32_t y) noexcept
{
	NV_PROFILE_FUNC;
	glfwSetWindowPos(s_WindowHandle, x, y);
}

void Nova::Window::Resize(std::int32_t width, std::int32_t height) noexcept
{
	NV_PROFILE_FUNC;
	glfwSetWindowSize(s_WindowHandle, width, height);
}

void Window::SetTitle(const std::string_view title) noexcept
{
	NV_PROFILE_FUNC;
	glfwSetWindowTitle(s_WindowHandle, title.data());
}

void Window::SetTitle(const std::wstring_view title) noexcept
{
	NV_PROFILE_FUNC;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	const auto titleConv = converter.to_bytes(title.data());
	
	SetTitle(titleConv);
}

void Window::Close() noexcept
{
	glfwSetWindowShouldClose(s_WindowHandle, true);
}

void Nova::Window::SetIcon(std::int32_t width, std::int32_t height, std::uint8_t* data)
{
	NV_PROFILE_FUNC;
	
	const GLFWImage image(width, height, data);
	glfwSetWindowIcon(s_WindowHandle, 1, &image.Get());
}

void Window::SetIcon(const std::filesystem::path &filepath)
{
	NV_PROFILE_FUNC;

	const GLFWImage image(filepath);
	glfwSetWindowIcon(s_WindowHandle, 1, &image.Get());
}

void Nova::Window::SetCursor(std::int32_t width, std::int32_t height, std::uint8_t* data, std::int32_t xHot, std::int32_t yHot)
{
	NV_PROFILE_FUNC;

	const GLFWImage image(width, height, data);
	s_CursorHandle = glfwCreateCursor(&image.Get(), xHot, yHot);
	if (!s_CursorHandle)
		throw std::runtime_error("Failed to create window cursor from image.");

	glfwSetCursor(s_WindowHandle, s_CursorHandle);
}

void Nova::Window::SetCursor(const std::filesystem::path& filepath, std::int32_t xHot, std::int32_t yHot)
{
	NV_PROFILE_FUNC;

	const GLFWImage image(filepath);
	s_CursorHandle = glfwCreateCursor(&image.Get(), xHot, yHot);
	if (!s_CursorHandle)
		throw std::runtime_error("Failed to create window cursor from image.");

	glfwSetCursor(s_WindowHandle, s_CursorHandle);
}

void Window::SetFullscreen(FullscreenMode mode) noexcept
{
	NV_PROFILE_FUNC;

	if (mode == s_CurrentFullscreenMode)
		return;

	switch (mode)
	{
	case FullscreenMode::Windowed:
	{
		glfwSetWindowMonitor(
			s_WindowHandle,
			nullptr,
			s_LastWindowPosX,
			s_LastWindowPosY,
			s_LastWindowWidth,
			s_LastWindowHeight,
			0);
		glfwSetWindowAttrib(s_WindowHandle, GLFW_DECORATED, s_IsWindowDefaultDecorated);

		break;
	}
	case FullscreenMode::Fullscreen:
	{
		const auto monitor = glfwGetPrimaryMonitor();
		const auto videoMode = glfwGetVideoMode(monitor);

		glfwGetWindowPos(s_WindowHandle, &s_LastWindowPosX, &s_LastWindowPosY);
		glfwGetWindowSize(s_WindowHandle, &s_LastWindowWidth, &s_LastWindowHeight);

		glfwSetWindowMonitor(
			s_WindowHandle,
			monitor,
			0,
			0,
			videoMode->width,
			videoMode->height,
			videoMode->refreshRate);

		break;
	}
	case FullscreenMode::FullscreenBorderless:
	{
		const auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		glfwGetWindowPos(s_WindowHandle, &s_LastWindowPosX, &s_LastWindowPosY);
		glfwGetWindowSize(s_WindowHandle, &s_LastWindowWidth, &s_LastWindowHeight);

		glfwSetWindowMonitor(
			s_WindowHandle,
			nullptr,
			0,
			0,
			videoMode->width,
			videoMode->height,
			0);
		glfwSetWindowAttrib(s_WindowHandle, GLFW_DECORATED, GLFW_FALSE);

		break;
	}
	}

	s_CurrentFullscreenMode = mode;
}

void Window::Maximize()
{
	NV_PROFILE_FUNC;

	glfwMaximizeWindow(s_WindowHandle);
}

void Window::Iconfiy()
{
	NV_PROFILE_FUNC;

	glfwIconifyWindow(s_WindowHandle);
}

std::pair<int, int> Window::GetSize() noexcept
{
	int width, height;
	Window::GetSize(width, height);

	return {width, height};
}

void Window::GetSize(int &width, int &height) noexcept
{
	glfwGetWindowSize(s_WindowHandle, &width, &height);
}

float Window::GetAspectRatio() noexcept
{
	const auto [width, height] = GetSize();
	return (float)width / (float)height;
}

std::pair<int, int> Window::GetFramebufferSize() noexcept
{
	int width, height;
	Window::GetFramebufferSize(width, height);

	return {width, height};
}

void Window::GetFramebufferSize(int &width, int &height) noexcept
{
	glfwGetFramebufferSize(s_WindowHandle, &width, &height);
}

int Nova::Window::GetFramebufferWidth() noexcept
{
	int width;
	glfwGetFramebufferSize(s_WindowHandle, &width, nullptr);

	return width;
}

int Nova::Window::GetFramebufferHeight() noexcept
{
	int height;
	glfwGetFramebufferSize(s_WindowHandle, nullptr, &height);

	return height;
}

float Window::GetFramebufferAspectRatio() noexcept
{
	const auto [width, height] = GetFramebufferSize();
	return (float)width / (float)height;
}

std::pair<int, int> Window::GetPosition() noexcept
{
	int x, y;
	Window::GetPosition(x, y);

	return {x, y};
}

void Window::GetPosition(int &x, int &y) noexcept
{
	glfwGetWindowPos(s_WindowHandle, &x, &y);
}

bool Window::IsVisible() noexcept
{
	const auto [width, height] = Window::GetSize();
	return width != 0 && height != 0;
}

GLFWwindow *Window::GetNativeHandle() noexcept
{
	return s_WindowHandle;
}

int Window::GetWidth() noexcept
{
	int width;
	glfwGetWindowSize(s_WindowHandle, &width, nullptr);

	return width;
}

int Window::GetHeight() noexcept
{
	int height;
	glfwGetWindowSize(s_WindowHandle, nullptr, &height);

	return height;
}

bool Window::IsFullscreen() noexcept
{
	return s_CurrentFullscreenMode != FullscreenMode::Windowed;
}

void Window::Initialize_(const WindowSettings &settings)
{
	NV_PROFILE_FUNC;

	NV_LOG_TRACE("Initializing window module...");

	s_IsWindowDefaultDecorated = !IS_FLAG_SET(settings.Flags, WindowFlags::Borderless);

	InitializeGLFW();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifdef NV_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#endif
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_RESIZABLE, IS_FLAG_SET(settings.Flags, WindowFlags::Resizable));
	glfwWindowHint(GLFW_DECORATED, s_IsWindowDefaultDecorated);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, IS_FLAG_SET(settings.Flags, WindowFlags::Transparent));
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);

	switch (settings.FullscreenMode)
	{
	case FullscreenMode::Windowed:
		s_WindowHandle = CreateWindowWindowed(settings);
		break;
	case FullscreenMode::Fullscreen:
		s_WindowHandle = CreateWindowFullscreen(settings);
		break;
	case FullscreenMode::FullscreenBorderless:
		s_WindowHandle = CreateWindowFullscreenBorderless(settings);
		break;
	}

	if (s_WindowHandle == nullptr)
		throw std::runtime_error("Failed to create GLFW window.");

	glfwGetWindowPos(s_WindowHandle, &s_LastWindowPosX, &s_LastWindowPosY);
	glfwGetWindowSize(s_WindowHandle, &s_LastWindowWidth, &s_LastWindowHeight);

	glfwMakeContextCurrent(s_WindowHandle);
	glfwSwapInterval(
		IS_FLAG_SET(settings.Flags, WindowFlags::Vsync)
			? 1
			: 0);

	if (IS_FLAG_SET(settings.Flags, WindowFlags::CaptureCursor))
		glfwSetInputMode(s_WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (settings.IconFilepath.has_value())
		SetIcon(settings.IconFilepath.value());

	if (settings.CursorFilepath.has_value())
		SetCursor(settings.CursorFilepath.value());
	
	glfwSetWindowSizeCallback(
		s_WindowHandle,
		[](GLFWwindow*, int width, int height)
		{
			Application::InvokeEvent_<WindowResizeEvent>(width, height);
		});
	glfwSetWindowPosCallback(
		s_WindowHandle,
		[](GLFWwindow*, int x, int y)
		{
			Application::InvokeEvent_<WindowMoveEvent>(x, y);
		});
	glfwSetWindowCloseCallback(
		s_WindowHandle,
		[](GLFWwindow*)
		{
			Application::InvokeEvent_<WindowCloseEvent>(glfwGetTime());
		});
	glfwSetCursorPosCallback(
		s_WindowHandle,
		[](GLFWwindow*, double x, double y)
		{
			if (s_FirstMouseEnter)
			{
				s_LastMouseX = x;
				s_LastMouseY = y;

				s_FirstMouseEnter = false;
			}

			Application::InvokeEvent_<MouseMoveEvent>(x, y, s_LastMouseX, s_LastMouseY);
			Input::_UpdateMousePos(x, y);

			s_LastMouseX = x;
			s_LastMouseY = y;
		});
	glfwSetKeyCallback(
		s_WindowHandle,
		[](GLFWwindow*, int key, int scancode, int action, int mods)
		{
			Application::InvokeEvent_<KeyEvent>((Key)key, action, (Modifier)mods);
			Input::_UpdateKey((Key)key, action);
			Input::_UpdateModifiers((Modifier)mods);
		});
	glfwSetMouseButtonCallback(
		s_WindowHandle,
		[](GLFWwindow* window, int button, int action, int mods)
		{
			const auto isPressed = action != GLFW_RELEASE;
			Application::InvokeEvent_<MouseButtonEvent>((Button)button, isPressed, (Modifier)mods);
			Input::_UpdateButton((Button)button, !isPressed); // TODO Input::_UpdateButton should accept isPressed rather than isReleased
			Input::_UpdateModifiers((Modifier)mods);
		});
	glfwSetScrollCallback(
		s_WindowHandle,
		[](GLFWwindow*, double horizontal, double vertical)
		{
			Application::InvokeEvent_<MouseScrollEvent>(vertical, horizontal);
			Input::_UpdateMouseScroll(vertical, horizontal);
		});
	glfwSetCharCallback(
		s_WindowHandle,
		[](GLFWwindow*, unsigned int codepoint)
		{
			// TODO Char event
			Input::_AppendTextChar((wchar_t)codepoint);
		});
	glfwSetDropCallback(
		s_WindowHandle,
		[](GLFWwindow*, int count, const char* paths[])
		{
			Application::InvokeEvent_<FileDropEvent>(count, paths);
		});
	glfwSetWindowFocusCallback(
		s_WindowHandle,
		[](GLFWwindow*, int isFocused)
		{
			Application::InvokeEvent_<WindowFocusEvent>(isFocused);
		});
}

void Window::Shutdown_() noexcept
{
	NV_PROFILE_FUNC;

	glfwDestroyCursor(s_CursorHandle);
	glfwDestroyWindow(s_WindowHandle);
	glfwTerminate();
}

void Window::Update_() noexcept
{
	NV_PROFILE_FUNC;

	glfwPollEvents();
}

void Window::SwapBuffers_() noexcept
{
	NV_PROFILE_FUNC;

	glfwSwapBuffers(s_WindowHandle);
}

GLADloadfunc Window::GetLoaderFunc_() noexcept
{
	return glfwGetProcAddress;
}