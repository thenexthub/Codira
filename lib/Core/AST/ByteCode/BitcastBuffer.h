//===--------------------- BitcastBuffer.h ----------------------*- C++ -*-===//
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
#ifndef LANGUAGE_CORE_AST_INTERP_BITCAST_BUFFER_H
#define LANGUAGE_CORE_AST_INTERP_BITCAST_BUFFER_H

#include "toolchain/ADT/SmallVector.h"
#include <cassert>
#include <cstddef>
#include <memory>

namespace language::Core {
namespace interp {

enum class Endian { Little, Big };

struct Bytes;

/// A quantity in bits.
struct Bits {
  size_t N = 0;
  Bits() = default;
  static Bits zero() { return Bits(0); }
  explicit Bits(size_t Quantity) : N(Quantity) {}
  size_t getQuantity() const { return N; }
  size_t roundToBytes() const { return N / 8; }
  size_t getOffsetInByte() const { return N % 8; }
  bool isFullByte() const { return N % 8 == 0; }
  bool nonZero() const { return N != 0; }
  bool isZero() const { return N == 0; }
  Bytes toBytes() const;

  Bits operator-(Bits Other) const { return Bits(N - Other.N); }
  Bits operator+(Bits Other) const { return Bits(N + Other.N); }
  Bits operator+=(size_t O) {
    N += O;
    return *this;
  }
  Bits operator+=(Bits O) {
    N += O.N;
    return *this;
  }

  bool operator>=(Bits Other) const { return N >= Other.N; }
  bool operator<=(Bits Other) const { return N <= Other.N; }
  bool operator==(Bits Other) const { return N == Other.N; }
  bool operator!=(Bits Other) const { return N != Other.N; }
};

/// A quantity in bytes.
struct Bytes {
  size_t N;
  explicit Bytes(size_t Quantity) : N(Quantity) {}
  size_t getQuantity() const { return N; }
  Bits toBits() const { return Bits(N * 8); }
};

inline Bytes Bits::toBytes() const {
  assert(isFullByte());
  return Bytes(N / 8);
}

/// A bit range. Both Start and End are inclusive.
struct BitRange {
  Bits Start;
  Bits End;

  BitRange(Bits Start, Bits End) : Start(Start), End(End) {}
  Bits size() const { return End - Start + Bits(1); }
  bool operator<(BitRange Other) const { return Start.N < Other.Start.N; }

  bool contains(Bits B) { return Start <= B && End >= B; }
};

/// Track what bits have been initialized to known values and which ones
/// have indeterminate value.
struct BitcastBuffer {
  Bits FinalBitSize;
  std::unique_ptr<std::byte[]> Data;
  toolchain::SmallVector<BitRange> InitializedBits;

  BitcastBuffer(Bits FinalBitSize) : FinalBitSize(FinalBitSize) {
    assert(FinalBitSize.isFullByte());
    unsigned ByteSize = FinalBitSize.roundToBytes();
    Data = std::make_unique<std::byte[]>(ByteSize);
  }

  /// Returns the buffer size in bits.
  Bits size() const { return FinalBitSize; }
  Bytes byteSize() const { return FinalBitSize.toBytes(); }

  /// Returns \c true if all bits in the buffer have been initialized.
  bool allInitialized() const;
  /// Marks the bits in the given range as initialized.
  /// FIXME: Can we do this automatically in pushData()?
  void markInitialized(Bits Start, Bits Length);
  bool rangeInitialized(Bits Offset, Bits Length) const;

  /// Push \p BitWidth bits at \p BitOffset from \p In into the buffer.
  /// \p TargetEndianness is the endianness of the target we're compiling for.
  /// \p In must hold at least \p BitWidth many bits.
  void pushData(const std::byte *In, Bits BitOffset, Bits BitWidth,
                Endian TargetEndianness);

  /// Copy \p BitWidth bits at offset \p BitOffset from the buffer.
  /// \p TargetEndianness is the endianness of the target we're compiling for.
  ///
  /// The returned output holds exactly (\p FullBitWidth / 8) bytes.
  std::unique_ptr<std::byte[]> copyBits(Bits BitOffset, Bits BitWidth,
                                        Bits FullBitWidth,
                                        Endian TargetEndianness) const;
};

} // namespace interp
} // namespace language::Core
#endif
