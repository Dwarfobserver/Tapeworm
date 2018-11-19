
#include "meta.hpp"

// Member and free function detection expressions.
namespace tom_no_adl
{
    // Expressions are tested to look if a function 
    // is implemented is it's member or free form.
    #define TOM_MAKE_EXPRESSION(name) \
     \
    template <class T> \
    using member_##name##_t = decltype( \
        std::declval<T>().name() \
    ); \
    template <class T> \
    using free_##name##_t = decltype( \
        name(std::declval<T>()) \
    )

    TOM_MAKE_EXPRESSION(begin);
    TOM_MAKE_EXPRESSION(end);
    TOM_MAKE_EXPRESSION(data);
    TOM_MAKE_EXPRESSION(size);

    #undef TOM_MAKE_EXPRESSION
    
    // The next functions take an additional parameter.
    #define TOM_MAKE_EXPRESSION(name) \
     \
    template <class T, class SizeT> \
    using member_##name##_t = decltype( \
        std::declval<T>().name(std::declval<SizeT>()) \
    ); \
    template <class T, class SizeT> \
    using free_##name##_t = decltype( \
        name(std::declval<T>(), std::declval<SizeT>()) \
    )

    TOM_MAKE_EXPRESSION(resize);
    TOM_MAKE_EXPRESSION(reserve);

    #undef TOM_MAKE_EXPRESSION

} // ::tom_no_adl

namespace tom {

    // Use custom begin and end free functions to allow unified usage of
    // member f and free f, and to allow correct free f lookup in SFINAE.

    #define MAKE_CONDITIONAL_FUNCTION(name) \
     \
    template <class T, size_t Size> \
    constexpr auto name(T(&array)[Size]) noexcept { \
        return std::name(array); \
    } \
     \
    template <class T, class = std::enable_if_t< \
        is_detected_v<tom_no_adl::free_##name##_t, T&&> || \
        is_detected_v<tom_no_adl::member_##name##_t, T&&> \
    >> \
    constexpr auto name(T&& t) noexcept { \
        if constexpr (is_detected_v<tom_no_adl::free_##name##_t, T&&>) { \
            return name(std::forward<T>(t)); \
        } \
        else { \
            return std::forward<T>(t).name(); \
        } \
    } \
    struct force_semicolon

    MAKE_CONDITIONAL_FUNCTION(begin);
    MAKE_CONDITIONAL_FUNCTION(end);
    MAKE_CONDITIONAL_FUNCTION(data);
    MAKE_CONDITIONAL_FUNCTION(size);

    #undef MAKE_CONDITIONAL_FUNCTION

    template <class T>
    constexpr bool is_range_v = is_detected_v<tom_no_adl::free_begin_t, T>;

    // Allow free functions 'resize' and 'reserve'.
    // The reserve fuction is optional.
    
    template <class T, class SizeT>
    void resize(T&& t, SizeT size) {
        if constexpr (is_detected_v<tom_no_adl::resize_expression, T&&, SizeT>) {
            std::forward<T>(t).resize(size);
        }
        else {
            resize(std::forward<T>(t), size);
        }
    }
    
    template <class T, class SizeT>
    void try_reserve(T&& t, SizeT size) {
        if constexpr (is_detected_v<tom_no_adl::reserve_expression, T&&, SizeT>) {
            std::forward<T>(t).reserve(size);
        }
        else if constexpr (is_detected_v<tom_no_adl::reserve_free_expression, T&&, SizeT>) {
            reserve(std::forward<T>(t), size);
        }
    }

}
