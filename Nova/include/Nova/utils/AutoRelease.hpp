#pragma once
#include <functional>
#include <utility>
#include <optional>
#include <type_traits>

namespace Nova
{
    template <typename T>
        requires std::is_trivially_copyable_v<T>
    class AutoRelease
    {
    public:
        using Deleter = std::function<void(T)>;

        constexpr AutoRelease() noexcept
            : value_(std::nullopt),
              deleter_(nullptr) {}

        constexpr AutoRelease(T value, Deleter deleter)
            : value_(value),
              deleter_(deleter) {}

        template <typename... Args>
        constexpr AutoRelease(Args &&...args, Deleter deleter)
            : value_(T(std::forward<Args>(args)...)),
              deleter_(deleter) {}

        AutoRelease(const AutoRelease<T> &) = delete;

        constexpr AutoRelease(AutoRelease<T> &&) noexcept = default;

        ~AutoRelease() noexcept
        {
            if (value_.has_value() && deleter_ != nullptr)
                deleter_(value_.value());
        }

        constexpr T &Get() noexcept { return value_.value(); }

        constexpr const T &Get() const noexcept { return value_.value(); }

        constexpr bool HasValue() const noexcept { return value_.has_value(); }

        constexpr bool operator==(const T &other) const noexcept { return value_.has_value() && (value_.get() == other); }

        constexpr bool operator!=(const T &other) const noexcept { return !value_.has_value() || (value_.has_value() && (value_.get() != other)); }

        AutoRelease<T> &operator=(const AutoRelease<T> &) = delete;

        AutoRelease<T> &operator=(AutoRelease<T> &&) noexcept = default;

        constexpr T Reset() noexcept
        {
            auto old = value_.value();
            value_ = std::nullopt;
            return old;
        }

    private:
        std::optional<T> value_;
        Deleter deleter_;
    };
}