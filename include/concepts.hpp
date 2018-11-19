
#pragma once

#include <priority_concept.hpp>

namespace tom {

namespace concept {

    // interface :
    // tuple interface (get<I> + size)
    // as_tuple, as_tuple_t
    // template <template <...> List> list_elements

    template <class T>
    struct static_visitable {

    };

    template <class T>
    struct static_array {

    };

    template <class T>
    struct tuple {

    };

    template <class T>
    struct aggregate {

    };

} // ::concept::tuple

using tuple_concepts = build_concept_list
    <concept::tuple::static_visitable, 4>::add
    <concept::tuple::static_array,     3>::add
    <concept::tuple::tuple,            2>::add
    <concept::tuple::aggregate,        1>;

namespace concept::range {

    // interface :
    // range interface + size + 

    template <class T>
    struct visitable {

    };

    template <class T>
    struct static_functor {

    };

    template <class T>
    struct array {

    };

    template <class T>
    struct range {

    };

    template <class T>
    struct optional {

    };

} // ::concept::range

using range_concepts = build_concept_list
    <concept::static_range::static_visitable, 6>::add
    <concept::range::visitable,               5>::add
    <concept::range::static_range,          4>::add
    <concept::range::array,                   3>::add
    <concept::range::range,                   2>::add
    <concept::range::optional,                1>;

namespace concept::generable {

// interface :
// visitable
// default-constructible
// move/copy assignable

template <class T>
struct static_functor {

};

template <class T>
struct functor {

};

} // ::concept::generable

} // ::tom

