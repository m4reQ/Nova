#pragma once
#include <concepts>
#include <limits>
#include <memory>

// default_delete_for: gives the correct default_delete<T> or default_delete<T[]>
template <typename T>
struct default_delete_for
{
    using type = std::default_delete<T>;
};

template <typename T>
struct default_delete_for<T[]>
{
    using type = std::default_delete<T[]>;
};

// Build result element type and reapply array-ness while preserving const from SrcU
template <typename SrcU, typename DestT>
struct preserve_const_and_extent
{
    using src_elem = std::remove_extent_t<SrcU>;   // e.g. Derived or const Derived
    using dest_elem = std::remove_extent_t<DestT>; // e.g. Base

    // if source element is const, make dest element const
    using result_elem = std::conditional_t<
        std::is_const_v<src_elem>,
        std::add_const_t<std::remove_cv_t<dest_elem>>,
        std::remove_cv_t<dest_elem>>;

    // reapply array-ness according to SrcU (if SrcU is an array, result is result_elem[])
    using type = std::conditional_t<std::is_array_v<SrcU>, result_elem[], result_elem>;
};

// --- static cast ---
template <typename TTarget, typename U, typename D>
auto unique_ptr_cast(std::unique_ptr<U, D> &&src)
{
    using ResultT = typename preserve_const_and_extent<U, TTarget>::type;
    using ResultDeleter = typename default_delete_for<ResultT>::type;
    using ResultUPtr = std::unique_ptr<ResultT, ResultDeleter>;
    using ResultPtr = typename ResultUPtr::pointer;

    // release the raw pointer
    auto raw = src.release();

    // perform the static cast on the raw pointer
    ResultPtr casted = reinterpret_cast<ResultPtr>(raw);

    if constexpr (std::is_convertible_v<D, ResultDeleter>)
    {
        // move the original deleter if it can convert to the target deleter
        return ResultUPtr(casted, std::move(src.get_deleter()));
    }
    else
    {
        // otherwise construct with default deleter
        return ResultUPtr(casted);
    }
}

template <typename TDst, typename TSrc>
constexpr bool check_fits_in(TSrc value) noexcept
{
    if (!std::numeric_limits<TDst>::is_integer)
    {
        return  (value > 0 ? value : -value) <= std::numeric_limits<TDst>::max();
    }

    if (std::numeric_limits<TDst>::is_signed == std::numeric_limits<TSrc>::is_signed)
    {
        return value >= std::numeric_limits<TDst>::min()
            && value <= std::numeric_limits<TDst>::max();
    }
    else if (std::numeric_limits<TDst>::is_signed)
    {
        return value <= std::numeric_limits<TDst>::max();
    }
    else
    {
        return value >= 0
            && value <= std::numeric_limits<TDst>::max();
    }
}
