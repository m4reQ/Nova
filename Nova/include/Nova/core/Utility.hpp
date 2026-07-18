#pragma once
#include <Nova/core/Memory.hpp>
#include <xxhash.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>
#include <algorithm>

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
        XXH64_hash_t operator()(const T &value) const noexcept
        {
            static_assert(std::is_trivially_copyable_v<T>, "xxHash hasher can hash only POD structs.");
            return XXH3_64bits(&value, sizeof(T));
        }
    };

    inline bool InsensitiveEquals(const std::string_view a, const std::string_view b) noexcept
    {
        return std::equal(
            a.begin(),
            a.end(),
            b.begin(),
            b.end(),
            [](char a, char b)
            {
                return std::tolower(a) == std::tolower(b);
            });
    }

    template <class... T>
    struct Overloaded : T...
    {
        using T::operator()...;
    };

    template <class... T>
    Overloaded(T...) -> Overloaded<T...>;

    inline void FitContentPreserveAspectRatio(
        float width,
        float height,
        float availableWidth,
        float availableHeight,
        float &finalWidth,
        float &finalHeight) noexcept
    {
        const auto scaleX = availableWidth / width;
        const auto scaleY = availableHeight / height;
        const auto scale = std::min(scaleX, scaleY);

        finalWidth = width * scale;
        finalHeight = height * scale;
    }

    inline void CenterContent(
        float width,
        float height,
        float availableWidth,
        float availableHeight,
        float &offsetX,
        float &offsetY) noexcept
    {
        offsetX = (availableWidth - width) / 2.0f;
        offsetY = (availableHeight - height) / 2.0f;
    }
}
