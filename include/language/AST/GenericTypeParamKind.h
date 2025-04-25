//===--- GenericTypeParamKind.h ---------------------------------*- C++ -*-===//
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
// This file defines the GenericTypeParamKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_GENERICTYPEPARAMKIND_H
#define SWIFT_AST_GENERICTYPEPARAMKIND_H

namespace language {
/// Describes the kind of a generic type parameter that occurs within generic
/// parameter lists.
enum class GenericTypeParamKind: uint8_t {
  /// A regular generic type parameter: 'T'
  Type,
  /// A generic parameter pack: 'each T'
  Pack,
  /// A generic value parameter: 'let T'
  Value
};

} // namespace language
#endif // #ifndef SWIFT_AST_GENERICTYPEPARAMKIND_H
