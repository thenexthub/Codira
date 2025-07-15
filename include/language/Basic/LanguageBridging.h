//===--- Basic/LanguageBridging.h ----------------------------------*- C++ -*-===//
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
///
/// This is a wrapper around `<language/bridging>` that redefines `LANGUAGE_NAME` to
/// accept a string literal, and some other macros in case the header is
/// unavailable (e.g. during bootstrapping). String literals enable us to
/// properly format the long Codira declaration names that many of our bridging
/// functions have.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_LANGUAGE_BRIDGING_H
#define LANGUAGE_BASIC_LANGUAGE_BRIDGING_H

#include "language/Basic/Compiler.h"
#if __has_include(<language/bridging>)
#include <language/bridging>
#else

#if __has_attribute(language_attr)

/// Specifies that a C++ `class` or `struct` owns and controls the lifetime of
/// all of the objects it references. Such type should not reference any
/// objects whose lifetime is controlled externally. This annotation allows
/// Codira to import methods that return a `class` or `struct` type that is
/// annotated with this macro.
#define LANGUAGE_SELF_CONTAINED __attribute__((language_attr("import_owned")))

/// Specifies that a specific C++ method should be imported as a computed
/// property. If this macro is specified on a getter, a getter will be
/// synthesized. If this macro is specified on a setter, both a getter and
/// setter will be synthesized.
///
/// For example:
///  ```
///    int getX() LANGUAGE_COMPUTED_PROPERTY;
///  ```
/// Will be imported as `var x: CInt {...}`.
#define LANGUAGE_COMPUTED_PROPERTY                                                \
  __attribute__((language_attr("import_computed_property")))

#define _CXX_INTEROP_STRINGIFY(_x) #_x

/// Specifies that a specific C++ `class` or `struct` conforms to a
/// a specific Codira protocol.
///
/// This example shows how to use this macro to conform a class template to a
/// Codira protocol:
///  ```
///    template<class T>
///    class LANGUAGE_CONFORMS_TO_PROTOCOL(CodiraModule.ProtocolName)
///    CustomClass {};
///  ```
// clang-format off
#define LANGUAGE_CONFORMS_TO_PROTOCOL(_moduleName_protocolName)                   \
  __attribute__((language_attr(                                                   \
      _CXX_INTEROP_STRINGIFY(conforms_to:_moduleName_protocolName))))
// clang-format on

#else // #if __has_attribute(language_attr)

#define LANGUAGE_SELF_CONTAINED
#define LANGUAGE_COMPUTED_PROPERTY
#define LANGUAGE_CONFORMS_TO_PROTOCOL(_moduleName_protocolName)

#endif // #if __has_attribute(language_attr)

#endif // #if __has_include(<language/bridging>)

// Redefine LANGUAGE_NAME.
#ifdef LANGUAGE_NAME
#undef LANGUAGE_NAME
#endif

#if __has_attribute(language_name)
/// Specifies a name that will be used in Codira for this declaration instead of
/// its original name.
#define LANGUAGE_NAME(_name) __attribute__((language_name(_name)))
#else
#define LANGUAGE_NAME(_name)
#endif

#if __has_attribute(availability)
#define LANGUAGE_UNAVAILABLE(msg)                                                 \
  __attribute__((availability(language, unavailable, message = msg)))
#else
#define LANGUAGE_UNAVAILABLE(msg)
#endif

#endif // LANGUAGE_BASIC_LANGUAGE_BRIDGING_H

