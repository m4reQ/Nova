#include <Nova/graphics/opengl/Context.hpp>
#include <Nova/platform/windows/DeviceContext.hpp>
#include <Nova/platform/windows/WGLContext.hpp>
#include <Nova/platform/windows/Error.hpp>
#include <Nova/debug/Profile.hpp>
#include <glad/wgl.h>
#include <stdexcept>
#include <array>

static GLADapiproc LoadGLFunction(const char *funcName)
{
    auto func = static_cast<void *>(wglGetProcAddress(funcName));
    if (func == nullptr ||
        func == reinterpret_cast<void *>(0x1) ||
        func == reinterpret_cast<void *>(0x2) ||
        func == reinterpret_cast<void *>(0x3) ||
        func == reinterpret_cast<void *>(-1))
    {
        static auto glLib = LoadLibraryA("opengl32.dll");
        func = static_cast<void *>(GetProcAddress(glLib, funcName));
    }

    return static_cast<GLADapiproc>(func);
}

static void SelectPixelFormat(HDC deviceContext, std::span<const int> formatAttributes)
{
    auto formatIndex = 0;
    auto foundFormatsCount = 0u;
    const auto formatFound = wglChoosePixelFormatARB(
        deviceContext,
        formatAttributes.data(),
        nullptr,
        1,
        &formatIndex,
        &foundFormatsCount);
    if (!formatFound || foundFormatsCount == 0)
        throw std::runtime_error("Failed to find suitable pixel format.");

    PIXELFORMATDESCRIPTOR desc{
        .nSize = sizeof(desc),
    };
    if (!DescribePixelFormat(deviceContext, formatIndex, sizeof(PIXELFORMATDESCRIPTOR), &desc))
        throw Nova::Win32::Exception("Failed to describe OpenGL pixel format.");

    if (!SetPixelFormat(deviceContext, formatIndex, nullptr))
        throw Nova::Win32::Exception("Failed to set pixel format.");
}

static void LoadModernWGL(HINSTANCE hInstance)
{
    NV_PROFILE_FUNC;

    // Create dummy window
    Nova::WindowHandle windowHandle(
        0,
        "STATIC",
        "",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        500,
        hInstance,
        nullptr);
    Nova::DeviceContext dc(windowHandle);

    const PIXELFORMATDESCRIPTOR pfd{
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
        .cAlphaBits = 0,
        .cDepthBits = 0,
        .cStencilBits = 0,
        .iLayerType = PFD_MAIN_PLANE,
    };
    const auto pfdIndex = ChoosePixelFormat(dc, &pfd);
    if (!pfdIndex)
        throw std::runtime_error("Failed to find suitable pixel format for dummy window.");

    if (!SetPixelFormat(dc, pfdIndex, &pfd))
        throw std::runtime_error("Failed to set pixel format for dummy window.");

    // Create old context to load WGL functions
    Nova::WGLContext legacyContext(dc);
    legacyContext.MakeCurrent(dc);
    legacyContext.LoadModern(dc);
}

Nova::GLContext::GLContext(const WindowData &windowData, unsigned int versionMajor, unsigned int versionMinor)
{
    NV_PROFILE_FUNC;

    LoadModernWGL(windowData.instance);

    // Create modern context
    data_.deviceContext = windowData.deviceContext;

    const auto formatAttribs = std::array<int, 17>({
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        24,
        WGL_ALPHA_BITS_ARB,
        8,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,
        0,
    });
    SelectPixelFormat(data_.deviceContext, formatAttribs);

#ifdef NV_DEBUG
    constexpr auto contextFlags = WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#else
    constexpr auto contextFlags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
#endif

    const auto contextAttribs = std::array<int, 9>({
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        static_cast<int>(versionMajor),
        WGL_CONTEXT_MINOR_VERSION_ARB,
        static_cast<int>(versionMinor),
        WGL_CONTEXT_FLAGS_ARB,
        contextFlags,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    });

    data_.context = wglCreateContextAttribsARB(
        data_.deviceContext,
        nullptr,
        contextAttribs.data());
    if (data_.context == nullptr)
        throw Win32::Exception("Failed to create WGL context.");
}

Nova::GLContext::~GLContext() noexcept
{
    if (data_.context != nullptr)
    {
        wglMakeCurrent(data_.deviceContext, nullptr);
        wglDeleteContext(data_.context);
    }
}

void Nova::GLContext::MakeCurrent()
{
    NV_PROFILE_FUNC;

    if (!wglMakeCurrent(data_.deviceContext, data_.context))
        throw Win32::Exception("Failed to make WGL context current");
}

void Nova::GLContext::LoadGL()
{
    NV_PROFILE_FUNC;

    if (!gladLoadGL(LoadGLFunction))
        throw Win32::Exception("Failed to load OpenGL function pointers");
}

double Nova::GLContext::GetRefreshRate()
{
    return static_cast<double>(GetDeviceCaps(data_.deviceContext, VREFRESH));
}