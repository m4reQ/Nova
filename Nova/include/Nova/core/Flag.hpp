#pragma once
#include <type_traits>

#define NV_DEFINE_BITWISE_OPERATOR(type, _operator)                                            \
    constexpr type operator##_operator(type a, type b) noexcept                                \
    {                                                                                          \
        using EnumType = std::underlying_type_t<type>;                                         \
        return static_cast<type>(static_cast<EnumType>(a) _operator static_cast<EnumType>(b)); \
    }

#define NV_DEFINE_INPLACE_OPERATOR(type, _operator)                                                 \
    constexpr type &operator##_operator(type & a, const type & b) noexcept                          \
    {                                                                                               \
        using EnumType = std::underlying_type_t<type>;                                              \
        static_cast<EnumType &>(a) = static_cast<EnumType &>(a) | static_cast<const EnumType &>(b); \
        return a;                                                                                   \
    }

#define NV_DEFINE_BITWISE_OPERATORS(type)                        \
    NV_DEFINE_BITWISE_OPERATOR(type, &)                          \
    NV_DEFINE_BITWISE_OPERATOR(type, |)                          \
    NV_DEFINE_BITWISE_OPERATOR(type, ^)                          \
    constexpr type operator~(type value) noexcept                \
    {                                                            \
        using EnumType = std::underlying_type_t<type>;           \
        return static_cast<type>(~static_cast<EnumType>(value)); \
    }

namespace Nova::Flag
{
    template <typename T>
    concept FlagsType = std::is_enum_v<T> || std::is_integral_v<T>;

    template <FlagsType T>
    constexpr auto ToBits_(T value) noexcept
    {
        if constexpr (std::is_enum_v<T>)
            return static_cast<std::underlying_type_t<T>>(value);
        else
            return value;
    }

    template <FlagsType T>
    constexpr bool IsSet(T flags, T flag) noexcept
    {
        return (ToBits_(flags) & ToBits_(flag)) == ToBits_(flag);
    }

    template <FlagsType T>
    constexpr void Clear(T &flags, T flag) noexcept
    {
        flags = static_cast<T>(ToBits_(flags) & ~ToBits_(flag));
    }

    template <FlagsType T>
    constexpr void Set(T &flags, T flag) noexcept
    {
        flags = static_cast<T>(ToBits_(flags) | ToBits_(flag));
    }
}