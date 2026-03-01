//===- DynamicAPInt.cpp - DynamicAPInt Implementation -----------*- C++ -*-===//
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
#include "vm/core/ADT/DynamicAPInt.h"
#include "vm/core/ADT/Hashing.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

hash_code toolchain::hash_value(const DynamicAPInt &X) {
  if (X.isSmall())
    return toolchain::hash_value(X.getSmall());
  return detail::hash_value(X.getLarge());
}

void DynamicAPInt::static_assert_layout() {
  constexpr size_t ValLargeOffset =
      offsetof(DynamicAPInt, ValLarge.Val.BitWidth);
  constexpr size_t ValSmallOffset = offsetof(DynamicAPInt, ValSmall);
  constexpr size_t ValSmallSize = sizeof(ValSmall);
  static_assert(ValLargeOffset >= ValSmallOffset + ValSmallSize);
}

raw_ostream &DynamicAPInt::print(raw_ostream &OS) const {
  if (isSmall())
    return OS << ValSmall;
  return OS << ValLarge;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void DynamicAPInt::dump() const { print(dbgs()); }
#endif
