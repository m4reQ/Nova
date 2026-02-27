#pragma once
#include <Nova/core/Memory.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include <xxhash.h>

#define NV_DEFINE_BITWISE_OPERATOR(type, _operator)                                            \
    constexpr type operator##_operator(type a, type b) noexcept                                \
    {                                                                                          \
        using EnumType = std::underlying_type_t<type>;                                         \
        return static_cast<type>(static_cast<EnumType>(a) _operator static_cast<EnumType>(b)); \
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

namespace Nova
{
    template <typename T, typename DataType = void>
    concept HasArrayInterface =
        requires(T t) {
            typename T::value_type;

            { t.size() } -> std::same_as<size_t>;
            { t.data() } -> std::same_as<typename T::value_type *>;
        };

    template <typename T, typename DataType = void>
    concept HasConstArrayInterface =
        requires(const T ct) {
            typename T::value_type;

            { ct.size() } -> std::same_as<size_t>;
            { ct.data() } -> std::same_as<const typename T::value_type *>;
        };

    template <class Tag, typename T>
    class StrongTypedef
    {
    public:
        constexpr StrongTypedef() noexcept
            : m_Value() {}

        constexpr explicit StrongTypedef(const T &value) noexcept
            : m_Value(value) {}

        constexpr explicit StrongTypedef(T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
            : m_Value(std::move(value)) {}

        constexpr explicit operator T &() noexcept { return m_Value; }

        constexpr explicit operator const T &() const noexcept { return m_Value; }

        friend void swap(StrongTypedef &a, StrongTypedef &b) noexcept { std::swap(static_cast<T &>(a), static_cast<T &>(b)); }

    private:
        T m_Value;
    };

    struct StringHash
    {
        using is_transparent = void;

        std::size_t operator()(const char *str) const { return std::hash<std::string_view>{}(str); }
        std::size_t operator()(std::string_view str) const { return std::hash<std::string_view>{}(str); }
        std::size_t operator()(const std::string &str) const { return std::hash<std::string>{}(str); }
    };

    template <typename T>
    struct XXHasher
    {
        XXH64_hash_t operator()(const T& value) const noexcept
        {
            static_assert(std::is_trivially_copyable_v<T>, "xxHash hasher can hash only POD structs.");
            return XXH3_64bits(&value, sizeof(T));
        }
    };

    template <typename T>
    constexpr bool IsFlagSet(T flags, T bit) noexcept
    {
        return (flags && bit) == bit;
    }
}
