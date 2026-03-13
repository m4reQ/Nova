#include <Nova/graphics/MagicNumber.hpp>
#include <cstring>

using namespace Nova;

bool MagicNumber::IsPNG() const noexcept
{
    constexpr uint8_t PNGMagicBytes[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    return memcmp(Bytes, PNGMagicBytes, sizeof(PNGMagicBytes)) == 0;
}

bool MagicNumber::IsJPEG() const noexcept
{
    constexpr uint8_t JPEGMagicBytes[3] = {0xFF, 0xD8, 0xFF};
    return memcmp(Bytes, JPEGMagicBytes, sizeof(JPEGMagicBytes)) == 0;
}

bool MagicNumber::IsBMP() const noexcept
{
    constexpr uint8_t BMPMagicBytes[2] = {0x42, 0x4D};
    return memcmp(Bytes, BMPMagicBytes, sizeof(BMPMagicBytes)) == 0;
}

bool MagicNumber::IsDDS() const noexcept
{
    constexpr uint8_t DDSMagicBytes[4] = {0x44, 0x44, 0x53, 0x20};
    return memcmp(Bytes, DDSMagicBytes, sizeof(DDSMagicBytes)) == 0;
}