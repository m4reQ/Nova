#pragma once
#include <cstdint>

namespace Nova
{
    struct MagicNumber
    {
        uint8_t Bytes[8];

        bool IsPNG() const noexcept;

        bool IsJPEG() const noexcept;

        bool IsBMP() const noexcept;

        bool IsDDS() const noexcept;
    };
}