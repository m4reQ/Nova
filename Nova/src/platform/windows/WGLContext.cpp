#include <Nova/platform/windows/WGLContext.hpp>
#include <glad/wgl.h>
#include <stdexcept>

Nova::WGLContext::WGLContext(HDC deviceContext)
    : context_(wglCreateContext(deviceContext))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create WGL context.");
}

Nova::WGLContext::WGLContext(HDC deviceContext, std::span<const int> attribs)
    : context_(wglCreateContextAttribsARB(deviceContext, nullptr, attribs.data()))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create modern WGL context.");
}

Nova::WGLContext::~WGLContext() noexcept
{
    if (context_ != nullptr)
    {
        if (wglGetCurrentContext() == context_)
            wglMakeCurrent(wglGetCurrentDC(), nullptr);

        wglDeleteContext(context_);
    }
}

void Nova::WGLContext::MakeCurrent(HDC deviceContext) const
{
    if (!wglMakeCurrent(deviceContext, context_))
        throw std::runtime_error("Failed to make WGL context current.");
}

void Nova::WGLContext::LoadModern(HDC deviceContext) const
{
    if (!gladLoadWGL(deviceContext, reinterpret_cast<GLADloadfunc>(wglGetProcAddress)))
        throw std::runtime_error("Failed to load modern WGL function pointers.");
}