//===-- language/Compability/Common/idioms.h ---------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_COMMON_IDIOMS_H_
#define LANGUAGE_COMPABILITY_COMMON_IDIOMS_H_

// Defines anything that might ever be useful in more than one source file
// or that is too weird or too specific to the host C++ compiler to be
// exposed elsewhere.

#ifndef __cplusplus
#error this is a C++ program
#endif
#if __cplusplus < 201703L
#error this is a C++17 program
#endif
#if !__clang__ && defined __GNUC__ && __GNUC__ < 7
#error g++ >= 7.2 is required
#endif

#include "enum-class.h"
#include "variant.h"
#include "visit.h"
#include <array>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

#if __GNUC__ == 7
// Avoid a deduction bug in GNU 7.x headers by forcing the answer.
namespace std {
template <typename A>
struct is_trivially_copy_constructible<list<A>> : false_type {};
template <typename A>
struct is_trivially_copy_constructible<optional<list<A>>> : false_type {};
} // namespace std
#endif

// enable "this is a std::string"s with the 's' suffix
using namespace std::literals::string_literals;

namespace language::Compability::common {

// Helper templates for combining a list of lambdas into an anonymous
// struct for use with common::visit() on a std::variant<> sum type.
// E.g.: common::visit(visitors{
//         [&](const firstType &x) { ... },
//         [&](const secondType &x) { ... },
//         ...
//         [&](const auto &catchAll) { ... }}, variantObject);

template <typename... LAMBDAS> struct visitors : LAMBDAS... {
  using LAMBDAS::operator()...;
};

template <typename... LAMBDAS> visitors(LAMBDAS... x) -> visitors<LAMBDAS...>;

// Calls std::fprintf(stderr, ...), then abort().
[[noreturn]] void die(const char *, ...);

#define DIE(x) language::Compability::common::die(x " at " __FILE__ "(%d)", __LINE__)

// For switch statement default: labels.
#define CRASH_NO_CASE DIE("no case")

// clang-format off
// For switch statements whose cases have return statements for
// all possibilities.  Clang emits warnings if the default: is
// present, gcc emits warnings if it is absent.
#if __clang__
#define SWITCH_COVERS_ALL_CASES
#else
#define SWITCH_COVERS_ALL_CASES default: CRASH_NO_CASE;
#endif
// clang-format on

// For cheap assertions that should be applied in production.
// To disable, compile with '-DCHECK=(void)'
#ifndef CHECK
#define CHECK(x) ((x) || (DIE("CHECK(" #x ") failed"), false))
#endif

// Same as above, but with a custom error message.
#ifndef CHECK_MSG
#define CHECK_MSG(x, y) ((x) || (DIE("CHECK(" #x ") failed: " #y), false))
#endif

// User-defined type traits that default to false:
// Invoke CLASS_TRAIT(traitName) to define a trait, then put
//   using traitName = std::true_type;  (or false_type)
// into the appropriate class definitions.  You can then use
//   typename std::enable_if_t<traitName<...>, ...>
// in template specialization definitions.
#define CLASS_TRAIT(T) \
  namespace class_trait_ns_##T { \
    template <typename A> std::true_type test(typename A::T *); \
    template <typename A> std::false_type test(...); \
    template <typename A> \
    constexpr bool has_trait{decltype(test<A>(nullptr))::value}; \
    template <typename A> constexpr bool trait_value() { \
      if constexpr (has_trait<A>) { \
        using U = typename A::T; \
        return U::value; \
      } else { \
        return false; \
      } \
    } \
  } \
  template <typename A> constexpr bool T{class_trait_ns_##T::trait_value<A>()};

// Check that a pointer is non-null and dereference it
#define DEREF(p) language::Compability::common::Deref(p, __FILE__, __LINE__)

template <typename T> constexpr T &Deref(T *p, const char *file, int line) {
  if (!p) {
    language::Compability::common::die("nullptr dereference at %s(%d)", file, line);
  }
  return *p;
}

template <typename T>
constexpr T &Deref(const std::unique_ptr<T> &p, const char *file, int line) {
  if (!p) {
    language::Compability::common::die("nullptr dereference at %s(%d)", file, line);
  }
  return *p;
}

// Given a const reference to a value, return a copy of the value.
template <typename A> A Clone(const A &x) { return x; }

// C++ does a weird and dangerous thing when deducing template type parameters
// from function arguments: lvalue references are allowed to match rvalue
// reference arguments.  Template function declarations like
//   template<typename A> int foo(A &&);
// need to be protected against this C++ language feature when functions
// may modify such arguments.  Use these type functions to invoke SFINAE
// on a result type via
//   template<typename A> common::IfNoLvalue<int, A> foo(A &&);
// or, for constructors,
//   template<typename A, typename = common::NoLvalue<A>> int foo(A &&);
// This works with parameter packs too.
template <typename A, typename... B>
using IfNoLvalue = std::enable_if_t<(... && !std::is_lvalue_reference_v<B>), A>;
template <typename... RVREF> using NoLvalue = IfNoLvalue<void, RVREF...>;
} // namespace language::Compability::common
#endif // FORTRAN_COMMON_IDIOMS_H_
