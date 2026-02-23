#pragma once
#include <glfw/glfw3.h>

namespace Nova
{
    enum class Button
    {
        _1 = GLFW_MOUSE_BUTTON_1,
        _2 = GLFW_MOUSE_BUTTON_2,
        _3 = GLFW_MOUSE_BUTTON_3,
        _4 = GLFW_MOUSE_BUTTON_4,
        _5 = GLFW_MOUSE_BUTTON_5,
        _6 = GLFW_MOUSE_BUTTON_6,
        _7 = GLFW_MOUSE_BUTTON_7,
        _8 = GLFW_MOUSE_BUTTON_8,
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        EnumMax = GLFW_MOUSE_BUTTON_LAST,
    };
}
