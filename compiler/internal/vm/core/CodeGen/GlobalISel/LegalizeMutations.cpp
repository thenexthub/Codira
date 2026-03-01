//===- lib/CodeGen/GlobalISel/LegalizerMutations.cpp - Mutations ----------===//
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
// A library of mutation factories to use for LegalityMutation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"

using namespace vm::core;

LegalizeMutation LegalizeMutations::changeTo(unsigned TypeIdx, LLT Ty) {
  return
      [=](const LegalityQuery &Query) { return std::make_pair(TypeIdx, Ty); };
}

LegalizeMutation LegalizeMutations::changeTo(unsigned TypeIdx,
                                             unsigned FromTypeIdx) {
  return [=](const LegalityQuery &Query) {
    return std::make_pair(TypeIdx, Query.Types[FromTypeIdx]);
  };
}

LegalizeMutation LegalizeMutations::changeElementTo(unsigned TypeIdx,
                                                    unsigned FromTypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT OldTy = Query.Types[TypeIdx];
    const LLT NewTy = Query.Types[FromTypeIdx];
    return std::make_pair(TypeIdx, OldTy.changeElementType(NewTy));
  };
}

LegalizeMutation LegalizeMutations::changeElementTo(unsigned TypeIdx,
                                                    LLT NewEltTy) {
  return [=](const LegalityQuery &Query) {
    const LLT OldTy = Query.Types[TypeIdx];
    return std::make_pair(TypeIdx, OldTy.changeElementType(NewEltTy));
  };
}

LegalizeMutation LegalizeMutations::changeElementCountTo(unsigned TypeIdx,
                                                         unsigned FromTypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT OldTy = Query.Types[TypeIdx];
    const LLT NewTy = Query.Types[FromTypeIdx];
    ElementCount NewEltCount =
        NewTy.isVector() ? NewTy.getElementCount() : ElementCount::getFixed(1);
    return std::make_pair(TypeIdx, OldTy.changeElementCount(NewEltCount));
  };
}

LegalizeMutation LegalizeMutations::changeElementCountTo(unsigned TypeIdx,
                                                         ElementCount EC) {
  return [=](const LegalityQuery &Query) {
    const LLT OldTy = Query.Types[TypeIdx];
    return std::make_pair(TypeIdx, OldTy.changeElementCount(EC));
  };
}

LegalizeMutation LegalizeMutations::changeElementSizeTo(unsigned TypeIdx,
                                                        unsigned FromTypeIdx) {
  return [=](const LegalityQuery &Query) {
    const LLT OldTy = Query.Types[TypeIdx];
    const LLT NewTy = Query.Types[FromTypeIdx];
    const LLT NewEltTy = LLT::scalar(NewTy.getScalarSizeInBits());
    return std::make_pair(TypeIdx, OldTy.changeElementType(NewEltTy));
  };
}

LegalizeMutation LegalizeMutations::widenScalarOrEltToNextPow2(unsigned TypeIdx,
                                                               unsigned Min) {
  return [=](const LegalityQuery &Query) {
    const LLT Ty = Query.Types[TypeIdx];
    unsigned NewEltSizeInBits =
        std::max(1u << Log2_32_Ceil(Ty.getScalarSizeInBits()), Min);
    return std::make_pair(TypeIdx, Ty.changeElementSize(NewEltSizeInBits));
  };
}

LegalizeMutation
LegalizeMutations::widenScalarOrEltToNextMultipleOf(unsigned TypeIdx,
                                                    unsigned Size) {
  return [=](const LegalityQuery &Query) {
    const LLT Ty = Query.Types[TypeIdx];
    unsigned NewEltSizeInBits = alignTo(Ty.getScalarSizeInBits(), Size);
    return std::make_pair(TypeIdx, Ty.changeElementSize(NewEltSizeInBits));
  };
}

LegalizeMutation LegalizeMutations::moreElementsToNextPow2(unsigned TypeIdx,
                                                           unsigned Min) {
  return [=](const LegalityQuery &Query) {
    const LLT VecTy = Query.Types[TypeIdx];
    unsigned NewNumElements =
        std::max(1u << Log2_32_Ceil(VecTy.getNumElements()), Min);
    return std::make_pair(
        TypeIdx, LLT::fixed_vector(NewNumElements, VecTy.getElementType()));
  };
}

LegalizeMutation LegalizeMutations::scalarize(unsigned TypeIdx) {
  return [=](const LegalityQuery &Query) {
    return std::make_pair(TypeIdx, Query.Types[TypeIdx].getElementType());
  };
}
