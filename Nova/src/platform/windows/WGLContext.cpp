#include <Nova/platform/windows/WGLContext.hpp>
#include <stdexcept>

Nova::WGLContext::WGLContext(HDC deviceContext)
    : context_(wglCreateContext(deviceContext))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create WGL context.");

    if (!wglMakeCurrent(deviceContext, context_))
        throw std::runtime_error("Failed to make WGL context current.");
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