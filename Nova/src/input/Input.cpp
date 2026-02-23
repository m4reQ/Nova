#include <Nova/input/Input.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/graphics/Window.hpp>
#include <GLFW/glfw3.h>
#include <memory>

using namespace Nova;

constexpr const size_t DefaultTextBufferCapacity = 512;

static Modifier s_Modifiers;
static uint8_t s_KeyState[(size_t)Key::EnumMax];
static bool s_ButtonState[(size_t)Button::EnumMax];
static std::unique_ptr<wchar_t[]> s_TextBuffer = std::make_unique<wchar_t[]>(DefaultTextBufferCapacity);
static size_t s_TextBufferCapacity = DefaultTextBufferCapacity;
static size_t s_TextBufferLength;
static bool s_TextCaptureEnabled;
static double s_MouseX;
static double s_MouseY;
static double s_VScroll;
static double s_HScroll;

static int GetButtonState(Button button) noexcept
{
    return glfwGetMouseButton(Window::GetNativeHandle(), (int)button);
}

static int GetKeyState(Key key) noexcept
{
    return glfwGetKey(Window::GetNativeHandle(), (int)key);
}

bool Input::IsButtonDown(Button button) noexcept
{
    return GetButtonState(button) == GLFW_PRESS;
}

bool Input::IsButtonUp(Button button) noexcept
{
    return GetButtonState(button) == GLFW_RELEASE;
}

bool Input::IsKeyDown(Key key) noexcept
{
    return GetKeyState(key) != GLFW_RELEASE;
}

bool Input::IsKeyUp(Key key) noexcept
{
    return GetKeyState(key) == GLFW_RELEASE;
}

bool Input::IsKeyRepeated(Key key) noexcept
{
    return GetKeyState(key) == GLFW_REPEAT;
}

double Input::GetMouseX() noexcept
{
    double x;
    glfwGetCursorPos(Window::GetNativeHandle(), &x, nullptr);
    return x;
}

double Input::GetMouseY() noexcept
{
    double y;
    glfwGetCursorPos(Window::GetNativeHandle(), nullptr, &y);
    return y;
}

std::pair<double, double> Input::GetMousePos() noexcept
{
    double x, y;
    glfwGetCursorPos(Window::GetNativeHandle(), &x, &y);
    return { x, y };
}

bool Input::IsModifierActive(Modifier modifier) noexcept
{
    return (s_Modifiers & modifier) == modifier;
}

void Input::BeginTextInput() noexcept
{
    s_TextBufferLength = 0;
    s_TextCaptureEnabled = true;
}

const std::wstring_view Input::EndTextInput() noexcept
{
    s_TextCaptureEnabled = false;
    s_TextBuffer[s_TextBufferLength] = L'\0';
    s_TextBufferLength = 0;

    return &s_TextBuffer[0];
}

const std::wstring_view Input::GetTextInput() noexcept
{
    return &s_TextBuffer[0];
}

size_t Input::GetTextInputCapacity() noexcept
{
    return s_TextBufferCapacity;
}

void Input::SetTextInputCapacity(size_t newSize)
{
    if (newSize > s_TextBufferCapacity)
    {
        wchar_t *newBuffer = (wchar_t *)std::realloc(s_TextBuffer.get(), s_TextBufferLength);
        s_TextBuffer.release();
        s_TextBuffer.reset(newBuffer);

        s_TextBufferCapacity = newSize;
    }
}

bool Input::IsTextInputActive() noexcept
{
    return s_TextCaptureEnabled;
}

double Input::GetMouseVScroll() noexcept
{
    return s_VScroll;
}

double Input::GetMouseHScroll() noexcept
{
    return s_HScroll;
}

void Input::_UpdateKey(Key key, uint8_t state) noexcept
{
    s_KeyState[(size_t)key] = state;
}

void Input::_UpdateButton(Button button, bool isPressed) noexcept
{
    s_ButtonState[(size_t)button] = isPressed;
}

void Input::_UpdateModifiers(Modifier modifiers) noexcept
{
    s_Modifiers = modifiers;
}

void Input::_UpdateMousePos(double x, double y) noexcept
{
    s_MouseX = x;
    s_MouseY = y;
}

void Input::_AppendTextChar(wchar_t character) noexcept
{
    if (!s_TextCaptureEnabled)
        return;

    // handle backspace
    if (character == L'\b')
    {
        s_TextBufferLength--;
        return;
    }

    // skip any non-printable characters
    // TODO Ability to configure handling non-printable characters
    if (!iswprint(character))
        return;

    // reserve space for NULL-terminator
    if ((s_TextBufferLength + 1) >= s_TextBufferCapacity)
    {
        NV_LOG_WARNING("Text input buffer capacity exceeded. Stopping text input.");
        s_TextCaptureEnabled = false;
    }
    else
    {
        s_TextBuffer[s_TextBufferLength++] = character;
    }
}

void Input::_UpdateMouseScroll(double vertical, double horizontal) noexcept
{
    s_VScroll = vertical;
    s_HScroll = horizontal;
}
