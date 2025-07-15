//===--- StableHasher.cpp - Stable Hasher ---------------------------------===//
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

#include "language/Basic/StableHasher.h"

using namespace language;

#define ROTATE_LEFT(ROTAND, DISTANCE)                                          \
  (uint64_t)((ROTAND) << (DISTANCE)) | ((ROTAND) >> (64 - (DISTANCE)))

namespace {
static inline void sip_round(uint64_t &v0, uint64_t &v1, uint64_t &v2,
                             uint64_t &v3) {
  v0 += v1;
  v1 = ROTATE_LEFT(v1, 13);
  v1 ^= v0;
  v0 = ROTATE_LEFT(v0, 32);
  v2 += v3;
  v3 = ROTATE_LEFT(v3, 16);
  v3 ^= v2;
  v0 += v3;
  v3 = ROTATE_LEFT(v3, 21);
  v3 ^= v0;
  v2 += v1;
  v1 = ROTATE_LEFT(v1, 17);
  v1 ^= v2;
  v2 = ROTATE_LEFT(v2, 32);
}
} // end anonymous namespace

void StableHasher::compress(uint64_t value) {
  state.v3 ^= value;
  for (unsigned i = 0; i < 2; ++i) {
    ::sip_round(state.v0, state.v1, state.v2, state.v3);
  }
  state.v0 ^= value;
}

std::pair<uint64_t, uint64_t> StableHasher::finalize() && {
  auto fillBitsFromBuffer = [](uint64_t fill, uint8_t *bits) {
    uint64_t head = 0;
    switch (fill) {
    case 7:
      head |= uint64_t(bits[6]) << 48;
      TOOLCHAIN_FALLTHROUGH;
    case 6:
      head |= uint64_t(bits[5]) << 40;
      TOOLCHAIN_FALLTHROUGH;
    case 5:
      head |= uint64_t(bits[4]) << 32;
      TOOLCHAIN_FALLTHROUGH;
    case 4:
      head |= uint64_t(bits[3]) << 24;
      TOOLCHAIN_FALLTHROUGH;
    case 3:
      head |= uint64_t(bits[2]) << 16;
      TOOLCHAIN_FALLTHROUGH;
    case 2:
      head |= uint64_t(bits[1]) << 8;
      TOOLCHAIN_FALLTHROUGH;
    case 1:
      head |= uint64_t(bits[0]);
      break;
    case 0:
      break;
    default:
      break;
    }
    return head;
  };

  const uint64_t b = fillBitsFromBuffer(getBufferLength(), byteBuffer);
  compress(((getDigestLength() & 0xFF) << 56) | b);

  state.v2 ^= 0xEE;

  for (unsigned i = 0; i < 4; ++i) {
    ::sip_round(state.v0, state.v1, state.v2, state.v3);
  }

  const uint64_t h1 = state.v0 ^ state.v1 ^ state.v2 ^ state.v3;

  state.v1 ^= 0xDD;

  for (unsigned i = 0; i < 4; ++i) {
    ::sip_round(state.v0, state.v1, state.v2, state.v3);
  }

  const uint64_t h2 = state.v0 ^ state.v1 ^ state.v2 ^ state.v3;

  return std::make_pair(h1, h2);
}

