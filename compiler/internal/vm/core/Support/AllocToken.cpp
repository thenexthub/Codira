//===- AllocToken.cpp - Allocation Token Calculation ----------------------===//
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
// Definition of AllocToken modes and shared calculation of stateless token IDs.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/AllocToken.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/SipHash.h"

using namespace vm::core;

std::optional<AllocTokenMode>
toolchain::getAllocTokenModeFromString(StringRef Name) {
  return StringSwitch<std::optional<AllocTokenMode>>(Name)
      .Case("increment", AllocTokenMode::Increment)
      .Case("random", AllocTokenMode::Random)
      .Case("typehash", AllocTokenMode::TypeHash)
      .Case("typehashpointersplit", AllocTokenMode::TypeHashPointerSplit)
      .Case("default", DefaultAllocTokenMode)
      .Default(std::nullopt);
}

StringRef toolchain::getAllocTokenModeAsString(AllocTokenMode Mode) {
  switch (Mode) {
  case AllocTokenMode::Increment:
    return "increment";
  case AllocTokenMode::Random:
    return "random";
  case AllocTokenMode::TypeHash:
    return "typehash";
  case AllocTokenMode::TypeHashPointerSplit:
    return "typehashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

static uint64_t getStableHash(const AllocTokenMetadata &Metadata,
                              uint64_t MaxTokens) {
  return getStableSipHash(Metadata.TypeName) % MaxTokens;
}

std::optional<uint64_t> toolchain::getAllocToken(AllocTokenMode Mode,
                                            const AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  assert(MaxTokens && "Must provide non-zero max tokens");

  switch (Mode) {
  case AllocTokenMode::Increment:
  case AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    return std::nullopt;

  case AllocTokenMode::TypeHash:
    return getStableHash(Metadata, MaxTokens);

  case AllocTokenMode::TypeHashPointerSplit: {
    if (MaxTokens == 1)
      return 0;
    const uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableHash(Metadata, HalfTokens);
    if (Metadata.ContainsPointer)
      Hash += HalfTokens;
    return Hash;
  }
  }

  llvm_unreachable("");
}
