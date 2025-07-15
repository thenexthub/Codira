//===--- LifetimeAnnotation.h - Lifetime-affecting attributes ---*- C++ -*-===//
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
// Defines a simple type-safe wrapper around the annotations that affect value
// lifetimes.  Used for both Decls and SILFunctionArguments.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_LIFETIMEANNOTATION_H
#define LANGUAGE_AST_LIFETIMEANNOTATION_H

#include <cstdint>

namespace language {

/// The annotation on a value (or type) explicitly indicating the lifetime that
/// it (or its instances) should have.
///
/// A LifetimeAnnotation is one of the following three values:
///
/// 1) None: No annotation has been applied.
/// 2) EagerMove: The @_eagerMove attribute has been applied.
/// 3) NoEagerMove: The @_noEagerMove attribute has been applied.
struct LifetimeAnnotation {
  enum Case : uint8_t {
    None,
    EagerMove,
    Lexical,
  } value;

  LifetimeAnnotation(Case newValue) : value(newValue) {}

  operator Case() const { return value; }

  bool isSome() { return value != None; }
};

} // namespace language

#endif
