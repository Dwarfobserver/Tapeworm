
#pragma once

#include "static_visit.hpp"
#include "aggregate_to_tuple.hpp"

/*
Concepts interface :

    auto tuple_size  = sizeof...(Ts)

    using tuple_type = std::tuple<Ts...>

    as_tuple(t&)       -> std::tuple<Ts&...>
    as_tuple(t const&) -> std::tuple<Ts const&...>
    as_tuple(t&&)      -> std::tuple<Ts&&...>

    to_tuple(t const&) -> std::tuple<Ts...>
    to_tuple(t&&)      -> std::tuple<Ts...>
*/

namespace tom {

namespace concept {

// Implemented for types with the form Tuple<Ts...> with
// std::get<I>(tuple) and std::tuple_size_v<Tuple<Ts...>>.
template <class T, class SFINAE>
struct tuple {
    static constexpr auto is_implemented = false;
};

template <template <class...> class Tuple, class...Ts>
struct tuple<Tuple<Ts...>, std::void_t<
    std::get<sizeof...(Ts) - 1>(std::declval<Tuple<Ts...>>()),
    tuple_size_v<Tuple<Ts...>>
>> {
private:
    template <class T>
    struct seq {};
    template <size_t...Is>
    struct seq<std::index_sequence<Is...>> {

        static constexpr std::tuple<Ts&...> as_tuple(Tuple<Ts...>& tuple) noexcept {
            return { std::get<Is>(tuple)... };
        }
        static constexpr std::tuple<Ts const&...> as_tuple(Tuple<Ts...> const& tuple) noexcept {
            return { std::get<Is>(tuple)... };
        }
        static constexpr std::tuple<Ts&&...> as_tuple(Tuple<Ts...>&& tuple) noexcept {
            return { std::move(std::get<Is>(tuple))... };
        }
        
        static constexpr std::tuple<Ts...> to_tuple(Tuple<Ts...> const& tuple) /* Conditional */ {
            return { std::get<Is>(tuple)... };
        }
        static constexpr std::tuple<Ts...> to_tuple(Tuple<Ts...>&& tuple) noexcept /* Conditional */ {
            return { std::move(std::get<Is>(tuple))... };
        }
    };
    using seq_t = seq<std::make_index_sequence<sizeof...(Ts)>>;
public:
    static constexpr auto is_implemented = true;

    static constexpr auto tuple_size = sizeof...(Ts);

    using tuple_type = std::tuple<Ts...>;

    static constexpr std::tuple<Ts&...> as_tuple(Tuple<Ts...>& tuple) noexcept {
        return seq_t::as_tuple(tuple);
    }
    static constexpr std::tuple<Ts const&...> as_tuple(Tuple<Ts...> const& tuple) noexcept {
        return seq_t::as_tuple(tuple);
    }
    static constexpr std::tuple<Ts&&...> as_tuple(Tuple<Ts...>&& tuple) noexcept {
        return seq_t::as_tuple(std::move(tuple));
    }

    static constexpr std::tuple<Ts...> to_tuple(Tuple<Ts...> const& tuple) /* Conditional */ {
        return seq_t::to_tuple(tuple);
    }
    static constexpr std::tuple<Ts...> to_tuple(Tuple<Ts...>&& tuple) noexcept /* Conditional */ {
        return seq_t::to_tuple(std::move(tuple));
    }
};

template <class T, class = std::enable_if_t<
    is_aggregate_v<T>
>>
struct aggregate {
    template <class Tuple, class Seq>
    struct seq {};
    template <class...Ts, size_t...Is>
    struct seq<std::tuple<Ts&...>, std::index_sequence<Is...>> {

        static constexpr auto tuple_size = sizeof...(Ts);

        using tuple_type = std::tuple<Ts...>;

        static constexpr std::tuple<Ts&...> as_tuple(T& agg) noexcept {
            return tom::as_tuple(agg);
        }
        static constexpr std::tuple<Ts const&...> as_tuple(T const& agg) noexcept {
            return tom::as_tuple(agg);
        }
        static constexpr std::tuple<Ts&&...> as_tuple(T&& agg) noexcept {
            return tom::as_tuple(std::move(agg));
        }
        
        static constexpr std::tuple<Ts...> to_tuple(T const& agg) /* Conditional */ {
            const auto tuple = tom::as_tuple(agg);
            return { std::get<Is>(tuple)... };
        }
        static constexpr std::tuple<Ts...> to_tuple(T&& agg) noexcept /* Conditional */ {
            const auto tuple = tom::as_tuple(std::move(agg));
            return { std::move(std::get<Is>(tuple))... };
        }
    };
    using seq_t = seq<as_tuple_t<T>, std::make_index_sequence<sizeof...(Ts)>>;*
public:
    static constexpr auto tuple_size = seq_t::tuple_size;

    using tuple_type = seq_t::tuple_type;

    static constexpr auto as_tuple(T& agg) noexcept {
        return seq_t::as_tuple(agg);
    }
    static constexpr auto as_tuple(T const& agg) noexcept {
        return seq_t::as_tuple(agg);
    }
    static constexpr auto as_tuple(T&& agg) noexcept {
        return seq_t::as_tuple(std::move(agg));
    }

    static constexpr auto to_tuple(T const& agg) /* Conditional */ {
        return seq_t::to_tuple(agg);
    }
    static constexpr auto to_tuple(T&& agg) noexcept /* Conditional */ {
        return seq_t::to_tuple(std::move(agg));
    }
};

} // ::concept


} // ::tom
