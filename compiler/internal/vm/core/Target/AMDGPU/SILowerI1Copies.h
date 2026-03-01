//===-- SILowerI1Copies.h --------------------------------------*- C++ -*--===//
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
/// \file
/// Interface definition of the PhiLoweringHelper class that implements lane
/// mask merging algorithm for divergent i1 phis.
//
//===----------------------------------------------------------------------===//

#include "GCNSubtarget.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachinePostDominators.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/MachineSSAUpdater.h"

namespace vm::core {

/// Incoming for lane maks phi as machine instruction, incoming register \p Reg
/// and incoming block \p Block are taken from machine instruction.
/// \p UpdatedReg (if valid) is \p Reg lane mask merged with another lane mask.
struct Incoming {
  Register Reg;
  MachineBasicBlock *Block;
  Register UpdatedReg;

  Incoming(Register Reg, MachineBasicBlock *Block, Register UpdatedReg)
      : Reg(Reg), Block(Block), UpdatedReg(UpdatedReg) {}
};

Register createLaneMaskReg(MachineRegisterInfo *MRI,
                           MachineRegisterInfo::VRegAttrs LaneMaskRegAttrs);

class PhiLoweringHelper {
public:
  PhiLoweringHelper(MachineFunction *MF, MachineDominatorTree *DT,
                    MachinePostDominatorTree *PDT);
  virtual ~PhiLoweringHelper() = default;

protected:
  bool IsWave32 = false;
  MachineFunction *MF = nullptr;
  MachineDominatorTree *DT = nullptr;
  MachinePostDominatorTree *PDT = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  const GCNSubtarget *ST = nullptr;
  const SIInstrInfo *TII = nullptr;
  MachineRegisterInfo::VRegAttrs LaneMaskRegAttrs;

#ifndef NDEBUG
  DenseSet<Register> PhiRegisters;
#endif

  Register ExecReg;
  unsigned MovOp;
  unsigned AndOp;
  unsigned OrOp;
  unsigned XorOp;
  unsigned AndN2Op;
  unsigned OrN2Op;

public:
  bool lowerPhis();
  bool isConstantLaneMask(Register Reg, bool &Val) const;
  MachineBasicBlock::iterator
  getSaluInsertionAtEnd(MachineBasicBlock &MBB) const;

  void initializeLaneMaskRegisterAttributes(Register LaneMask) {
    LaneMaskRegAttrs = MRI->getVRegAttrs(LaneMask);
  }

  void
  initializeLaneMaskRegisterAttributes(MachineRegisterInfo::VRegAttrs Attrs) {
    LaneMaskRegAttrs = Attrs;
  }

  bool isLaneMaskReg(Register Reg) const {
    return TII->getRegisterInfo().isSGPRReg(*MRI, Reg) &&
           TII->getRegisterInfo().getRegSizeInBits(Reg, *MRI) ==
               ST->getWavefrontSize();
  }

  // Helpers from lowerPhis that are different between sdag and global-isel.

  virtual void markAsLaneMask(Register DstReg) const = 0;
  virtual void getCandidatesForLowering(
      SmallVectorImpl<MachineInstr *> &Vreg1Phis) const = 0;
  virtual void
  collectIncomingValuesFromPhi(const MachineInstr *MI,
                               SmallVectorImpl<Incoming> &Incomings) const = 0;
  virtual void replaceDstReg(Register NewReg, Register OldReg,
                             MachineBasicBlock *MBB) = 0;
  virtual void buildMergeLaneMasks(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator I,
                                   const DebugLoc &DL, Register DstReg,
                                   Register PrevReg, Register CurReg) = 0;
  virtual void constrainAsLaneMask(Incoming &In) = 0;
};

} // end namespace vm::core
