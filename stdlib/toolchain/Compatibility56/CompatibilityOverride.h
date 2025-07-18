//===--- CompatibilityOverride.h - Back-deploying compatibility fixes --*- C++ -*-===//
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
//
// Support back-deploying compatibility fixes for newer apps running on older runtimes.
//
//===----------------------------------------------------------------------===//

#ifndef COMPATIBILITY_OVERRIDE_H
#define COMPATIBILITY_OVERRIDE_H

#include "public/runtime/Private.h"

#include "Runtime/Concurrency.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/Once.h"
#include <type_traits>

namespace language {

// Macro utilities.
#define COMPATIBILITY_UNPAREN(...) __VA_ARGS__
#define COMPATIBILITY_CONCAT2(x, y) x##y
#define COMPATIBILITY_CONCAT(x, y) COMPATIBILITY_CONCAT2(x, y)

// This ridiculous construct will remove the parentheses from the argument and
// add a trailing comma, or will produce nothing when passed no argument. For
// example:
// COMPATIBILITY_UNPAREN_WITH_COMMA((1, 2, 3)) -> 1, 2, 3,
// COMPATIBILITY_UNPAREN_WITH_COMMA((4)) -> 4,
// COMPATIBILITY_UNPAREN_WITH_COMMA() ->
#define COMPATIBILITY_UNPAREN_WITH_COMMA(x)                                    \
  COMPATIBILITY_CONCAT(COMPATIBILITY_UNPAREN_ADD_TRAILING_COMMA_,              \
                       COMPATIBILITY_UNPAREN_WITH_COMMA2 x)
#define COMPATIBILITY_UNPAREN_WITH_COMMA2(...) PARAMS(__VA_ARGS__)
#define COMPATIBILITY_UNPAREN_ADD_TRAILING_COMMA_PARAMS(...) __VA_ARGS__,
#define COMPATIBILITY_UNPAREN_ADD_TRAILING_COMMA_COMPATIBILITY_UNPAREN_WITH_COMMA2

// This ridiculous construct will preserve the parentheses around the argument,
// or will produce an empty pair of parentheses when passed no argument. For
// example:
// COMPATIBILITY_PAREN((1, 2, 3)) -> (1, 2, 3)
// COMPATIBILITY_PAREN((4)) -> (4)
// COMPATIBILITY_PAREN() -> ()
#define COMPATIBILITY_PAREN(x)                                                 \
  COMPATIBILITY_CONCAT(COMPATIBILITY_PAREN_, COMPATIBILITY_PAREN2 x)
#define COMPATIBILITY_PAREN2(...) PARAMS(__VA_ARGS__)
#define COMPATIBILITY_PAREN_PARAMS(...) (__VA_ARGS__)
#define COMPATIBILITY_PAREN_COMPATIBILITY_PAREN2 ()

// Include path computation. Code that includes this file can write
// `#include "CompatibilityOverrideIncludePath.h"` to include the appropriate
// .def file for the current library.
//
// DISCUSSION: We do not use COMPATIBILITY_OVERRIDE_INCLUDE_PATH directly since
// #including it can break syntax highlighting in certain editors. By using a
// different file, we keep the broken-ness to that one file that only #include
// COMPATIBILITY_OVERRIDE_INCLUDE_PATH.
#define COMPATIBILITY_OVERRIDE_INCLUDE_PATH_languageRuntime                       \
  "CompatibilityOverrideRuntime.def"
#define COMPATIBILITY_OVERRIDE_INCLUDE_PATH_language_Concurrency                  \
  "CompatibilityOverrideConcurrency.def"

#define COMPATIBILITY_OVERRIDE_INCLUDE_PATH                                    \
  COMPATIBILITY_CONCAT(COMPATIBILITY_OVERRIDE_INCLUDE_PATH_,                   \
                       LANGUAGE_TARGET_LIBRARY_NAME)

// Compatibility overrides are only supported on Darwin.
#ifndef LANGUAGE_RUNTIME_NO_COMPATIBILITY_OVERRIDES
#if !(defined(__APPLE__) && defined(__MACH__))
#define LANGUAGE_RUNTIME_NO_COMPATIBILITY_OVERRIDES
#endif
#endif

#ifdef LANGUAGE_RUNTIME_NO_COMPATIBILITY_OVERRIDES

# error Back-deployment library must always be built with compatibility overrides

#else // #ifdef LANGUAGE_RUNTIME_NO_COMPATIBILITY_OVERRIDES

// Override section name computation. `COMPATIBILITY_OVERRIDE_SECTION_NAME` will
// resolve to string literal containing the appropriate section name for the
// current library.
#define COMPATIBILITY_OVERRIDE_SECTION_NAME_language_Concurrency "__s_async_hook"

#define COMPATIBILITY_OVERRIDE_SECTION_NAME                                    \
  COMPATIBILITY_CONCAT(COMPATIBILITY_OVERRIDE_SECTION_NAME_,                   \
                       LANGUAGE_TARGET_LIBRARY_NAME)

// Create typedefs for function pointers to call the original implementation.
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  ccAttrs typedef ret(*Original_##name) COMPATIBILITY_PAREN(typedArgs);
#include "CompatibilityOverrideRuntime.def"
#include "CompatibilityOverrideConcurrency.def"
#undef OVERRIDE


// Create typedefs for override function pointers.
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  ccAttrs typedef ret (*Override_##name)(COMPATIBILITY_UNPAREN_WITH_COMMA(     \
      typedArgs) Original_##name originalImpl);
#include "CompatibilityOverrideRuntime.def"
#include "CompatibilityOverrideConcurrency.def"
#undef OVERRIDE

// Create declarations for getOverride functions.
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name getOverride_ ## name();
#include "CompatibilityOverrideRuntime.def"
#include "CompatibilityOverrideConcurrency.def"
#undef OVERRIDE

/// Used to define an override point. The override point #defines the appropriate
/// OVERRIDE macro from CompatibilityOverride.def to this macro, then includes
/// the file to generate the override points. The original implementation of the
/// functionality must be available as language_funcNameHereImpl.
#define COMPATIBILITY_OVERRIDE(name, ret, attrs, ccAttrs, namespace,           \
                               typedArgs, namedArgs)                           \
  attrs ccAttrs ret namespace language_##name COMPATIBILITY_PAREN(typedArgs) {    \
    static Override_##name Override;                                           \
    static language_once_t Predicate;                                             \
    language_once(                                                                \
        &Predicate, [](void *) { Override = getOverride_##name(); }, nullptr); \
    if (Override != nullptr)                                                   \
      return Override(COMPATIBILITY_UNPAREN_WITH_COMMA(namedArgs)              \
                          language_##name##Impl);                                 \
    return language_##name##Impl COMPATIBILITY_PAREN(namedArgs);                  \
  }

#endif // #else LANGUAGE_RUNTIME_NO_COMPATIBILITY_OVERRIDES

} /* end namespace language */

#endif /* COMPATIBILITY_OVERRIDE_H */
