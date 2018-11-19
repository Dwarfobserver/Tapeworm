
#include "catch.hpp"

#include <priority_concept.hpp>

namespace {
    template <class...>
    struct type_list {};

    struct Mini   { enum { tag = -2 }; };
    struct Little { enum { tag = -1  }; };
    struct Medium { enum { tag = 0 }; };
    struct Huge   { enum { tag = 1 }; };
    struct Maxi   { enum { tag = 2 }; };
    
    template <class T1, class T2>
    struct tag_comparer {
        constexpr static bool value = T1::tag < T2::tag;
    };
    
    template <class T>
    struct is_medium {
        constexpr static bool value = T::tag == Medium::tag;
    };

    template <auto...>
    struct tag_ {};

    template <class T>
    struct struct_to_tag {
        using type = tag_<T::tag>;
    };
}

TEST_CASE("Compile-time list manipulation") {
    {
        using list   = type_list<>;
        using find   = tom::find_first_or_t<list, is_medium>;
        using map    = tom::map_list_t<list, struct_to_tag>;
        using sorted = tom::sort_list_t<list, tag_comparer>;
        CHECK(std::is_same_v<find,   void>);
        CHECK(std::is_same_v<map,    type_list<>>);
        CHECK(std::is_same_v<sorted, type_list<>>);
    }
    {
        using list   = type_list<Huge, Maxi, Little>;
        using find   = tom::find_first_or_t<list, is_medium, int>;
        using map    = tom::map_list_t<list, struct_to_tag>;
        using sorted = tom::sort_list_t<list, tag_comparer>;
        CHECK(std::is_same_v<find, int>);
        CHECK(std::is_same_v<map,    type_list<tag_<1>, tag_<2>, tag_<-1>>>);
        CHECK(std::is_same_v<sorted, type_list<Little,  Huge,    Maxi>>);
    }
    {
        using list   = type_list<Medium, Little, Maxi, Mini, Huge>;
        using find   = tom::find_first_or_t<list, is_medium>;
        using map    = tom::map_list_t<list, struct_to_tag>;
        using sorted = tom::sort_list_t<list, tag_comparer>;
        CHECK(std::is_same_v<find, Medium>);
        CHECK(std::is_same_v<map,    type_list<tag_<0>, tag_<-1>, tag_<2>, tag_<-2>, tag_<1>>>);
        CHECK(std::is_same_v<sorted, type_list<Mini,    Little,   Medium,  Huge,     Maxi>>);
    }
}

namespace {
    template <class T, class = std::enable_if_t<(
        T::tag > 0
    )>>
    struct abs_concept_positive {
        static constexpr int absolute_tag() { return T::tag; }
    };

    template <class T, class = std::enable_if_t<(
        T::tag < 0
    )>>
    struct abs_concept_negative {
        static constexpr int absolute_tag() { return - T::tag; }
    };

    using absolute_tag_concepts = typename tom::build_concept_list
        <abs_concept_negative, 0>::add
        <abs_concept_positive, 10>;
}

TEST_CASE("Concept selection") {
    using pos_pc = priority_concept<abs_concept_positive, 10>;
    using neg_pc = priority_concept<abs_concept_negative, 0>;
    CHECK(std::is_same_v<
        absolute_tag_concepts,
        priority_concept_list<pos_pc, neg_pc>
    >);

    CHECK( has_concept_v<absolute_tag_concepts, Maxi>);
    CHECK( has_concept_v<absolute_tag_concepts, Mini>);
    CHECK(!has_concept_v<absolute_tag_concepts, Medium>);

    using mini_concept = tom::pick_concept_t<absolute_tag_concepts, Mini>;
    CHECK(mini_concept::absolute_tag() == 2);
    
    using maxi_concept = tom::pick_concept_t<absolute_tag_concepts, Maxi>;
    CHECK(maxi_concept::absolute_tag() == 2);
}
