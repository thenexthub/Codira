//===--- Numeric.h - Swift Language ABI numerics support --------*- C++ -*-===//
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
// Swift runtime support for numeric operations.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_NUMERIC_H
#define SWIFT_RUNTIME_NUMERIC_H

#include "language/ABI/MetadataValues.h"
#include "language/Runtime/Config.h"
#include "language/Basic/LLVM.h"
#include "llvm/ADT/ArrayRef.h"

namespace language {

/// A constant integer literal.  The format is designed to optimize the
/// checked-truncation operation typically performed by conformances to the
/// ExpressibleByBuiltinIntegerLiteral protocol.
class IntegerLiteral {
public:
  using SignedChunk = intptr_t;
  using UnsignedChunk = uintptr_t;
  enum : size_t { BitsPerChunk = sizeof(SignedChunk) * 8 };

private:
  const UnsignedChunk *Data;
  IntegerLiteralFlags Flags;

public:
  constexpr IntegerLiteral(const UnsignedChunk *data, IntegerLiteralFlags flags)
    : Data(data), Flags(flags) {}

  /// Return the chunks of data making up this value, arranged starting from
  /// the least-significant chunk.  The value is sign-extended to fill the
  /// final chunk.
  llvm::ArrayRef<UnsignedChunk> getData() const {
    return llvm::ArrayRef(Data, (Flags.getBitWidth() + BitsPerChunk - 1) /
                                    BitsPerChunk);
  }

  /// The flags for this value.
  IntegerLiteralFlags getFlags() const { return Flags; }

  /// Whether this value is negative.
  bool isNegative() const { return Flags.isNegative(); }

  /// The minimum number of bits necessary to store this value.
  /// Because this always includes the sign bit, it is never zero.
  size_t getBitWidth() const { return Flags.getBitWidth(); }
};

SWIFT_RUNTIME_EXPORT SWIFT_CC(swift) 
float swift_intToFloat32(IntegerLiteral value);

SWIFT_RUNTIME_EXPORT SWIFT_CC(swift)
double swift_intToFloat64(IntegerLiteral value);

// TODO: Float16 instead of just truncating from float?
// TODO: Float80 on x86?
// TODO: Float128 on targets that provide it?

}

#endif
