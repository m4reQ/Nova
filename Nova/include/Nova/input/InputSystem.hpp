#pragma once
#include <Nova/input/Button.hpp>
#include <Nova/input/Modifier.hpp>
#include <Nova/input/Key.hpp>
#include <Nova/core/Flag.hpp>
#include <vector>
#include <array>
#include <utility>
#include <string>

namespace Nova
{
    class InputSystem
    {
    public:
        InputSystem() = default;

        InputSystem(const InputSystem &) = delete;

        InputSystem(InputSystem &&) noexcept = default;

        InputSystem &operator=(const InputSystem &) = delete;

        InputSystem &operator=(InputSystem &&) noexcept = default;

        constexpr bool IsButtonDown(Button button) const noexcept { return Flag::IsSet(buttonMask_, button); }

        constexpr bool IsButtonUp(Button button) const noexcept { return !IsButtonDown(button); }

        constexpr bool IsKeyDown(Key key) const noexcept { return keys_[static_cast<size_t>(key)]; }

        constexpr bool IsKeyUp(Key key) const noexcept { return !IsKeyDown(key); }

        constexpr bool IsModifierActive(Modifier modifier) const noexcept { return Flag::IsSet(modifierMask_, modifier); }

        constexpr double GetMouseX() const noexcept { return mouseX_; }

        constexpr double GetMouseY() const noexcept { return mouseY_; }

        constexpr std::pair<double, double> GetMousePos() const noexcept { return {mouseX_, mouseY_}; }

        constexpr double GetMouseVScroll() const noexcept { return scrollV_; }

        constexpr double GetMouseHScroll() const noexcept { return scrollH_; }

        constexpr std::pair<double, double> GetMouseScroll() const noexcept { return {scrollV_, scrollH_}; }

        constexpr bool IsTextInputEnabled() const noexcept { return textInputEnabled_; }

        void BeginTextInput() noexcept;

        std::wstring EndTextInput() noexcept;

        constexpr void UpdateKey(Key key, bool isDown) noexcept
        {
            keys_[static_cast<size_t>(key)] = isDown;
        }

        constexpr void UpdateButton(Button button, bool isPressed) noexcept
        {
            if (isPressed)
                Flag::Set(buttonMask_, button);
            else
                Flag::Clear(buttonMask_, button);
        }

        constexpr void UpdateButtons(Button buttonMask) noexcept
        {
            buttonMask_ = buttonMask;
        }

        constexpr void UpdateModifiers(Modifier modifierMask) noexcept
        {
            modifierMask_ = modifierMask;
        }

        constexpr void UpdateMousePos(double x, double y) noexcept
        {
            mouseX_ = x;
            mouseY_ = y;
        }

        constexpr void UpdateMouseVScroll(double v) noexcept
        {
            scrollV_ = v;
        }

        constexpr void UpdateMouseHScroll(double h) noexcept
        {
            scrollH_ = h;
        }

        constexpr void UpdateMouseScroll(double v, double h) noexcept
        {
            scrollV_ = v;
            scrollH_ = h;
        }

        // Make this constexpr by using own std::iswprint replacement
        void AppendTextChar(wchar_t character) noexcept;

    private:
        std::array<bool, static_cast<size_t>(Key::EnumMax_)> keys_;
        std::vector<wchar_t> textBuffer_;
        Modifier modifierMask_ = Modifier::None;
        Button buttonMask_ = Button::None;
        double mouseX_ = 0.0;
        double mouseY_ = 0.0;
        double scrollV_ = 0.0;
        double scrollH_ = 0.0;
        bool textInputEnabled_ = false;
    };
}