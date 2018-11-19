
#pragma once

#include <meta.hpp>

namespace tom {

namespace detail {

template <class T>
bool constexpr workaround_against_unions() {
    static_assert(!std::is_union_v<T>,
        "Don't try to detect if a structure containing unions can be destructured. "
        "It is not, but the SFINAE deduction can't correctly eliminate this case. "
        "Feel free to remove this assert if you have a better solution.");
    return true;
}

template <class T>
using workaround_against_unions_t = std::enable_if_t<workaround_against_unions<T>()>;

struct wildcard {
    template <class T, class = workaround_against_unions_t<T>>
    operator T() const;

    template <size_t I>
    static wildcard any();
};

// Detects brace constructor with N arguments for a given type.

namespace impl {
    template <class T, class Sequence, class SFINAE = void>
    constexpr bool is_brace_constructible_v = false;

    template <class T, size_t...Is>
    constexpr bool is_brace_constructible_v<T, std::index_sequence<Is...>, std::void_t<decltype(
        T{ wildcard::any<Is>()... }
    )>>  = true;
} // ::impl

template <class T, size_t N>
constexpr bool is_brace_constructible_v = impl::is_brace_constructible_v<T, std::make_index_sequence<N>>;

// The 'as_tuple' functions for each airity implemented (up to 10 here).

constexpr int max_arity = 10;

#define fwd(x) static_cast<std::conditional_t<std::is_reference_v<T>, decltype(x) &, decltype(x)>>(x)

template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 0>) {
    return std::forward_as_tuple();
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 1>) {
    auto && [v1, v2, v3, v4, v5] = val;
    return std::forward_as_tuple(fwd(v1));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 2>) {
    auto && [v1, v2] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 3>) {
    auto && [v1, v2, v3] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 4>) {
    auto && [v1, v2, v3, v4] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 5>) {
    auto && [v1, v2, v3, v4, v5] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 6>) {
    auto && [v1, v2, v3, v4, v5, v6] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5), fwd(v6));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 7>) {
    auto && [v1, v2, v3, v4, v5, v6, v7] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5), fwd(v6), fwd(v7));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 8>) {
    auto && [v1, v2, v3, v4, v5, v6, v7, v8] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5), fwd(v6), fwd(v7), fwd(v8));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 9>) {
    auto && [v1, v2, v3, v4, v5, v6, v7, v8, v9] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5), fwd(v6), fwd(v7), fwd(v8), fwd(v9));
}
template <class T>
constexpr auto as_tuple_impl(T && val, std::integral_constant<int, 10>) {
    auto && [v1, v2, v3, v4, v5, v6, v7, v8, v9, v10] = val;
    return std::forward_as_tuple(fwd(v1), fwd(v2), fwd(v3), fwd(v4), fwd(v5), fwd(v6), fwd(v7), fwd(v8), fwd(v9), fwd(v10));
}

#undef fwd

// Computes the number of elements in the aggregate.

template <class T, size_t N>
constexpr int get_airity() {

    static_assert(N <= max_arity + 1,
        "Not enough functions are available to interpret T as a tuple. "
        "You can increase the number of these functions by hand. "
        "If the type is or contains an array, remember that native arrays are not supported. "
        "Consider handling static arrays before treating them as aggregates if they are too big.");

    if constexpr (is_brace_constructible_v<T, N>) {
        return get_airity<T, N + 1>();
    }
    else return N - 1;
}

} // ::detail

// Returns the number of elements in an aggregate, or -1 if the type can't
// be destructured.

template <class T>
constexpr int airity_v =
    (std::is_aggregate_v<T> && !std::is_union_v<T>)
    ? detail::get_airity<T, 0>() : -1;

template <class T>
constexpr bool is_aggregate_v = airity_v<T> >= 0;

template <class T, class = std::enable_if_t<
    is_aggregate_v<remove_cvref_t<T>>
>>
constexpr auto as_tuple(T && aggregate) noexcept {
    using tag = std::integral_constant<int, airity_v<remove_cvref_t<T>>>;
    return detail::as_tuple(TOM_FWD(aggregate), tag{});
}

template <class Aggregate>
using as_tuple_t = typename std::remove_reference_t<decltype(
    as_tuple(std::declval<Aggregate &>())
)>;

} // ::tom
