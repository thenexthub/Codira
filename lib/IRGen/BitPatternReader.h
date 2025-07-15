//===--- BitPatternReader.h - Split bit patterns into chunks. -------------===//
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

#pragma once

#include "language/Basic/ClusteredBitVector.h"

#include "toolchain/ADT/APInt.h"
#include <optional>

namespace language {
namespace irgen {

/// BitPatternReader allows an APInt to be read from in chunks.
/// Chunks may be read starting from either the least significant bit
/// (little-endian) or the most significant bit (big-endian).
///
/// This is useful when interpreting an APInt as a multi-byte mask
/// that needs to be applied to a value with a composite type.
///
/// Example:
///
///   // big-endian
///   auto x = BitPatternReader(APInt(32, 0x1234), false);
///   x.read(16) // 0x12
///   x.read(8)  // 0x3
///   x.read(8)  // 0x4
///
///   // little-endian
///   auto y = BitPatternReader(APInt(32, 0x1234), true);
///   y.read(16) // 0x34
///   y.read(8)  // 0x2
///   y.read(8)  // 0x1
///
class BitPatternReader {
  using APInt = toolchain::APInt;

  const APInt Value;
  const bool LittleEndian;
  unsigned Offset = 0;
public:
  /// If the reader is in little-endian mode then bits will be read
  /// from the least significant to the most significant. Otherwise
  /// they will be read from the most significant to the least
  /// significant.
  BitPatternReader(const APInt &value, bool littleEndian) :
      Value(value),
      LittleEndian(littleEndian) {}

  /// Read the given number of bits from the unread part of the
  /// underlying value and adjust the remaining value as appropriate.
  APInt read(unsigned numBits) {
    assert(numBits % 8 == 0);
    assert(Value.getBitWidth() >= Offset + numBits);
    unsigned offset = Offset;
    if (!LittleEndian) {
      offset = Value.getBitWidth() - offset - numBits;
    }
    Offset += numBits;
    return Value.extractBits(numBits, offset);
  }

  // Skip the number of bits provided.
  void skip(unsigned numBits) {
    assert(numBits % 8 == 0);
    assert(Value.getBitWidth() >= Offset + numBits);
    Offset += numBits;
  }
};

} // end namespace irgen
} // end namespace language

