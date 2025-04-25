//===--- Numeric.cpp - Swift Language ABI numerics support ----------------===//
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
// Implementations of the numeric-support ABI functions.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Numeric.h"

using namespace language;

/// Convert an integer literal to the floating-point type T.
template <class T>
static T convert(IntegerLiteral value) {
  using SignedChunk = IntegerLiteral::SignedChunk;
  using UnsignedChunk = IntegerLiteral::UnsignedChunk;

  auto data = value.getData();
  assert(!data.empty() && "always require at least one chunk");

  // The single-word case is the easiest.
  if (data.size() == 1) {
    return T(SignedChunk(data[0]));
  }

  // In two's complement, only the topmost chunk is really signed;
  // everything else is added to that as an unsigned value.
  static_assert(IntegerLiteral::BitsPerChunk == 32 ||
                IntegerLiteral::BitsPerChunk == 64,
                "expected either 32-bit or 64-bit chunking");
  T chunkFactor = (IntegerLiteral::BitsPerChunk == 32 ? 0x1p32 : 0x1p64);

  T result = UnsignedChunk(data[0]);
  T scale = chunkFactor;
  for (size_t i = 1, e = data.size() - 1; i != e; ++i) {
    result += UnsignedChunk(data[i]) * scale;
    scale *= chunkFactor;
  }
  result += SignedChunk(data.back()) * scale;

  return result;
}

float swift::swift_intToFloat32(IntegerLiteral value) {
  return convert<float>(value);
}

double swift::swift_intToFloat64(IntegerLiteral value) {
  return convert<double>(value);
}
