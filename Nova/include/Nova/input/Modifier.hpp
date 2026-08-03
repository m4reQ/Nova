#pragma once
#include <Nova/core/Flag.hpp>

namespace Nova
{
    enum class Modifier
    {
        None = 0,
        Shift = 1,
        Control = 2,
        Alt = 4,
        Super = 8,
        CapsLock = 16,
        NumLock = 32,
    };

    NV_DEFINE_BITWISE_OPERATORS(Modifier);
}
