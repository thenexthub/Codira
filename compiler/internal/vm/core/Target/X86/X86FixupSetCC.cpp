//===- X86FixupSetCC.cpp - fix zero-extension of setcc patterns -----------===//
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
// This file defines a pass that fixes zero-extension of setcc patterns.
// X86 setcc instructions are modeled to have no input arguments, and a single
// GR8 output argument. This is consistent with other similar instructions
// (e.g. movb), but means it is impossible to directly generate a setcc into
// the lower GR8 of a specified GR32.
// This means that ISel must select (zext (setcc)) into something like
// seta %al; movzbl %al, %eax.
// Unfortunately, this can cause a stall due to the partial register write
// performed by the setcc. Instead, we can use:
// xor %eax, %eax; seta %al
// This both avoids the stall, and encodes shorter.
//
// Furthurmore, we can use:
// setzua %al
// if feature zero-upper is available. It's faster than the xor+setcc sequence.
// When r16-r31 is used, it even encodes shorter.
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"

using namespace vm::core;

#define DEBUG_TYPE "x86-fixup-setcc"

STATISTIC(NumSubstZexts, "Number of setcc + zext pairs substituted");

namespace {
class X86FixupSetCCLegacy : public MachineFunctionPass {
public:
  static char ID;

  X86FixupSetCCLegacy() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "X86 Fixup SetCC"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // end anonymous namespace

char X86FixupSetCCLegacy::ID = 0;

INITIALIZE_PASS(X86FixupSetCCLegacy, DEBUG_TYPE, DEBUG_TYPE, false, false)

FunctionPass *toolchain::createX86FixupSetCCLegacyPass() {
  return new X86FixupSetCCLegacy();
}

static bool fixupSetCC(MachineFunction &MF) {
  bool Changed = false;
  MachineRegisterInfo *MRI = &MF.getRegInfo();
  const X86Subtarget *ST = &MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = ST->getInstrInfo();

  SmallVector<MachineInstr*, 4> ToErase;

  for (auto &MBB : MF) {
    MachineInstr *FlagsDefMI = nullptr;
    for (auto &MI : MBB) {
      // Remember the most recent preceding eflags defining instruction.
      if (MI.definesRegister(X86::EFLAGS, /*TRI=*/nullptr))
        FlagsDefMI = &MI;

      // Find a setcc/setzucc (if ZU is enabled) that is used by a zext.
      // This doesn't have to be the only use, the transformation is safe
      // regardless.
      if (MI.getOpcode() != X86::SETCCr && MI.getOpcode() != X86::SETZUCCr)
        continue;

      MachineInstr *ZExt = nullptr;
      Register Reg0 = MI.getOperand(0).getReg();
      for (auto &Use : MRI->use_instructions(Reg0))
        if (Use.getOpcode() == X86::MOVZX32rr8)
          ZExt = &Use;

      if (!ZExt)
        continue;

      if (!FlagsDefMI)
        continue;

      // We'd like to put something that clobbers eflags directly before
      // FlagsDefMI. This can't hurt anything after FlagsDefMI, because
      // it, itself, by definition, clobbers eflags. But it may happen that
      // FlagsDefMI also *uses* eflags, in which case the transformation is
      // invalid.
      if (FlagsDefMI->readsRegister(X86::EFLAGS, /*TRI=*/nullptr))
        continue;

      // On 32-bit, we need to be careful to force an ABCD register.
      const TargetRegisterClass *RC =
          ST->is64Bit() ? &X86::GR32RegClass : &X86::GR32_ABCDRegClass;
      if (!MRI->constrainRegClass(ZExt->getOperand(0).getReg(), RC)) {
        // If we cannot constrain the register, we would need an additional copy
        // and are better off keeping the MOVZX32rr8 we have now.
        continue;
      }

      ++NumSubstZexts;
      Changed = true;

      // X86 setcc/setzucc only takes an output GR8, so fake a GR32 input by
      // inserting the setcc/setzucc result into the low byte of the zeroed
      // register.
      Register ZeroReg = MRI->createVirtualRegister(RC);
      if (ST->hasZU()) {
        if (!ST->preferLegacySetCC())
          assert((MI.getOpcode() == X86::SETZUCCr) &&
                 "Expect setzucc instruction!");
        else
          MI.setDesc(TII->get(X86::SETZUCCr));
        BuildMI(*ZExt->getParent(), ZExt, ZExt->getDebugLoc(),
                TII->get(TargetOpcode::IMPLICIT_DEF), ZeroReg);
      } else {
        // Initialize a register with 0. This must go before the eflags def
        BuildMI(MBB, FlagsDefMI, MI.getDebugLoc(), TII->get(X86::MOV32r0),
                ZeroReg);
      }

      BuildMI(*ZExt->getParent(), ZExt, ZExt->getDebugLoc(),
              TII->get(X86::INSERT_SUBREG), ZExt->getOperand(0).getReg())
          .addReg(ZeroReg)
          .addReg(Reg0)
          .addImm(X86::sub_8bit);

      // Redirect the debug-instr-number to the setcc.
      if (unsigned InstrNum = ZExt->peekDebugInstrNum())
        MF.makeDebugValueSubstitution({InstrNum, 0},
                                      {MI.getDebugInstrNum(), 0});

      ToErase.push_back(ZExt);
    }
  }

  for (auto &I : ToErase)
    I->eraseFromParent();

  return Changed;
}

bool X86FixupSetCCLegacy::runOnMachineFunction(MachineFunction &MF) {
  return fixupSetCC(MF);
}

PreservedAnalyses X86FixupSetCCPass::run(MachineFunction &MF,
                                         MachineFunctionAnalysisManager &MFAM) {
  return fixupSetCC(MF) ? getMachineFunctionPassPreservedAnalyses()
                              .preserveSet<CFGAnalyses>()
                        : PreservedAnalyses::all();
}
