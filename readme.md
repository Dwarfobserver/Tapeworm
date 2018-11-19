
# Tapeworm

### Prioritized concepts applied binary serialization

Fun personal project in construction (not functional yet).

Write template claass concepts implementing the same interface.
Constraint their type by SFINAE or pattern matching.
Optionally, can set 'is_correctly_implemented' static boolean in concept.

List concepts with associed priorities.

'pick_concept<ConceptsList, Ts...>' will alias the best concept for the given types.

Binary serialization is implemented with these concepts :
 1) T has 'serialized_size(t)', 'serialize(buf, t)' and 'deserialize(buf, t)'.
 2) T has 'visit(t, f)'.
 3) T is trivially copyable and has no pointer nor reference.
 4) T has size(), data() and it's elements are trivially copyable.
 5) T has begin() and end(), or size() and data().
 6) T has std::get<I>() and std::tuple_size().
 7) T is a deconstructible aggregate.

io spans write to a non-owned buffer. Several error policies are available :
 - Unsafe : No size checks are performed.
 - Throwing : An exception is throw when the space check fails.
 - Error : A boolean is set when the space check fails.
 - Monadic : Same as error, but the flag is checked before serialization to do nothing.

Additional requirements are set to serialize containers.
They can be fulfilled through other prioritized concepts.
