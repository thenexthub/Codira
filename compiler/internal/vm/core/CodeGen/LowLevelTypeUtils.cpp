//===-- toolchain/CodeGen/LowLevelTypeUtils.cpp --------------------------------===//
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

#include "vm/core/CodeGen/LowLevelTypeUtils.h"
#include "vm/core/ADT/APFloat.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
using namespace vm::core;

LLT toolchain::getLLTForType(Type &Ty, const DataLayout &DL) {
  if (auto VTy = dyn_cast<VectorType>(&Ty)) {
    auto EC = VTy->getElementCount();
    LLT ScalarTy = getLLTForType(*VTy->getElementType(), DL);
    if (EC.isScalar())
      return ScalarTy;
    return LLT::vector(EC, ScalarTy);
  }

  if (auto PTy = dyn_cast<PointerType>(&Ty)) {
    unsigned AddrSpace = PTy->getAddressSpace();
    return LLT::pointer(AddrSpace, DL.getPointerSizeInBits(AddrSpace));
  }

  if (Ty.isSized() && !Ty.isScalableTargetExtTy()) {
    // Aggregates are no different from real scalars as far as GlobalISel is
    // concerned.
    auto SizeInBits = DL.getTypeSizeInBits(&Ty);
    assert(SizeInBits != 0 && "invalid zero-sized type");
    return LLT::scalar(SizeInBits);
  }

  if (Ty.isTokenTy())
    return LLT::token();

  return LLT();
}

MVT toolchain::getMVTForLLT(LLT Ty) {
  if (!Ty.isVector())
    return MVT::getIntegerVT(Ty.getSizeInBits());

  return MVT::getVectorVT(
      MVT::getIntegerVT(Ty.getElementType().getSizeInBits()),
      Ty.getElementCount());
}

EVT toolchain::getApproximateEVTForLLT(LLT Ty, LLVMContext &Ctx) {
  if (Ty.isVector()) {
    EVT EltVT = getApproximateEVTForLLT(Ty.getElementType(), Ctx);
    return EVT::getVectorVT(Ctx, EltVT, Ty.getElementCount());
  }

  return EVT::getIntegerVT(Ctx, Ty.getSizeInBits());
}

LLT toolchain::getLLTForMVT(MVT Ty) {
  if (!Ty.isVector())
    return LLT::scalar(Ty.getSizeInBits());

  return LLT::scalarOrVector(Ty.getVectorElementCount(),
                             Ty.getVectorElementType().getSizeInBits());
}

const toolchain::fltSemantics &toolchain::getFltSemanticForLLT(LLT Ty) {
  assert(Ty.isScalar() && "Expected a scalar type.");
  switch (Ty.getSizeInBits()) {
  case 16:
    return APFloat::IEEEhalf();
  case 32:
    return APFloat::IEEEsingle();
  case 64:
    return APFloat::IEEEdouble();
  case 128:
    return APFloat::IEEEquad();
  }
  llvm_unreachable("Invalid FP type size.");
}
