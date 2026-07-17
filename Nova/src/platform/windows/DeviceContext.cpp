#include <Nova/platform/windows/DeviceContext.hpp>
#include <stdexcept>

Nova::DeviceContext::DeviceContext(HWND window)
    : DeviceContext(GetDC(window))
{
    if (context_ == nullptr)
        throw std::runtime_error("Failed to get window DC.");
}

Nova::DeviceContext::~DeviceContext() noexcept
{
    if (context_ != nullptr)
        if (freeContext_)
            DeleteDC(context_);
        else
            ReleaseDC(WindowFromDC(context_), context_);
}