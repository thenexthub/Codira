//===--- PatternInit.cpp - Pattern Initialization -------------------------===//
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

#include "PatternInit.h"
#include "CodeGenModule.h"
#include "language/Core/Basic/TargetInfo.h"
#include "toolchain/IR/Constant.h"
#include "toolchain/IR/Type.h"

toolchain::Constant *language::Core::CodeGen::initializationPatternFor(CodeGenModule &CGM,
                                                         toolchain::Type *Ty) {
  // The following value is a guaranteed unmappable pointer value and has a
  // repeated byte-pattern which makes it easier to synthesize. We use it for
  // pointers as well as integers so that aggregates are likely to be
  // initialized with this repeated value.
  // For 32-bit platforms it's a bit trickier because, across systems, only the
  // zero page can reasonably be expected to be unmapped. We use max 0xFFFFFFFF
  // assuming that memory access will overlap into zero page.
  const uint64_t IntValue =
      CGM.getContext().getTargetInfo().getMaxPointerWidth() < 64
          ? 0xFFFFFFFFFFFFFFFFull
          : 0xAAAAAAAAAAAAAAAAull;
  // Floating-point values are initialized as NaNs because they propagate. Using
  // a repeated byte pattern means that it will be easier to initialize
  // all-floating-point aggregates and arrays with memset. Further, aggregates
  // which mix integral and a few floats might also initialize with memset
  // followed by a handful of stores for the floats. Using fairly unique NaNs
  // also means they'll be easier to distinguish in a crash.
  constexpr bool NegativeNaN = true;
  constexpr uint64_t NaNPayload = 0xFFFFFFFFFFFFFFFFull;
  if (Ty->isIntOrIntVectorTy()) {
    unsigned BitWidth =
        cast<toolchain::IntegerType>(Ty->getScalarType())->getBitWidth();
    if (BitWidth <= 64)
      return toolchain::ConstantInt::get(Ty, IntValue);
    return toolchain::ConstantInt::get(
        Ty, toolchain::APInt::getSplat(BitWidth, toolchain::APInt(64, IntValue)));
  }
  if (Ty->isPtrOrPtrVectorTy()) {
    auto *PtrTy = cast<toolchain::PointerType>(Ty->getScalarType());
    unsigned PtrWidth =
        CGM.getDataLayout().getPointerSizeInBits(PtrTy->getAddressSpace());
    if (PtrWidth > 64)
      toolchain_unreachable("pattern initialization of unsupported pointer width");
    toolchain::Type *IntTy = toolchain::IntegerType::get(CGM.getLLVMContext(), PtrWidth);
    auto *Int = toolchain::ConstantInt::get(IntTy, IntValue);
    return toolchain::ConstantExpr::getIntToPtr(Int, PtrTy);
  }
  if (Ty->isFPOrFPVectorTy()) {
    unsigned BitWidth = toolchain::APFloat::semanticsSizeInBits(
        Ty->getScalarType()->getFltSemantics());
    toolchain::APInt Payload(64, NaNPayload);
    if (BitWidth >= 64)
      Payload = toolchain::APInt::getSplat(BitWidth, Payload);
    return toolchain::ConstantFP::getQNaN(Ty, NegativeNaN, &Payload);
  }
  if (Ty->isArrayTy()) {
    // Note: this doesn't touch tail padding (at the end of an object, before
    // the next array object). It is instead handled by replaceUndef.
    auto *ArrTy = cast<toolchain::ArrayType>(Ty);
    toolchain::SmallVector<toolchain::Constant *, 8> Element(
        ArrTy->getNumElements(),
        initializationPatternFor(CGM, ArrTy->getElementType()));
    return toolchain::ConstantArray::get(ArrTy, Element);
  }

  // Note: this doesn't touch struct padding. It will initialize as much union
  // padding as is required for the largest type in the union. Padding is
  // instead handled by replaceUndef. Stores to structs with volatile members
  // don't have a volatile qualifier when initialized according to C++. This is
  // fine because stack-based volatiles don't really have volatile semantics
  // anyways, and the initialization shouldn't be observable.
  auto *StructTy = cast<toolchain::StructType>(Ty);
  toolchain::SmallVector<toolchain::Constant *, 8> Struct(StructTy->getNumElements());
  for (unsigned El = 0; El != Struct.size(); ++El)
    Struct[El] = initializationPatternFor(CGM, StructTy->getElementType(El));
  return toolchain::ConstantStruct::get(StructTy, Struct);
}
