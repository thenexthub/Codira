//===-- X86LoadValueInjectionRetHardening.cpp - LVI RET hardening for x86 --==//
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
///
/// Description: Replaces every `ret` instruction with the sequence:
/// ```
/// pop <scratch-reg>
/// lfence
/// jmp *<scratch-reg>
/// ```
/// where `<scratch-reg>` is some available scratch register, according to the
/// calling convention of the function being mitigated.
///
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define PASS_KEY "x86-lvi-ret"
#define DEBUG_TYPE PASS_KEY

STATISTIC(NumFences, "Number of LFENCEs inserted for LVI mitigation");
STATISTIC(NumFunctionsConsidered, "Number of functions analyzed");
STATISTIC(NumFunctionsMitigated, "Number of functions for which mitigations "
                                 "were deployed");

namespace {

class X86LoadValueInjectionRetHardeningPass : public MachineFunctionPass {
public:
  X86LoadValueInjectionRetHardeningPass() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override {
    return "X86 Load Value Injection (LVI) Ret-Hardening";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;

  static char ID;
};

} // end anonymous namespace

char X86LoadValueInjectionRetHardeningPass::ID = 0;

bool X86LoadValueInjectionRetHardeningPass::runOnMachineFunction(
    MachineFunction &MF) {
  const X86Subtarget *Subtarget = &MF.getSubtarget<X86Subtarget>();
  if (!Subtarget->useLVIControlFlowIntegrity() || !Subtarget->is64Bit())
    return false; // FIXME: support 32-bit

  // Don't skip functions with the "optnone" attr but participate in opt-bisect.
  const Function &F = MF.getFunction();
  if (!F.hasOptNone() && skipFunction(F))
    return false;

  LLVM_DEBUG(dbgs() << "***** " << getPassName() << " : " << MF.getName()
                    << " *****\n");
  ++NumFunctionsConsidered;
  const X86RegisterInfo *TRI = Subtarget->getRegisterInfo();
  const X86InstrInfo *TII = Subtarget->getInstrInfo();

  bool Modified = false;
  for (auto &MBB : MF) {
    for (auto MBBI = MBB.begin(); MBBI != MBB.end(); ++MBBI) {
      if (MBBI->getOpcode() != X86::RET64)
        continue;

      unsigned ClobberReg = TRI->findDeadCallerSavedReg(MBB, MBBI);
      if (ClobberReg != X86::NoRegister) {
        BuildMI(MBB, MBBI, DebugLoc(), TII->get(X86::POP64r))
            .addReg(ClobberReg, RegState::Define)
            .setMIFlag(MachineInstr::FrameDestroy);
        BuildMI(MBB, MBBI, DebugLoc(), TII->get(X86::LFENCE));
        BuildMI(MBB, MBBI, DebugLoc(), TII->get(X86::JMP64r))
            .addReg(ClobberReg);
        MBB.erase(MBBI);
      } else {
        // In case there is no available scratch register, we can still read
        // from RSP to assert that RSP points to a valid page. The write to RSP
        // is also helpful because it verifies that the stack's write
        // permissions are intact.
        MachineInstr *Fence =
            BuildMI(MBB, MBBI, DebugLoc(), TII->get(X86::LFENCE));
        addRegOffset(BuildMI(MBB, Fence, DebugLoc(), TII->get(X86::SHL64mi)),
                     X86::RSP, false, 0)
            .addImm(0)
            ->addRegisterDead(X86::EFLAGS, TRI);
      }

      ++NumFences;
      Modified = true;
      break;
    }
  }

  if (Modified)
    ++NumFunctionsMitigated;
  return Modified;
}

INITIALIZE_PASS(X86LoadValueInjectionRetHardeningPass, PASS_KEY,
                "X86 LVI ret hardener", false, false)

FunctionPass *toolchain::createX86LoadValueInjectionRetHardeningPass() {
  return new X86LoadValueInjectionRetHardeningPass();
}
