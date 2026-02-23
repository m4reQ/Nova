#pragma once
#include <glfw/glfw3.h>
#include <type_traits>

namespace Nova
{
    enum class Modifier
    {
        Shift = GLFW_MOD_SHIFT,
        Control = GLFW_MOD_CONTROL,
        Alt = GLFW_MOD_ALT,
        Super = GLFW_MOD_SUPER,
        CapsLock = GLFW_MOD_CAPS_LOCK,
        NumLock = GLFW_MOD_NUM_LOCK,
    };

    constexpr Modifier operator|(Modifier a, Modifier b) noexcept
    {
        return (Modifier)((std::underlying_type_t<Modifier>)a | (std::underlying_type_t<Modifier>)b);
    }

    constexpr Modifier operator&(Modifier a, Modifier b) noexcept
    {
        return (Modifier)((std::underlying_type_t<Modifier>)a & (std::underlying_type_t<Modifier>)b);
    }

    constexpr Modifier operator^(Modifier a, Modifier b) noexcept
    {
        return (Modifier)((std::underlying_type_t<Modifier>)a ^ (std::underlying_type_t<Modifier>)b);
    }

    constexpr Modifier operator~(Modifier mod) noexcept
    {
        return (Modifier) ~(std::underlying_type_t<Modifier>)mod;
    }
}
