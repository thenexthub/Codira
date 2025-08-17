//===-- toolchain/ADT/APSInt.cpp - Arbitrary Precision Signed Integer ---*- C++ -*--===//
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
//
// This file implements the APSInt class, which is a simple class that
// represents an arbitrary sized integer that knows its signedness.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_APSInt.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_FoldingSet.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>

using namespace toolchain;

APSInt::APSInt(StringRef Str) {
  assert(!Str.empty() && "Invalid string length");

  // (Over-)estimate the required number of bits.
  unsigned NumBits = ((Str.size() * 64) / 19) + 2;
  APInt Tmp(NumBits, Str, /*Radix=*/10);
  if (Str[0] == '-') {
    unsigned MinBits = Tmp.getMinSignedBits();
    if (MinBits > 0 && MinBits < NumBits)
      Tmp = Tmp.trunc(MinBits);
    *this = APSInt(Tmp, /*IsUnsigned=*/false);
    return;
  }
  unsigned ActiveBits = Tmp.getActiveBits();
  if (ActiveBits > 0 && ActiveBits < NumBits)
    Tmp = Tmp.trunc(ActiveBits);
  *this = APSInt(Tmp, /*IsUnsigned=*/true);
}

void APSInt::Profile(FoldingSetNodeID& ID) const {
  ID.AddInteger((unsigned) (IsUnsigned ? 1 : 0));
  APInt::Profile(ID);
}
