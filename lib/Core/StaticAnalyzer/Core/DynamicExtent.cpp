//===- DynamicExtent.cpp - Dynamic extent related APIs ----------*- C++ -*-===//
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
//  This file defines APIs that track and query dynamic extent information.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"

REGISTER_MAP_WITH_PROGRAMSTATE(DynamicExtentMap, const language::Core::ento::MemRegion *,
                               language::Core::ento::DefinedOrUnknownSVal)

namespace language::Core {
namespace ento {

DefinedOrUnknownSVal getDynamicExtent(ProgramStateRef State,
                                      const MemRegion *MR, SValBuilder &SVB) {
  MR = MR->StripCasts();

  if (const DefinedOrUnknownSVal *Size = State->get<DynamicExtentMap>(MR))
    if (auto SSize =
            SVB.convertToArrayIndex(*Size).getAs<DefinedOrUnknownSVal>())
      return *SSize;

  return MR->getMemRegionManager().getStaticSize(MR, SVB);
}

DefinedOrUnknownSVal getElementExtent(QualType Ty, SValBuilder &SVB) {
  return SVB.makeIntVal(SVB.getContext().getTypeSizeInChars(Ty).getQuantity(),
                        SVB.getArrayIndexType());
}

static DefinedOrUnknownSVal getConstantArrayElementCount(SValBuilder &SVB,
                                                         const MemRegion *MR) {
  MR = MR->StripCasts();

  const auto *TVR = MR->getAs<TypedValueRegion>();
  if (!TVR)
    return UnknownVal();

  if (const ConstantArrayType *CAT =
          SVB.getContext().getAsConstantArrayType(TVR->getValueType()))
    return SVB.makeIntVal(CAT->getSize(), /* isUnsigned = */ false);

  return UnknownVal();
}

static DefinedOrUnknownSVal
getDynamicElementCount(ProgramStateRef State, SVal Size,
                       DefinedOrUnknownSVal ElementSize) {
  SValBuilder &SVB = State->getStateManager().getSValBuilder();

  auto ElementCount =
      SVB.evalBinOp(State, BO_Div, Size, ElementSize, SVB.getArrayIndexType())
          .getAs<DefinedOrUnknownSVal>();
  return ElementCount.value_or(UnknownVal());
}

DefinedOrUnknownSVal getDynamicElementCount(ProgramStateRef State,
                                            const MemRegion *MR,
                                            SValBuilder &SVB,
                                            QualType ElementTy) {
  assert(MR != nullptr && "Not-null region expected");
  MR = MR->StripCasts();

  DefinedOrUnknownSVal ElementSize = getElementExtent(ElementTy, SVB);
  if (ElementSize.isZeroConstant())
    return getConstantArrayElementCount(SVB, MR);

  return getDynamicElementCount(State, getDynamicExtent(State, MR, SVB),
                                ElementSize);
}

SVal getDynamicExtentWithOffset(ProgramStateRef State, SVal BufV) {
  SValBuilder &SVB = State->getStateManager().getSValBuilder();
  const MemRegion *MRegion = BufV.getAsRegion();
  if (!MRegion)
    return UnknownVal();
  RegionOffset Offset = MRegion->getAsOffset();
  if (Offset.hasSymbolicOffset())
    return UnknownVal();
  const MemRegion *BaseRegion = MRegion->getBaseRegion();
  if (!BaseRegion)
    return UnknownVal();

  NonLoc OffsetInChars =
      SVB.makeArrayIndex(Offset.getOffset() / SVB.getContext().getCharWidth());
  DefinedOrUnknownSVal ExtentInBytes = getDynamicExtent(State, BaseRegion, SVB);

  return SVB.evalBinOp(State, BinaryOperator::Opcode::BO_Sub, ExtentInBytes,
                       OffsetInChars, SVB.getArrayIndexType());
}

DefinedOrUnknownSVal getDynamicElementCountWithOffset(ProgramStateRef State,
                                                      SVal BufV,
                                                      QualType ElementTy) {
  const MemRegion *MR = BufV.getAsRegion();
  if (!MR)
    return UnknownVal();

  SValBuilder &SVB = State->getStateManager().getSValBuilder();
  DefinedOrUnknownSVal ElementSize = getElementExtent(ElementTy, SVB);
  if (ElementSize.isZeroConstant())
    return getConstantArrayElementCount(SVB, MR);

  return getDynamicElementCount(State, getDynamicExtentWithOffset(State, BufV),
                                ElementSize);
}

ProgramStateRef setDynamicExtent(ProgramStateRef State, const MemRegion *MR,
                                 DefinedOrUnknownSVal Size) {
  MR = MR->StripCasts();

  if (Size.isUnknown())
    return State;

  return State->set<DynamicExtentMap>(MR->StripCasts(), Size);
}

} // namespace ento
} // namespace language::Core
