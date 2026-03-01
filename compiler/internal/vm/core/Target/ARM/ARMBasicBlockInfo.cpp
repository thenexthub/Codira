//===--- ARMBasicBlockInfo.cpp - Utilities for block sizes ---------------===//
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

#include "ARMBasicBlockInfo.h"
#include "ARM.h"
#include "ARMBaseInstrInfo.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "arm-bb-utils"

using namespace vm::core;

namespace vm::core {

// mayOptimizeThumb2Instruction - Returns true if optimizeThumb2Instructions
// below may shrink MI.
static bool
mayOptimizeThumb2Instruction(const MachineInstr *MI) {
  switch(MI->getOpcode()) {
    // optimizeThumb2Instructions.
    case ARM::t2LEApcrel:
    case ARM::t2LDRpci:
    // optimizeThumb2Branches.
    case ARM::t2B:
    case ARM::t2Bcc:
    case ARM::tBcc:
    // optimizeThumb2JumpTables.
    case ARM::t2BR_JT:
    case ARM::tBR_JTr:
      return true;
  }
  return false;
}

void ARMBasicBlockUtils::computeBlockSize(MachineBasicBlock *MBB) {
  LLVM_DEBUG(dbgs() << "computeBlockSize: " << MBB->getName() << "\n");
  BasicBlockInfo &BBI = BBInfo[MBB->getNumber()];
  BBI.Size = 0;
  BBI.Unalign = 0;
  BBI.PostAlign = Align(1);

  for (MachineInstr &I : *MBB) {
    BBI.Size += TII->getInstSizeInBytes(I);
    // For inline asm, getInstSizeInBytes returns a conservative estimate.
    // The actual size may be smaller, but still a multiple of the instr size.
    if (I.isInlineAsm())
      BBI.Unalign = isThumb ? 1 : 2;
    // Also consider instructions that may be shrunk later.
    else if (isThumb && mayOptimizeThumb2Instruction(&I))
      BBI.Unalign = 1;
  }

  // tBR_JTr contains a .align 2 directive.
  if (!MBB->empty() && MBB->back().getOpcode() == ARM::tBR_JTr) {
    BBI.PostAlign = Align(4);
    MBB->getParent()->ensureAlignment(Align(4));
  }
}

/// getOffsetOf - Return the current offset of the specified machine instruction
/// from the start of the function.  This offset changes as stuff is moved
/// around inside the function.
unsigned ARMBasicBlockUtils::getOffsetOf(MachineInstr *MI) const {
  const MachineBasicBlock *MBB = MI->getParent();

  // The offset is composed of two things: the sum of the sizes of all MBB's
  // before this instruction's block, and the offset from the start of the block
  // it is in.
  unsigned Offset = BBInfo[MBB->getNumber()].Offset;

  // Sum instructions before MI in MBB.
  for (MachineBasicBlock::const_iterator I = MBB->begin(); &*I != MI; ++I) {
    assert(I != MBB->end() && "Didn't find MI in its own basic block?");
    Offset += TII->getInstSizeInBytes(*I);
  }
  return Offset;
}

/// isBBInRange - Returns true if the distance between specific MI and
/// specific BB can fit in MI's displacement field.
bool ARMBasicBlockUtils::isBBInRange(MachineInstr *MI,
                                     MachineBasicBlock *DestBB,
                                     unsigned MaxDisp) const {
  unsigned PCAdj      = isThumb ? 4 : 8;
  unsigned BrOffset   = getOffsetOf(MI) + PCAdj;
  unsigned DestOffset = BBInfo[DestBB->getNumber()].Offset;

  LLVM_DEBUG(dbgs() << "Branch of destination " << printMBBReference(*DestBB)
                    << " from " << printMBBReference(*MI->getParent())
                    << " max delta=" << MaxDisp << " from " << getOffsetOf(MI)
                    << " to " << DestOffset << " offset "
                    << int(DestOffset - BrOffset) << "\t" << *MI);

  if (BrOffset <= DestOffset) {
    // Branch before the Dest.
    if (DestOffset-BrOffset <= MaxDisp)
      return true;
  } else {
    if (BrOffset-DestOffset <= MaxDisp)
      return true;
  }
  return false;
}

void ARMBasicBlockUtils::adjustBBOffsetsAfter(MachineBasicBlock *BB) {
  assert(BB->getParent() == &MF &&
         "Basic block is not a child of the current function.\n");

  unsigned BBNum = BB->getNumber();
  LLVM_DEBUG(dbgs() << "Adjust block:\n"
             << " - name: " << BB->getName() << "\n"
             << " - number: " << BB->getNumber() << "\n"
             << " - function: " << MF.getName() << "\n"
             << "   - blocks: " << MF.getNumBlockIDs() << "\n");

  for(unsigned i = BBNum + 1, e = MF.getNumBlockIDs(); i < e; ++i) {
    // Get the offset and known bits at the end of the layout predecessor.
    // Include the alignment of the current block.
    const Align Align = MF.getBlockNumbered(i)->getAlignment();
    const unsigned Offset = BBInfo[i - 1].postOffset(Align);
    const unsigned KnownBits = BBInfo[i - 1].postKnownBits(Align);

    // This is where block i begins.  Stop if the offset is already correct,
    // and we have updated 2 blocks.  This is the maximum number of blocks
    // changed before calling this function.
    if (i > BBNum + 2 &&
        BBInfo[i].Offset == Offset &&
        BBInfo[i].KnownBits == KnownBits)
      break;

    BBInfo[i].Offset = Offset;
    BBInfo[i].KnownBits = KnownBits;
  }
}

} // end namespace vm::core
