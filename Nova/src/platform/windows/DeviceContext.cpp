#include <Nova/platform/windows/DeviceContext.hpp>
#include <stdexcept>

Nova::DeviceContext::DeviceContext(HDC &&dc) noexcept
    : handle_(
          std::forward<HDC>(dc),
          [](auto x)
          {
              if (x != nullptr)
                  DeleteDC(x);
          })
{
}

Nova::DeviceContext::DeviceContext(HWND window)
    : handle_(
          GetDC(window),
          [](auto x)
          {
              if (x != nullptr)
                  ReleaseDC(WindowFromDC(x), x);
          })
{
    if (handle_ == nullptr)
        throw std::runtime_error("Failed to get window DC.");
}

Nova::DeviceContext::DeviceContext(
    const std::wstring_view driver,
    const std::wstring_view device,
    const std::wstring_view port,
    const DEVMODEW &pdm)
    : DeviceContext(
          CreateDCW(
              driver.data(),
              device.data(),
              port.data(),
              &pdm))
{
}

Nova::DeviceContext::DeviceContext(
    const std::string_view driver,
    const std::string_view device,
    const std::string_view port,
    const DEVMODEA &pdm)
    : DeviceContext(
          CreateDCA(
              driver.data(),
              device.data(),
              port.data(),
              &pdm))
{
}