
#pragma once

#include "meta.hpp"
#include <array>
#include <cstdint>
#include <memory>

/*
    Concepts : default constructible +
        - trivially_serializable
            - white-list of trivially copyable types
            - T[N] and std::array<T, N> of theses
        - trivial_array
            - data(), size(), resize(), [i] + trivially_serializable values
        - array
            - data(), size(), resize(), [i]
        - tuple_like
            - std::get + std::tuple_size
            - static_visitable
            - deconstructible aggregate
        - range
            - begin(), end(), push(v), remove_deep_const_t<value_type>
        - optional
            - operator*, operator->, put_value(v)
        - visitable
            - visit(visitor, type)
*/

namespace tom
{

template <class T>
constexpr bool is_trivially_serializable_v = false;

template <class T, size_t Size>
constexpr bool is_trivially_serializable_v<T [Size]> = is_trivially_serializable_v<T>;
template <class T, size_t Size>
constexpr bool is_trivially_serializable_v<std::array<T, Size>> = is_trivially_serializable_v<T>;

template <>
constexpr bool is_trivially_serializable_v<int32_t> = true;
// ...

template <class T>
constexpr bool has_optional_semantics_v = false;

template <class T>
constexpr bool has_optional_semantics_v<std::optional<T>> = true;
template <class T, class DeleterT>
constexpr bool has_optional_semantics_v<std::unique_ptr<T, DeleterT>> = true;

namespace serial::concept
{
    template <class T>
    struct forbidden_types {
        static constexpr bool is_implemented =
            std::is_empty_v<T>   ||
            std::is_union_v<T>   ||
            std::is_pointer_v<T> ||
            std::is_member_pointer_v<T>;

        static_assert(!is_implemented, "Raw pointers, empty types and unions are not allowed to be serialized.");
    };
    
    template <class T, std::void_t<decltype(
        std::size(std::declval<T const&>()),
        std::data(std::declval<T const&>()),
        // resize, default construct
    )>>
    struct trivial_array {
        using value_type = remove_cvref_t<decltype(*std::size(std::declval<T const&>()))>;
        static constexpr bool is_implemented = is_trivially_serializable_v<value_type>;

        // ...
    };

    template <class T, std::void_t<decltype(
        std::begin(std::declval<T const&>()),
        std::end  (std::declval<T const&>()),
        // add_element, default_construct
    )>>
    struct range {
        using value_type = remove_cvref_t<decltype(*std::begin(std::declval<T const&>()))>;

        // ...
    };

    template <class T, std::void_t<decltype(
                         *std::declval<T const&>(),
        static_cast<bool>(std::declval<T const&>()),
        // make_optional() -> default construct T&
    )>>
    struct optional {
        static constexpr bool is_implemented = has_optional_semantics_v<value_type>;

        // ...
    }
    
    template <class T>
    struct tuple {
        static constexpr bool is_implemented = false;
    }
    template <class...Ts>
    struct tuple<std::tuple<Ts...>> {
        static constexpr bool is_implemented = true;
        using value_type = remove_deep_const_t<std::tuple<Ts...>>;
    }


} // ::serial::tag

template <class T>
struct expression_tree {};

template <class T>
using expression_tree_t = typename expression_tree<T>::type;

namespace expression
{
    template <class T, class Concept>
    struct leaf {};
    template <class T, class Concept, class...Trees>
    struct node
    {
        static constexpr int size = sizeof...(Trees);
    };

    template <class Tree>
    struct traits {};
    
    template <class T, class Concept, class...Trees>
    struct traits<node<T, Concept, Trees...>>
    {
        static constexpr bool is_leaf = false;
        static constexpr bool is_empty = node<T, Concept, Trees...>::size == 0;

        static constexpr bool has_constant_size = (traits<Trees>::has_constant_size && ...);


    };

    template <class T, class Concept>
    struct traits<leaf<T, Concept>>
    {
        static constexpr bool is_leaf = true;
        static constexpr bool is_empty = false;

        static constexpr bool has_constant_size = Concept::has_constant_size;
    };

} // ::expression

} // ::tom
