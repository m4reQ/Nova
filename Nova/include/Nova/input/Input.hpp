#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/input/Button.hpp>
#include <Nova/input/Modifier.hpp>
#include <Nova/input/Key.hpp>

namespace Nova::Input
{
    // Checks if a specific key is currently pressed down.
    // @param key The keycode of the key to check.
    // @return `true` if the key is down, `false` otherwise.
    NV_API bool IsKeyDown(Key key) noexcept;

    // Checks if a specific key is currently released.
    // @param key The keycode of the key to check.
    // @return `true` if the key is up, `false` otherwise.
    NV_API bool IsKeyUp(Key key) noexcept;

    NV_API bool IsKeyRepeated(Key key) noexcept;

    // Checks if a specific mouse button is currently pressed down.
    // @param button The button code of the mouse button to check.
    // @return `true` if the button is down, `false` otherwise.
    NV_API bool IsButtonDown(Button button) noexcept;

    // Checks if a specific mouse button is currently released.
    // @param button The button code of the mouse button to check.
    // @return `true` if the button is up, `false` otherwise.
    NV_API bool IsButtonUp(Button button) noexcept;

    // Retrieves the current X-coordinate of the mouse cursor.
    // @return The X-coordinate of the mouse cursor as a double.
    NV_API double GetMouseX() noexcept;

    // Retrieves the current Y-coordinate of the mouse cursor.
    // @return The Y-coordinate of the mouse cursor as a double.
    NV_API double GetMouseY() noexcept;

    NV_API double GetMouseVScroll() noexcept;

    NV_API double GetMouseHScroll() noexcept;

    // Retrieves the current position of the mouse cursor.
    NV_API std::pair<double, double> GetMousePos() noexcept;

    // Checks if a specific modifier key (e.g., Shift, Ctrl) is active.
    // @param modifier The keycode of the modifier to check.
    // @return `true` if the modifier is active, `false` otherwise.
    NV_API bool IsModifierActive(Modifier modifier) noexcept;

    // Starts text input capture mode.
    // This should be called before using text input functions.
    NV_API void BeginTextInput() noexcept;

    // Ends text input capture mode and retrieves the input text.
    NV_API const std::wstring_view EndTextInput() noexcept;

    // Retrieves the current text input.
    NV_API const std::wstring_view GetTextInput() noexcept;

    // Retrieves the capacity of the current text input buffer.
    // @return The size of the text input buffer.
    NV_API size_t GetTextInputCapacity() noexcept;

    // Sets the capacity of the text input buffer.
    // @param newSize The new buffer size.
    NV_API void SetTextInputCapacity(size_t newSize);

    // Checks if text input mode is currently active.
    // @return `true` if text input mode is active, `false` otherwise.
    NV_API bool IsTextInputActive() noexcept;

#pragma region Internal
    void _UpdateKey(Key key, uint8_t state) noexcept;
    void _AppendTextChar(wchar_t character) noexcept;
    void _UpdateButton(Button button, bool isPressed) noexcept;
    void _UpdateModifiers(Modifier modifiers) noexcept;
    void _UpdateMousePos(double x, double y) noexcept;
    void _UpdateMouseScroll(double vertical, double horizontal) noexcept;
#pragma endregion
};
