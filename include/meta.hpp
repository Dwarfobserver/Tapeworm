
#pragma once

#include <utility>

namespace tom {

// (in C++20) Removes volatile, const and reference qualifiers from the type.
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

// (in C++20) Detects if an expression is well-formed with the given template parameters (through SFINAE).

namespace detail
{
    template <template <class...> class Expr, class SFINAE, class...Args>
    constexpr bool is_detected = false;
    template <template <class...> class Expr, class...Args>
    constexpr bool is_detected<Expr, std::void_t<Expr<Args...>>, Args...> = true;
} // ::detail

template <template <class...> class Expr, class...Args>
constexpr bool is_detected_v = detail::is_detected<Expr, void, Args...>;

// Vocabulary type to pass a type as a value.
template <class T>
struct type_tag {
    using type = T;
};

// Used to not trigger a compiler error in static_asserts.
template <class...Ts>
constexpr bool always_false_v = false;

// Drop-in replacements of std::forward and std:move.

#define TOM_FWD(x) std::forward<decltype(x)>(x)

template <class T>
constexpr decltype(auto) move(T&& value) noexcept {
    using value_type = std::remove_reference_t<T>;
    static_assert(!std::is_const_v<value_type>);
    static_assert(std::is_nothrow_move_constructible_v<value_type>
               && std::is_nothrow_move_assignable_v   <value_type>);
    return static_cast<value_type&&>(value);
}

// Removes const of the value type.
// Applied to a list of types, removes const on each type.
// Used to construct pairs or tuples with const values.

template <class T>
struct remove_deep_const {
    using type = T;
};
template <class T>
using remove_deep_const_t = typename remove_deep_const<T>::type;

template <class T>
struct remove_deep_const<T const> {
    using type = T;
};
template <template <class...> List, class...Ts>
struct remove_deep_const<List<Ts...>> {
    using type = List<remove_deep_const_t<Ts>...>;
};


// Sort a list of types based on a predicate.

template <
    class List,
    template <class, class> class Comparer
>
struct sort_list {};

namespace sort_detail {

    // Insert T in it's next elements.
    template <
        template <class, class> class Comparer,
        class BeforeList,
        class T,
        class AfterList
    >
    struct insert_element {};

    // Match the general case.
    template <
        template <class...> class List,
        template <class, class> class Comparer,
        class...Before,
        class T,
        class Next,
        class...After
    >
    struct insert_element<Comparer,
        List<Before...>,
        T,
        List<Next, After...>
    > {
        using type = std::conditional_t<
            Comparer<T, Next>::value, 
            // We found T position.
            List<Before..., T, Next, After...>,
            // We continue the serach.
            typename insert_element<Comparer,
                List<Before..., Next>,
                T,
                List<After...>
            >::type
        >;
    };

    // Match the list's end.
    template <
        template <class...> class List,
        template <class, class> class Comparer,
        class...Before,
        class T
    >
    struct insert_element<Comparer,
        List<Before...>,
        T,
        List<>
    > {
        using type = List<Before..., T>;
    };

    // Call 'insert' on each element.
    // The first elements are assured to be sorted beteen them.
    template <
        template <class, class> class Comparer,
        class SortedList,
        class T,
        class AfterList
    >
    struct iterate {};

    // Match the general case.
    template <
        template <class...> class List,
        template <class, class> class Comparer,
        class...Sorted,
        class T,
        class Next,
        class...After
    >
    struct iterate<Comparer,
        List<Sorted...>,
        T,
        List<Next, After...>
    > {
        using type = typename iterate<Comparer,
            typename insert_element<Comparer,
                List<>,
                T,
                List<Sorted...>
            >::type,
            Next,
            List<After...>
        >::type;
    };

    // Match the list's end.
    template <
        template <class...> class List,
        template <class, class> class Comparer,
        class...Sorted,
        class T
    >
    struct iterate<Comparer,
        List<Sorted...>,
        T,
        List<>
    > {
        using type = typename insert_element<Comparer,
            List<>,
            T,
            List<Sorted...>
        >::type;
    };

} // ::sort_detail

// Match the general case.
template <
    template <class...> class List,
    template <class, class> class Comparer,
    class T,
    class...Ts
>
struct sort_list<
    List<T, Ts...>,
    Comparer
> {
    using type = typename sort_detail::iterate<Comparer,
        List<>,
        T,
        List<Ts...>
    >::type;
};

// Match the empty list.
template <
    template <class...> class List,
    template <class, class> class Comparer
>
struct sort_list<
    List<>,
    Comparer
> {
    using type = List<>;
};

template <
    class List,
    template <class, class> class Comparer
>
using sort_list_t = typename sort_list<List, Comparer>::type;

// Find the first element respecting the given predicate or returns the default type.

template <
    class List,
    template <class> class Predicate,
    class DefaultT
>
struct find_first_or {};

// Match the general case.
template <
    template <class...> class List,
    template <class> class Predicate,
    class T,
    class...Ts,
    class DefaultT
>
struct find_first_or<
    List<T, Ts...>,
    Predicate,
    DefaultT
> {
    using type = typename std::conditional_t<
        Predicate<T>::value,
        T,
        typename find_first_or<
            List<Ts...>,
            Predicate,
            DefaultT
        >::type
    >;
};

// Match the empty list.
template <
    template <class...> class List,
    template <class> class Predicate,
    class DefaultT
>
struct find_first_or<List<>, Predicate, DefaultT> {
    using type = DefaultT;
};

template <
    class List,
    template <class> class Predicate,
    class DefaultT = void
>
using find_first_or_t = typename find_first_or<List, Predicate, DefaultT>::type;

// Map a list to another list with a meta-function.

namespace map_detail {

    template <class MappedList, class List, template <class> class Transformer>
    struct map {};
    
    // Match the general case.
    template <
        template <class...> class List,
        template <class> class Transformer,
        class...Mapped,
        class T,
        class...After
    >
    struct map<
        List<Mapped...>,
        List<T, After...>,
        Transformer
    > {
        using type = typename map<
            List<Mapped..., typename Transformer<T>::type>,
            List<After...>,
            Transformer
        >::type;
    };
    
    // Match the empty list.
    template <
        template <class...> class List,
        template <class> class Transformer,
        class...Mapped
    >
    struct map<
        List<Mapped...>,
        List<>,
        Transformer
    > {
        using type = List<Mapped...>;
    };

} // ::map_detail

template <class List, template <class> class Transformer>
struct map_list {};

template <
    template <class...> class List,
    template <class> class Transformer,
    class...Ts
>
struct map_list<
    List<Ts...>,
    Transformer
> {
    using type = typename map_detail::map<
        List<>,
        List<Ts...>,
        Transformer
    >::type;
};

template <class List, template <class> class Transformer>
using map_list_t = typename map_list<List, Transformer>::type;

} // ::tom
