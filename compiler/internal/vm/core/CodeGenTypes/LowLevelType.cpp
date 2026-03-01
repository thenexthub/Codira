//===-- toolchain/CodeGenTypes/LowLevelType.cpp
//---------------------------------===//
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
/// \file This file implements the more header-heavy bits of the LLT class to
/// avoid polluting users' namespaces.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGenTypes/LowLevelType.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

LLT::LLT(MVT VT) {
  if (VT.isVector()) {
    bool asVector = VT.getVectorMinNumElements() > 1 || VT.isScalableVector();
    init(/*IsPointer=*/false, asVector, /*IsScalar=*/!asVector,
         VT.getVectorElementCount(), VT.getVectorElementType().getSizeInBits(),
         /*AddressSpace=*/0);
  } else if (VT.isValid() && !VT.isScalableTargetExtVT()) {
    // Aggregates are no different from real scalars as far as GlobalISel is
    // concerned.
    init(/*IsPointer=*/false, /*IsVector=*/false, /*IsScalar=*/true,
         ElementCount::getFixed(0), VT.getSizeInBits(), /*AddressSpace=*/0);
  } else {
    IsScalar = false;
    IsPointer = false;
    IsVector = false;
    RawData = 0;
  }
}

void LLT::print(raw_ostream &OS) const {
  if (isVector()) {
    OS << "<";
    OS << getElementCount() << " x " << getElementType() << ">";
  } else if (isPointer())
    OS << "p" << getAddressSpace();
  else if (isValid()) {
    assert(isScalar() && "unexpected type");
    OS << "s" << getScalarSizeInBits();
  } else
    OS << "LLT_invalid";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void LLT::dump() const {
  print(dbgs());
  dbgs() << '\n';
}
#endif
