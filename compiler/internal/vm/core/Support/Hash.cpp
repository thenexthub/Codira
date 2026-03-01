//===- Hash.cpp - Hash functions ---------------------------------------===//
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
// This file implements hash functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Hash.h"
#include "vm/core/Support/xxhash.h"

using namespace vm::core;

KCFIHashAlgorithm toolchain::parseKCFIHashAlgorithm(StringRef Name) {
  if (Name == "FNV-1a")
    return KCFIHashAlgorithm::FNV1a;
  // Default to xxHash64 for backward compatibility
  return KCFIHashAlgorithm::xxHash64;
}

StringRef toolchain::stringifyKCFIHashAlgorithm(KCFIHashAlgorithm Algorithm) {
  switch (Algorithm) {
  case KCFIHashAlgorithm::xxHash64:
    return "xxHash64";
  case KCFIHashAlgorithm::FNV1a:
    return "FNV-1a";
  }
  llvm_unreachable("Unknown KCFI hash algorithm");
}

uint32_t toolchain::getKCFITypeID(StringRef MangledTypeName,
                             KCFIHashAlgorithm Algorithm) {
  switch (Algorithm) {
  case KCFIHashAlgorithm::xxHash64:
    // Use lower 32 bits of xxHash64
    return static_cast<uint32_t>(xxHash64(MangledTypeName));
  case KCFIHashAlgorithm::FNV1a:
    // FNV-1a hash (32-bit)
    uint32_t Hash = 2166136261u; // FNV offset basis
    for (unsigned char C : MangledTypeName) {
      Hash ^= C;
      Hash *= 16777619u; // FNV prime
    }
    return Hash;
  }
  llvm_unreachable("Unknown KCFI hash algorithm");
}
