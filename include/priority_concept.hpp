
#pragma once

#include <meta.hpp>

namespace tom {

// Checks if a given concept is implemented by the given types.

namespace detail {
template <template <class...> class Concept, class SFINAE, class...Ts>
constexpr bool implement_concept = is_detected_v<Concept, Ts...>;

template <template <class...> class Concept, class...Ts>
constexpr bool implement_concept<Concept, std::enable_if_t<
    Concept<Ts...>::is_implemented
>, Ts...> = true;
} // ::detail

template <template <class...> class Concept, class...Ts>
constexpr bool implement_concept_v = detail::implement_concept<Concept, void, Ts...>;


template <class PrioConcept, class SFINAE = void>
struct accept_concept {
    static constexpr bool value = is_detected_v<PrioConcept::template concept, Ts...>;
};
template <class PrioConcept>
struct accept_concept<PrioConcept, std::enable_if_t<
    PrioConcept::template concept<Ts...>::is_implemented
>> {
    static constexpr bool value = true;
};


// Builds a list of prioritized concpets.

// Associates a concept with a priority (the higher the more prioritized).
template <template <class...> class Concept, int Priority>
struct priority_concept {
    static constexpr auto priority = Priority;

    template <class...Ts>
    using concept = Concept<Ts...>;
};

namespace detail {
    // The predicate used to sort the concept list.
    template <class PrioConcept1, class PrioConcept2>
    struct priority_comparer {
        static constexpr bool value = PrioConcept1::priority > PrioConcept2::priority;
    };
} // ::detail

// The list builder.
template <class...PrioConcepts>
struct priority_concept_list {
    template <
        template <class...> class Concept,
        int Priority
    >
    using add = sort_list_t<
        priority_concept_list<
            priority_concept<Concept, Priority>,
            PrioConcepts...
        >,
        detail::priority_comparer
    >;
};

template <
    template <class...> class Concept,
    int Priority
>
using build_concept_list = typename priority_concept_list<>::template add<Concept, Priority>;

// Gets the best concept from a prioritized concepts list for the given types.

namespace detail {

    // Indirection to catch compile-time error with a (hopefully) friendly message.
    template <class ConceptsList, class ChoosenPrioConcept, class...Ts>
    struct extract_concept {
        using type = typename ChoosenPrioConcept::template concept<Ts...>;
    };
    template <template <class...> class List, class...PrioConcepts, class...Ts>
    struct extract_concept<List<PrioConcepts...>, void, Ts...> {
        static_assert(always_false_v<PrioConcepts...>,
            "No one concept is implemented by the given types");
    };

    template <class ConceptsList, class...Ts>
    struct get_priority_concept {};
    template <template <class...> class List, class...PrioConcepts, class...Ts>
    struct get_priority_concept<List<PrioConcepts...>, Ts...> {

        template <class PrioConcept, class SFINAE = void>
        struct accept_concept {
            static constexpr bool value = is_detected_v<PrioConcept::template concept, Ts...>;
        };
        template <class PrioConcept>
        struct accept_concept<PrioConcept, std::enable_if_t<
            PrioConcept::template concept<Ts...>::is_implemented
        >> {
            static constexpr bool value = true;
        };
        using type = find_first_or_t<
            List<PrioConcepts...>,
            accept_concept
        >;
    };

} // ::detail

template <class PrioConceptsList, class...Ts>
using pick_concept_t = typename detail::extract_concept<
    PrioConceptsList,
    typename detail::get_priority_concept<PrioConceptsList, Ts...>::type,
    Ts...
>::type;

template <class PrioConceptsList, class...Ts>
constexpr bool has_concept_v = !std::is_same_v<
    detail::get_priority_concept<PrioConceptsList, Ts...>::type,
    void
>;

}
