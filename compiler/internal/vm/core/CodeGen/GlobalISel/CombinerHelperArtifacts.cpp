//===- CombinerHelperArtifacts.cpp-----------------------------------------===//
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
// This file implements CombinerHelper for legalization artifacts.
//
//===----------------------------------------------------------------------===//
//
// G_MERGE_VALUES
//
//===----------------------------------------------------------------------===//
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerHelper.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"
#include "vm/core/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "vm/core/CodeGen/GlobalISel/Utils.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetOpcodes.h"
#include "vm/core/Support/Casting.h"

#define DEBUG_TYPE "gi-combiner"

using namespace vm::core;

bool CombinerHelper::matchMergeXAndUndef(const MachineInstr &MI,
                                         BuildFnTy &MatchInfo) const {
  const GMerge *Merge = cast<GMerge>(&MI);

  Register Dst = Merge->getReg(0);
  LLT DstTy = MRI.getType(Dst);
  LLT SrcTy = MRI.getType(Merge->getSourceReg(0));

  // Otherwise, we would miscompile.
  assert(Merge->getNumSources() == 2 && "Unexpected number of operands");

  //
  //   %bits_8_15:_(s8) = G_IMPLICIT_DEF
  //   %0:_(s16) = G_MERGE_VALUES %bits_0_7:(s8), %bits_8_15:(s8)
  //
  // ->
  //
  //   %0:_(s16) = G_ANYEXT %bits_0_7:(s8)
  //

  if (!isLegalOrBeforeLegalizer({TargetOpcode::G_ANYEXT, {DstTy, SrcTy}}))
    return false;

  MatchInfo = [=](MachineIRBuilder &B) {
    B.buildAnyExt(Dst, Merge->getSourceReg(0));
  };
  return true;
}

bool CombinerHelper::matchMergeXAndZero(const MachineInstr &MI,
                                        BuildFnTy &MatchInfo) const {
  const GMerge *Merge = cast<GMerge>(&MI);

  Register Dst = Merge->getReg(0);
  LLT DstTy = MRI.getType(Dst);
  LLT SrcTy = MRI.getType(Merge->getSourceReg(0));

  // No multi-use check. It is a constant.

  //
  //   %bits_8_15:_(s8) = G_CONSTANT i8 0
  //   %0:_(s16) = G_MERGE_VALUES %bits_0_7:(s8), %bits_8_15:(s8)
  //
  // ->
  //
  //   %0:_(s16) = G_ZEXT %bits_0_7:(s8)
  //

  if (!isLegalOrBeforeLegalizer({TargetOpcode::G_ZEXT, {DstTy, SrcTy}}))
    return false;

  MatchInfo = [=](MachineIRBuilder &B) {
    B.buildZExt(Dst, Merge->getSourceReg(0));
  };
  return true;
}
