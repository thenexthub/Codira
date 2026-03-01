//===-- X86LowerTileCopy.cpp - Expand Tile Copy Instructions---------------===//
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
// This file defines the pass which lower AMX tile copy instructions. Since
// there is no tile copy instruction, we need store tile register to stack
// and load from stack to another tile register. We need extra GR to hold
// the stride, and we need stack slot to hold the tile data register.
// We would run this pass after copy propagation, so that we don't miss copy
// optimization. And we would run this pass before prolog/epilog insertion,
// so that we can allocate stack slot.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86MachineFunctionInfo.h"
#include "X86Subtarget.h"
#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/DebugLoc.h"

using namespace vm::core;

#define DEBUG_TYPE "x86-lower-tile-copy"

namespace {

class X86LowerTileCopyLegacy : public MachineFunctionPass {
public:
  static char ID;

  X86LowerTileCopyLegacy() : MachineFunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "X86 Lower Tile Copy"; }
};

} // namespace

char X86LowerTileCopyLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(X86LowerTileCopyLegacy, DEBUG_TYPE, "Tile Copy Lowering",
                      false, false)
INITIALIZE_PASS_END(X86LowerTileCopyLegacy, DEBUG_TYPE, "Tile Copy Lowering",
                    false, false)

void X86LowerTileCopyLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

FunctionPass *toolchain::createX86LowerTileCopyLegacyPass() {
  return new X86LowerTileCopyLegacy();
}

static bool lowerTileCopy(MachineFunction &MF) {
  X86MachineFunctionInfo *FuncInfo = MF.getInfo<X86MachineFunctionInfo>();
  if (FuncInfo->getAMXProgModel() != AMXProgModelEnum::ManagedRA)
    return false;

  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  assert(ST.hasAMXTILE() && "Only supported on AMXTILE targets");

  const X86InstrInfo *TII = ST.getInstrInfo();
  const TargetRegisterInfo *TRI = ST.getRegisterInfo();
  BitVector GR64Regs =
      TRI->getAllocatableSet(MF, TRI->getRegClass(X86::GR64RegClassID));
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    LiveRegUnits UsedRegs(*TRI);
    UsedRegs.addLiveOuts(MBB);
    for (MachineInstr &MI : toolchain::make_early_inc_range(reverse(MBB))) {
      UsedRegs.stepBackward(MI);
      if (!MI.isCopy())
        continue;
      MachineOperand &DstMO = MI.getOperand(0);
      MachineOperand &SrcMO = MI.getOperand(1);
      Register SrcReg = SrcMO.getReg();
      Register DstReg = DstMO.getReg();
      if (!X86::TILERegClass.contains(DstReg, SrcReg))
        continue;

      // Allocate stack slot for tile register
      unsigned Size = TRI->getSpillSize(X86::TILERegClass);
      Align Alignment = TRI->getSpillAlign(X86::TILERegClass);
      int TileSS = MF.getFrameInfo().CreateSpillStackObject(Size, Alignment);

      int StrideSS = 0;

      // Pick a killed register to avoid a save/reload.
      Register GR64Cand = X86::NoRegister;
      for (auto RegT : GR64Regs.set_bits()) {
        if (UsedRegs.available(RegT)) {
          GR64Cand = RegT;
          break;
        }
      }

      const DebugLoc &DL = MI.getDebugLoc();
      if (GR64Cand) {
        // mov 64 %reg
        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri), GR64Cand).addImm(64);
      } else {
        // No available register? Save RAX and reload it after use.

        // Allocate stack slot for stride register
        Size = TRI->getSpillSize(X86::GR64RegClass);
        Alignment = TRI->getSpillAlign(X86::GR64RegClass);
        StrideSS = MF.getFrameInfo().CreateSpillStackObject(Size, Alignment);

        // mov %reg (%sp)
        addFrameReference(BuildMI(MBB, MI, DL, TII->get(X86::MOV64mr)),
                          StrideSS)
            .addReg(X86::RAX);
        // mov 64 %reg
        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri), X86::RAX).addImm(64);
      }
      // tilestored %tmm, (%sp, %idx)
#define GET_EGPR_IF_ENABLED(OPC) (ST.hasEGPR() ? OPC##_EVEX : OPC)
      unsigned Opc = GET_EGPR_IF_ENABLED(X86::TILESTORED);
      MachineInstr *NewMI =
          addFrameReference(BuildMI(MBB, MI, DL, TII->get(Opc)), TileSS)
              .addReg(SrcReg, getKillRegState(SrcMO.isKill()));
      MachineOperand *MO = &NewMI->getOperand(X86::AddrIndexReg);
      MO->setReg(GR64Cand ? GR64Cand : X86::RAX);
      // tileloadd (%sp, %idx), %tmm
      Opc = GET_EGPR_IF_ENABLED(X86::TILELOADD);
#undef GET_EGPR_IF_ENABLED
      NewMI = addFrameReference(BuildMI(MBB, MI, DL, TII->get(Opc), DstReg),
                                TileSS);
      MO = &NewMI->getOperand(1 + X86::AddrIndexReg);
      MO->setReg(GR64Cand ? GR64Cand : X86::RAX);
      MO->setIsKill(true);
      if (!GR64Cand) {
        // restore %rax
        // mov (%sp) %rax
        addFrameReference(
            BuildMI(MBB, MI, DL, TII->get(X86::MOV64rm), X86::RAX), StrideSS);
      }
      MI.eraseFromParent();
      Changed = true;
    }
  }
  return Changed;
}

bool X86LowerTileCopyLegacy::runOnMachineFunction(MachineFunction &MF) {
  return lowerTileCopy(MF);
}

PreservedAnalyses
X86LowerTileCopyPass::run(MachineFunction &MF,
                          MachineFunctionAnalysisManager &MFAM) {
  return lowerTileCopy(MF) ? getMachineFunctionPassPreservedAnalyses()
                                 .preserveSet<CFGAnalyses>()
                           : PreservedAnalyses::all();
}
