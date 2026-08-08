#include <Nova/platform/windows/WGLContext.hpp>
#include <Nova/debug/Profile.hpp>
#include <glad/wgl.h>
#include <stdexcept>

Nova::WGLContext::WGLContext(HGLRC context) noexcept
    : context_(
          context,
          [](auto x)
          {
              if (x != nullptr)
              {
                  if (wglGetCurrentContext() == x)
                      wglMakeCurrent(wglGetCurrentDC(), nullptr);

                  wglDeleteContext(x);
              }
          }) {}

Nova::WGLContext::WGLContext(HDC deviceContext)
    : WGLContext(wglCreateContext(deviceContext))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create WGL context.");
}

Nova::WGLContext::WGLContext(HDC deviceContext, std::span<const int> attribs)
    : WGLContext(wglCreateContextAttribsARB(deviceContext, nullptr, attribs.data()))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to create modern WGL context.");
}

void Nova::WGLContext::MakeCurrent(HDC deviceContext) const
{
    NV_PROFILE_FUNC;

    if (!wglMakeCurrent(deviceContext, context_.Get()))
        throw std::runtime_error("Failed to make WGL context current.");
}

void Nova::WGLContext::LoadModern(HDC deviceContext) const
{
    NV_PROFILE_FUNC;

    if (!gladLoadWGL(deviceContext, reinterpret_cast<GLADloadfunc>(wglGetProcAddress)))
        throw std::runtime_error("Failed to load modern WGL function pointers.");
}