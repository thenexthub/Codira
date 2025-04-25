//===--- RequirementKind.h - Swift RequirementKind AST ---------*- C++ -*-===//
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
// This file defines the RequirementKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_REQUIREMENTKIND_H
#define SWIFT_AST_REQUIREMENTKIND_H

namespace language {
/// Describes the kind of a requirement that occurs within a requirements
/// clause.
enum class RequirementKind : unsigned {
  /// A conformance requirement T : P, where T is a type that depends
  /// on a generic parameter and P is a protocol to which T must conform.
  Conformance,
  /// A superclass requirement T : C, where T is a type that depends
  /// on a generic parameter and C is a concrete class type which T must
  /// equal or be a subclass of.
  Superclass,
  /// A same-type requirement T == U, where T and U are types that shall be
  /// equivalent.
  SameType,
  /// A layout bound T : L, where T is a type that depends on a generic
  /// parameter and L is some layout specification that should bound T.
  Layout,
  /// A same-shape requirement shape(T) == shape(U), where T and U are pack
  /// parameters.
  SameShape,

  // Note: there is code that packs this enum in a 3-bit bitfield.  Audit users
  // when adding enumerators.
  LAST_KIND=SameShape
};

} // namespace language
#endif
