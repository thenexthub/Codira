//===--- SipHash.cpp - An ABI-stable string hash --------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
//  This file implements an ABI-stable string hash based on SipHash, used to
//  compute ptrauth discriminators.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/SipHash.h"
#include "siphash/SipHash.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/Endian.h"
#include <cstdint>

using namespace vm::core;
using namespace support;

#define DEBUG_TYPE "toolchain-siphash"

void toolchain::getSipHash_2_4_64(ArrayRef<uint8_t> In, const uint8_t (&K)[16],
                             uint8_t (&Out)[8]) {
  siphash<2, 4>(In.data(), In.size(), K, Out);
}

void toolchain::getSipHash_2_4_128(ArrayRef<uint8_t> In, const uint8_t (&K)[16],
                              uint8_t (&Out)[16]) {
  siphash<2, 4>(In.data(), In.size(), K, Out);
}

/// Compute an ABI-stable 64-bit hash of the given string.
uint64_t toolchain::getStableSipHash(StringRef Str) {
  static const uint8_t K[16] = {0xb5, 0xd4, 0xc9, 0xeb, 0x79, 0x10, 0x4a, 0x79,
                                0x6f, 0xec, 0x8b, 0x1b, 0x42, 0x87, 0x81, 0xd4};

  uint8_t RawHashBytes[8];
  getSipHash_2_4_64(arrayRefFromStringRef(Str), K, RawHashBytes);
  return endian::read64le(RawHashBytes);
}

/// Compute an ABI-stable 16-bit hash of the given string.
uint16_t toolchain::getPointerAuthStableSipHash(StringRef Str) {
  uint64_t RawHash = getStableSipHash(Str);

  // Produce a non-zero 16-bit discriminator.
  uint16_t Discriminator = (RawHash % 0xFFFF) + 1;
  LLVM_DEBUG(
      dbgs() << "ptrauth stable hash discriminator: " << utostr(Discriminator)
             << " (0x"
             << utohexstr(Discriminator, /*Lowercase=*/false, /*Width=*/4)
             << ")"
             << " of: " << Str << "\n");
  return Discriminator;
}
