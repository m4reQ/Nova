#pragma once
#include <Nova/core/Build.hpp>
#include <Nova/core/Flag.hpp>

namespace Nova
{
    enum class Button
    {
        None = 0,
        Left = 1,
        Middle = 2,
        Right = 4,
        Extra1 = 8,
        Extra2 = 16,
        Unknown = 32,
    };

    enum class Modifier
    {
        None = 0,
        Shift = 1,
        Control = 2,
        Alt = 4,
        Super = 8,
        CapsLock = 16,
        NumLock = 32,
    };

    enum class Key
    {
        Unknown,
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Number0,
        Number1,
        Number2,
        Number3,
        Number4,
        Number5,
        Number6,
        Number7,
        Number8,
        Number9,
        Semicolon,
        Equal,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        WorldLeft,
        WorldRight,
        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        Kp0,
        Kp1,
        Kp2,
        Kp3,
        Kp4,
        Kp5,
        Kp6,
        Kp7,
        Kp8,
        Kp9,
        KpDecimal,
        KpDivide,
        KpMultiply,
        KpSubtract,
        KpAdd,
        KpEnter,
        KpEqual,
        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper,
        RightShift,
        RightControl,
        RightAlt,
        RightSuper,
        Menu,
        VolumeMute,
        VolumeUp,
        VolumeDown,
        EnumMax_
    };

    namespace Input
    {
        /// @brief Checks if a specific key is currently pressed down.
        /// @param key The keycode of the key to check.
        /// @return `true` if the key is down, `false` otherwise.
        NV_API bool IsKeyDown(Key key) noexcept;

        /// @brief Checks if a specific key is currently released.
        /// @param key The keycode of the key to check.
        /// @return `true` if the key is up, `false` otherwise.
        NV_API bool IsKeyUp(Key key) noexcept;

        /// @brief Checks if a specific mouse button is currently pressed down.
        /// @param button The button code of the mouse button to check.
        /// @return `true` if the button is down, `false` otherwise.
        NV_API bool IsButtonDown(Button button) noexcept;

        /// @brief Checks if a specific mouse button is currently released.
        /// @param button The button code of the mouse button to check.
        /// @return `true` if the button is up, `false` otherwise.
        NV_API bool IsButtonUp(Button button) noexcept;

        /// @brief Retrieves the current X-coordinate of the mouse cursor.
        /// @return The X-coordinate of the mouse cursor as a double.
        NV_API double GetMouseX() noexcept;

        /// @brief Retrieves the current Y-coordinate of the mouse cursor.
        /// @return The Y-coordinate of the mouse cursor as a double.
        NV_API double GetMouseY() noexcept;

        /// @brief Retrieves the current position of the mouse cursor.
        /// @return Current mouse position.
        NV_API std::pair<double, double> GetMousePos() noexcept;

        /// @brief Retrieves current mouse vertical scroll offset.
        /// @return Current vertical scroll offset.
        NV_API double GetMouseVScroll() noexcept;

        /// @brief Retrieves current mouse horizontal scroll offset.
        /// @return Current horizontal scroll offset.
        NV_API double GetMouseHScroll() noexcept;

        /// @brief Checks if a specific modifier key (e.g., Shift, Ctrl) is active.
        /// @param modifier The keycode of the modifier to check.
        /// @return `true` if the modifier is active, `false` otherwise.
        NV_API bool IsModifierActive(Modifier modifier) noexcept;

        /// @brief Starts text capture mode.
        NV_API void BeginTextInput() noexcept;

        /// @brief Ends text input capture mode and retrieves the accumulated text.
        /// @return Text captured since last `Input::BeginTextInput` call.
        NV_API std::wstring EndTextInput() noexcept;

        /// @brief Checks if text input mode is currently active.
        /// @return `true` if text input mode is active, `false` otherwise.
        NV_API bool IsTextInputEnabled() noexcept;

        /// @brief Internal API. Don't use directly!
        void AppendTextChar_(wchar_t character) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateKey_(Key key, bool isDown) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateButton_(Button button, bool isPressed) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateButtons_(Button buttonsMask) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateModifiers_(Modifier modifiersMask) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateMousePos_(double x, double y) noexcept;

        /// @brief Internal API. Don't use directly!
        void UpdateMouseScroll_(double vertical, double horizontal) noexcept;
    }

    NV_DEFINE_BITWISE_OPERATORS(Modifier);

    NV_DEFINE_BITWISE_OPERATORS(Button);
};
