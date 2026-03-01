//===-- OpDescriptor.cpp --------------------------------------------------===//
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

#include "vm/core/FuzzMutate/OpDescriptor.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;
using namespace fuzzerop;

static cl::opt<bool> UseUndef("use-undef",
                              cl::desc("Use undef when generating programs."),
                              cl::init(false));

void fuzzerop::makeConstantsWithType(Type *T, std::vector<Constant *> &Cs) {
  if (auto *IntTy = dyn_cast<IntegerType>(T)) {
    uint64_t W = IntTy->getBitWidth();
    Cs.push_back(ConstantInt::get(IntTy, 0));
    Cs.push_back(ConstantInt::get(IntTy, 1));
    Cs.push_back(ConstantInt::get(IntTy, 42, /*IsSigned=*/false,
                                  /*ImplicitTrunc=*/true));
    Cs.push_back(ConstantInt::get(IntTy, APInt::getMaxValue(W)));
    Cs.push_back(ConstantInt::get(IntTy, APInt::getMinValue(W)));
    Cs.push_back(ConstantInt::get(IntTy, APInt::getSignedMaxValue(W)));
    Cs.push_back(ConstantInt::get(IntTy, APInt::getSignedMinValue(W)));
    Cs.push_back(ConstantInt::get(IntTy, APInt::getOneBitSet(W, W / 2)));
  } else if (T->isFloatingPointTy()) {
    auto &Ctx = T->getContext();
    auto &Sem = T->getFltSemantics();
    Cs.push_back(ConstantFP::get(Ctx, APFloat::getZero(Sem)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat(Sem, 1)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat(Sem, 42)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat::getLargest(Sem)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat::getSmallest(Sem)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat::getInf(Sem)));
    Cs.push_back(ConstantFP::get(Ctx, APFloat::getNaN(Sem)));
  } else if (VectorType *VecTy = dyn_cast<VectorType>(T)) {
    std::vector<Constant *> EleCs;
    Type *EltTy = VecTy->getElementType();
    makeConstantsWithType(EltTy, EleCs);
    ElementCount EC = VecTy->getElementCount();
    for (Constant *Elt : EleCs) {
      Cs.push_back(ConstantVector::getSplat(EC, Elt));
    }
  } else {
    if (UseUndef)
      Cs.push_back(UndefValue::get(T));
    Cs.push_back(PoisonValue::get(T));
  }
}

std::vector<Constant *> fuzzerop::makeConstantsWithType(Type *T) {
  std::vector<Constant *> Result;
  makeConstantsWithType(T, Result);
  return Result;
}
