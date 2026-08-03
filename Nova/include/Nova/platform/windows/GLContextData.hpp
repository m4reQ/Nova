#pragma once
#include <Windows.h>
#include <glad/wgl.h>

namespace Nova
{
    struct GLContextData
    {
        HDC deviceContext = nullptr;
        HGLRC context = nullptr;
    };
}