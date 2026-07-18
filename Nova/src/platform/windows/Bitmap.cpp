#include <Nova/platform/windows/Bitmap.hpp>
#include <stdexcept>

Nova::Bitmap::Bitmap(
    int width,
    int height,
    UINT planes,
    UINT bitCount,
    std::span<const std::byte> bits)
    : Bitmap(CreateBitmap(width, height, planes, bitCount, bits.data()))
{
    if (handle_ == nullptr)
        throw std::runtime_error("Failed to create bitmap.");
}

Nova::Bitmap::~Bitmap() noexcept
{
    if (handle_ != nullptr)
        DeleteObject(handle_);
}