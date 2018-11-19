
#pragma once

#include <tuple>

namespace tom {

    // Requirement :
    // This constexpr function visiting all members.
    // Be careful to return the visitor.
    // The visit can't be conditional (unless the branch happens at compile-time).
    // 
    // [constexpr] auto static_visit(tom::static_visitor v, MyClass& val) [noexcept] {
    //     return v & val.x & val.z & val.z;
    // }
    //
    // Interface :
    // tuple_size
    // tuple_type
    // as_tuple(t&)
    // as_tuple(t const&)
    // as_tuple(t&&)
    // to_tuple(t const&)
    // to_tuple(t&&)
    namespace concept {
        template <class T, class SFINAE>
        struct static_visitable;
    }

    namespace detail {
        template <class...Ts>
        struct static_visitor_expression : std::tuple<Ts&...> {
            template <class, class>
            friend class concept::static_visitable;
        public:
            static constexpr auto is_visitor_expression = true;

            template <class T>
            constexpr static_visitor_expression<Ts..., T> operator&(T& val) const noexcept {
                return { std::tuple_cat(my_tuple(), std::tuple<T&>{val}) };
            }
        private:
            constexpr std::tuple<Ts&...> const& my_tuple() const noexcept { return *this; }

            static constexpr auto tuple_size = sizeof...(Ts);

            template <size_t I>
            constexpr auto& get() const noexcept {
                return std::get<I>(my_tuple());
            }
            
            template <size_t...Is>
            constexpr std::tuple<Ts&...> lref_tuple(std::index_sequence<Is...>) const noexcept {
                return { get<Is>()... };
            }
            template <size_t...Is>
            constexpr std::tuple<Ts&&...> rref_tuple(std::index_sequence<Is...>) const noexcept {
                return { std::move(get<Is>())... };
            }
            template <size_t...Is>
            constexpr std::tuple<Ts const&...> cref_tuple(std::index_sequence<Is...>) const noexcept {
                return { get<Is>()... };
            }
            template <size_t...Is>
            constexpr std::tuple<Ts...> copy_tuple(std::index_sequence<Is...>) const noexcept {
                return { get<Is>()... };
            }
            template <size_t...Is>
            constexpr std::tuple<Ts...> move_tuple(std::index_sequence<Is...>) const noexcept {
                return { std::move(get<Is>())... };
            }
        };
    } // ::detail

    struct static_visitor {
        template <class T>
        constexpr detail::static_visitor_expression<T> operator&(T& val) {
            return { val };
        }
    };

    namespace concept {

        template <class T, class = std::enable_if_t<
            decltype(static_visit(static_visitor{}, std::declval<T&>()))::is_visitor_expression
        >>
        struct static_visitable {
        private:
            static constexpr auto make_expression(T& value) noexcept {
                return static_visit(static_visitor{}, value);
            }
            static constexpr auto make_expression(T const& value) noexcept {
                return static_visit(static_visitor{}, const_cast<T&>(value));
            }
            using expression_t = decltype(make_expression(std::declval<T>()));

            using seq_type = std::make_index_sequence<expression_t::tuple_size>;
        public:
            static constexpr auto as_tuple(T& value) noexcept {
                return make_expression(value).lref_tuple(seq_type{});
            }
            static constexpr auto as_tuple(T&& value) noexcept {
                return make_expression(value).rref_tuple(seq_type{});
            }
            static constexpr auto as_tuple(T const& value) noexcept {
                return make_expression(value).cref_tuple(seq_type{});
            }

            static constexpr auto to_tuple(T const& value) /* TODO Conditional */ {
                return make_expression(value).copy_tuple(seq_type{});
            }
            static constexpr auto to_tuple(T&& value) noexcept /* TODO Conditional */ {
                return make_expression(value).move_tuple(seq_type{});
            }

            static constexpr auto tuple_size = expression_t::tuple_size;

            using tuple_type = decltype(to_tuple(std::declval<T>()));
        };

    } // ::concept

} // ::tom
