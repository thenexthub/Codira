//===--- Pack.h - Helpers for wording with class template packs -*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_PACKS_H
#define LANGUAGE_BASIC_PACKS_H

namespace language {
namespace packs {

/// A pack of types.
template <class...>
struct Pack;

/// A trait indicating whether the given type is a specialization
/// of Pack<...>.
template <class>
struct IsPack {
  static constexpr bool value = false;
};

template <class... Components>
struct IsPack<Pack<Components...>> {
  static constexpr bool value = true;
};

/// Transform a variadic list of types using the given transform, a class
/// template which is expected to define a `result` type providing the
/// result of the transform.
///
///   template <class Arg, class Component>
///   class Transform {
///     using result = ...;
///   };
///
/// In principle, this would be cleaner as a member template, but that
/// works out poorly because in practice the member template must be
/// partially specialized in order to destructure it.
template <template <class, class> class Transform,
          class TransformArg, class... Values>
struct PackMapComponents {
  using result = Pack<typename Transform<TransformArg, Values>::result...>;
};

/// Transform the components of a pack using the given transform, a class
/// template which is expected to define a `result` type providing the
/// result of the transform.
///
///   template <class Arg, class Component>
///   class Transform {
///     using result = ...;
///   };
template <template <class, class> class Transform, class TransformArg,
          class P>
struct PackMap;

template <template <class, class> class Transform, class TransformArg,
          class... Components>
struct PackMap<Transform, TransformArg, Pack<Components...>>
  : PackMapComponents<Transform, TransformArg, Components...> {};

/// Concatenate the components of a variadic list of packs.
template <class... Packs>
struct PackConcat;

template <>
struct PackConcat<> {
  using result = Pack<>;
};

template <class First>
struct PackConcat<First> {
  static_assert(IsPack<First>::value, "argument is not a pack");
  using result = First;
};

template <class... FirstComponents, class... SecondComponents>
struct PackConcat<Pack<FirstComponents...>, Pack<SecondComponents...>> {
  using result = Pack<FirstComponents..., SecondComponents...>;
};

template <class Head, class... Tail>
struct PackConcat<Head, Tail...>
  : PackConcat<Head, typename PackConcat<Tail...>::result> {};

/// Flatten a pack of packs by concatenating their components.
template <class>
struct PackFlatten;

template <class... PackComponents>
struct PackFlatten<Pack<PackComponents...>>
  : PackConcat<PackComponents...> {};

/// Apply the given pack-producing transform to each component of a pack,
/// then concatenate the results.
///
/// For example, if we start with Pack<A, B, C>, and the transform turns:
///   A => Pack<X, Y>
///   B => Pack<>
///   C => Pack<Z>
/// then the result will be Pack<X, Y, Z>.
template <template <class, class> class Transform, class TransformArg, class P>
struct PackFlatMap
    : PackFlatten<typename PackMap<Transform, TransformArg, P>::result> {};

/// Reverse a variadic list of types.
template <class... PackComponents>
struct PackReverseComponents;

template <>
struct PackReverseComponents<> {
  using result = Pack<>;
};

template <class Head>
struct PackReverseComponents<Head> {
  using result = Pack<Head>;
};

template <class Head, class... Tail>
struct PackReverseComponents<Head, Tail...>
    : PackConcat<typename PackReverseComponents<Tail...>::result,
                 Pack<Head>> {};


/// Reverse a pack.
template <class>
struct PackReverse;

template <class... PackComponents>
struct PackReverse<Pack<PackComponents...>>
    : PackReverseComponents<PackComponents...> {};

/// Determine whether the given pack is transitively ordered according to
/// the given predicate:
///
///   template <class First, class Second>
///   struct IsOrdered {
///     static constexpr bool value;
///   };
template <template <class, class> class IsOrdered,
          class... Components>
struct PackComponentsAreOrdered;

template <template <class, class> class IsOrdered>
struct PackComponentsAreOrdered<IsOrdered> {
  static constexpr bool value = true;
};

template <template <class, class> class IsOrdered,
          class A>
struct PackComponentsAreOrdered<IsOrdered, A> {
  static constexpr bool value = true;
};

template <template <class, class> class IsOrdered,
          class A, class B>
struct PackComponentsAreOrdered<IsOrdered, A, B> : IsOrdered<A, B> {};

template <template <class, class> class IsOrdered,
          class A, class B, class... C>
struct PackComponentsAreOrdered<IsOrdered, A, B, C...> {
  static constexpr bool value =
    IsOrdered<A, B>::value &&
    PackComponentsAreOrdered<IsOrdered, B, C...>::value;
};

/// Determine whether the given pack is well-ordered according to the given
/// predicate:
///
///   template <class First, class Second>
///   struct IsOrdered {
///     static constexpr bool value;
///   };
template <template <class, class> class IsOrdered,
          class P>
struct PackIsOrdered;

template <template <class, class> class IsOrdered,
          class... Components>
struct PackIsOrdered<IsOrdered, Pack<Components...>>
    : PackComponentsAreOrdered<IsOrdered, Components...> {};

} // end namespace packs
} // end namespace language

#endif
