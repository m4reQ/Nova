#include <Nova/core/Input.hpp>
#include <Nova/core/Flag.hpp>
#include <vector>

using namespace Nova;

static uint8_t sKeyState[(size_t)Key::EnumMax_];

static double sMouseX;
static double sMouseY;
static double sVScroll;
static double sHScroll;

static Modifier sModifierMask = Modifier::None;
static Button sButtonMask = Button::None;

// text input
static std::vector<wchar_t> sTextBuffer;
static bool sIsTextInputEnabled;

bool Input::IsButtonDown(Button button) noexcept
{
    return Flag::IsSet(sButtonMask, button);
}

bool Input::IsButtonUp(Button button) noexcept
{
    return !Flag::IsSet(sButtonMask, button);
}

bool Input::IsKeyDown(Key key) noexcept
{
    return sKeyState[(size_t)key];
}

bool Input::IsKeyUp(Key key) noexcept
{
    return !IsKeyDown(key);
}

double Input::GetMouseX() noexcept
{
    return sMouseX;
}

double Input::GetMouseY() noexcept
{
    return sMouseY;
}

std::pair<double, double> Input::GetMousePos() noexcept
{
    return std::make_pair(sMouseX, sMouseY);
}

bool Input::IsModifierActive(Modifier modifier) noexcept
{
    return Flag::IsSet(sModifierMask, modifier);
}

void Input::BeginTextInput() noexcept
{
    sIsTextInputEnabled = true;
    sTextBuffer.clear();
}

std::wstring Input::EndTextInput() noexcept
{
    sIsTextInputEnabled = false;
    const auto result = std::wstring(sTextBuffer.begin(), sTextBuffer.end());
    sTextBuffer.clear();

    return result;
}

bool Input::IsTextInputEnabled() noexcept
{
    return sIsTextInputEnabled;
}

double Input::GetMouseVScroll() noexcept
{
    return sVScroll;
}

double Input::GetMouseHScroll() noexcept
{
    return sHScroll;
}

void Input::UpdateKey_(Key key, bool isDown) noexcept
{
    sKeyState[(size_t)key] = isDown;
}

void Input::UpdateButton_(Button button, bool isPressed) noexcept
{
    if (isPressed)
        Flag::Set(sButtonMask, button);
    else
        Flag::Clear(sButtonMask, button);
}

void Input::UpdateButtons_(Button buttons) noexcept
{
    sButtonMask = buttons;
}

void Input::UpdateModifiers_(Modifier modifiers) noexcept
{
    sModifierMask = modifiers;
}

void Input::UpdateMousePos_(double x, double y) noexcept
{
    sMouseX = x;
    sMouseY = y;
}

void Input::AppendTextChar_(wchar_t character) noexcept
{
    if (!sIsTextInputEnabled)
        return;

    // handle backspace
    if (character == L'\b')
    {
        sTextBuffer.pop_back();
        return;
    }

    // skip any non-printable characters
    // TODO Ability to configure handling non-printable characters
    if (!iswprint(character))
        return;

    sTextBuffer.push_back(character);
}

void Input::UpdateMouseScroll_(double vertical, double horizontal) noexcept
{
    sVScroll = vertical;
    sHScroll = horizontal;
}
