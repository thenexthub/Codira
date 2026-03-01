//===-- PHIEliminationUtils.cpp - Helper functions for PHI elimination ----===//
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

#include "PHIEliminationUtils.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"

using namespace vm::core;

// Returns true if MBB contains an INLINEASM_BR instruction that may
// branch to SuccMBB, requiring specialized copy placement.
static bool hasInlineAsmBrToSuccessor(MachineBasicBlock *MBB,
                                      MachineBasicBlock *SuccMBB) {
  if (!SuccMBB->isInlineAsmBrIndirectTarget())
    return false;

  for (const MachineInstr &MI : reverse(*MBB))
    if (MI.getOpcode() == TargetOpcode::INLINEASM_BR)
      return true;
  return false;
}

// findCopyInsertPoint - Find a safe place in MBB to insert a copy from SrcReg
// when following the CFG edge to SuccMBB. This needs to be after any def of
// SrcReg, but before any subsequent point where control flow might jump out of
// the basic block.
MachineBasicBlock::iterator
toolchain::findPHICopyInsertPoint(MachineBasicBlock* MBB, MachineBasicBlock* SuccMBB,
                             Register SrcReg) {
  // Handle the trivial case trivially.
  if (MBB->empty())
    return MBB->begin();

  // Usually, we just want to insert the copy before the first terminator
  // instruction. However, for the edge going to a landing pad, we must insert
  // the copy before the call/invoke instruction. Similarly for an INLINEASM_BR
  // going to an indirect target. This is similar to SplitKit.cpp's
  // computeLastInsertPoint, and similarly assumes that there cannot be multiple
  // instructions that are Calls with EHPad successors or INLINEASM_BR in a
  // block.
  // Note that, if the successor basic block happens to be an indirect target,
  // and the current block, which may be the successor itself, does not contain
  // any INLINEASM_BR, we may not need any specialized handling.
  bool EHPadSuccessor = SuccMBB->isEHPad();
  if (!EHPadSuccessor && !hasInlineAsmBrToSuccessor(MBB, SuccMBB))
    return MBB->getFirstTerminator();

  // Discover any defs in this basic block.
  SmallPtrSet<MachineInstr *, 8> DefsInMBB;
  MachineRegisterInfo& MRI = MBB->getParent()->getRegInfo();
  for (MachineInstr &RI : MRI.def_instructions(SrcReg))
    if (RI.getParent() == MBB)
      DefsInMBB.insert(&RI);

  MachineBasicBlock::iterator InsertPoint = MBB->begin();
  // Insert the copy at the _latest_ point of:
  // 1. Immediately AFTER the last def
  // 2. Immediately BEFORE a call/inlineasm_br.
  for (auto I = MBB->rbegin(), E = MBB->rend(); I != E; ++I) {
    if (DefsInMBB.contains(&*I)) {
      InsertPoint = std::next(I.getReverse());
      break;
    }
    if ((EHPadSuccessor && I->isCall()) ||
        I->getOpcode() == TargetOpcode::INLINEASM_BR) {
      InsertPoint = I.getReverse();
      break;
    }
  }

  // Make sure the copy goes after any phi nodes but before
  // any debug nodes.
  return MBB->SkipPHIsAndLabels(InsertPoint);
}
