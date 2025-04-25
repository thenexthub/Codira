//===------------------- FunctionBodySkipping.h -----------------*- C++ -*-===//
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

#ifndef SWIFT_BASIC_FUNCTIONBODYSKIPPING_H
#define SWIFT_BASIC_FUNCTIONBODYSKIPPING_H

#include "llvm/Support/DataTypes.h"

namespace language {

/// Describes the function bodies that can be skipped in type-checking.
enum class FunctionBodySkipping : uint8_t {
  /// Do not skip type-checking for any function bodies.
  None,
  /// Only non-inlinable function bodies should be skipped.
  NonInlinable,
  /// Only non-inlinable functions bodies without type definitions should
  /// be skipped.
  NonInlinableWithoutTypes,
  /// All function bodies should be skipped, where not otherwise required
  /// for type inference.
  All
};

} // end namespace language

#endif // SWIFT_BASIC_FUNCTIONBODYSKIPPING_H
