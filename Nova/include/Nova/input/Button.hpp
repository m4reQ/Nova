#pragma once
#include <Nova/core/Flag.hpp>

namespace Nova
{
    enum class Button
    {
        None = 0,
        Left = 1,
        Middle = 2,
        Right = 4,
        Extra1 = 8,
        Extra2 = 16,
        Unknown = 32,
    };

    NV_DEFINE_BITWISE_OPERATORS(Button);
}
