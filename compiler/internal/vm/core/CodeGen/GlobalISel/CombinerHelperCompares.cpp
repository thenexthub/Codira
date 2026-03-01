//===- CombinerHelperCompares.cpp------------------------------------------===//
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
// This file implements CombinerHelper for G_ICMP.
//
//===----------------------------------------------------------------------===//
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"
#include "vm/core/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerHelper.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"
#include "vm/core/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "vm/core/CodeGen/GlobalISel/Utils.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Support/Casting.h"
#include <cstdlib>

#define DEBUG_TYPE "gi-combiner"

using namespace vm::core;

bool CombinerHelper::constantFoldICmp(const GICmp &ICmp,
                                      const GIConstant &LHSCst,
                                      const GIConstant &RHSCst,
                                      BuildFnTy &MatchInfo) const {
  if (LHSCst.getKind() != GIConstant::GIConstantKind::Scalar)
    return false;

  Register Dst = ICmp.getReg(0);
  LLT DstTy = MRI.getType(Dst);

  if (!isConstantLegalOrBeforeLegalizer(DstTy))
    return false;

  CmpInst::Predicate Pred = ICmp.getCond();
  APInt LHS = LHSCst.getScalarValue();
  APInt RHS = RHSCst.getScalarValue();

  bool Result = ICmpInst::compare(LHS, RHS, Pred);

  MatchInfo = [=](MachineIRBuilder &B) {
    if (Result)
      B.buildConstant(Dst, getICmpTrueVal(getTargetLowering(),
                                          /*IsVector=*/DstTy.isVector(),
                                          /*IsFP=*/false));
    else
      B.buildConstant(Dst, 0);
  };

  return true;
}

bool CombinerHelper::constantFoldFCmp(const GFCmp &FCmp,
                                      const GFConstant &LHSCst,
                                      const GFConstant &RHSCst,
                                      BuildFnTy &MatchInfo) const {
  if (LHSCst.getKind() != GFConstant::GFConstantKind::Scalar)
    return false;

  Register Dst = FCmp.getReg(0);
  LLT DstTy = MRI.getType(Dst);

  if (!isConstantLegalOrBeforeLegalizer(DstTy))
    return false;

  CmpInst::Predicate Pred = FCmp.getCond();
  APFloat LHS = LHSCst.getScalarValue();
  APFloat RHS = RHSCst.getScalarValue();

  bool Result = FCmpInst::compare(LHS, RHS, Pred);

  MatchInfo = [=](MachineIRBuilder &B) {
    if (Result)
      B.buildConstant(Dst, getICmpTrueVal(getTargetLowering(),
                                          /*IsVector=*/DstTy.isVector(),
                                          /*IsFP=*/true));
    else
      B.buildConstant(Dst, 0);
  };

  return true;
}

bool CombinerHelper::matchCanonicalizeICmp(const MachineInstr &MI,
                                           BuildFnTy &MatchInfo) const {
  const GICmp *Cmp = cast<GICmp>(&MI);

  Register Dst = Cmp->getReg(0);
  Register LHS = Cmp->getLHSReg();
  Register RHS = Cmp->getRHSReg();

  CmpInst::Predicate Pred = Cmp->getCond();
  assert(CmpInst::isIntPredicate(Pred) && "Not an integer compare!");
  if (auto CLHS = GIConstant::getConstant(LHS, MRI)) {
    if (auto CRHS = GIConstant::getConstant(RHS, MRI))
      return constantFoldICmp(*Cmp, *CLHS, *CRHS, MatchInfo);

    // If we have a constant, make sure it is on the RHS.
    std::swap(LHS, RHS);
    Pred = CmpInst::getSwappedPredicate(Pred);

    MatchInfo = [=](MachineIRBuilder &B) { B.buildICmp(Pred, Dst, LHS, RHS); };
    return true;
  }

  return false;
}

bool CombinerHelper::matchCanonicalizeFCmp(const MachineInstr &MI,
                                           BuildFnTy &MatchInfo) const {
  const GFCmp *Cmp = cast<GFCmp>(&MI);

  Register Dst = Cmp->getReg(0);
  Register LHS = Cmp->getLHSReg();
  Register RHS = Cmp->getRHSReg();

  CmpInst::Predicate Pred = Cmp->getCond();
  assert(CmpInst::isFPPredicate(Pred) && "Not an FP compare!");

  if (auto CLHS = GFConstant::getConstant(LHS, MRI)) {
    if (auto CRHS = GFConstant::getConstant(RHS, MRI))
      return constantFoldFCmp(*Cmp, *CLHS, *CRHS, MatchInfo);

    // If we have a constant, make sure it is on the RHS.
    std::swap(LHS, RHS);
    Pred = CmpInst::getSwappedPredicate(Pred);

    MatchInfo = [=](MachineIRBuilder &B) {
      B.buildFCmp(Pred, Dst, LHS, RHS, Cmp->getFlags());
    };
    return true;
  }

  return false;
}
