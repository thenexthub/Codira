//==- X86ReturnThunks.cpp - Replace rets with thunks or inline thunks --=//
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
/// \file
///
/// Pass that replaces ret instructions with a jmp to __x86_return_thunk.
///
/// This corresponds to -mfunction-return=thunk-extern or
/// __attribute__((function_return("thunk-extern").
///
/// This pass is a minimal implementation necessary to help mitigate
/// RetBleed for the Linux kernel.
///
/// Should support for thunk or thunk-inline be necessary in the future, then
/// this pass should be combined with x86-retpoline-thunks which already has
/// machinery to emit thunks. Until then, YAGNI.
///
/// This pass is very similar to x86-lvi-ret.
///
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

#define PASS_KEY "x86-return-thunks"
#define DEBUG_TYPE PASS_KEY

namespace {
struct X86ReturnThunks final : public MachineFunctionPass {
  static char ID;
  X86ReturnThunks() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return "X86 Return Thunks"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // namespace

char X86ReturnThunks::ID = 0;

bool X86ReturnThunks::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << getPassName() << "\n");

  bool Modified = false;

  if (!MF.getFunction().hasFnAttribute(toolchain::Attribute::FnRetThunkExtern))
    return Modified;

  StringRef ThunkName = "__x86_return_thunk";
  if (MF.getFunction().getName() == ThunkName)
    return Modified;

  const auto &ST = MF.getSubtarget<X86Subtarget>();
  const bool Is64Bit = ST.getTargetTriple().isX86_64();
  const unsigned RetOpc = Is64Bit ? X86::RET64 : X86::RET32;
  SmallVector<MachineInstr *, 16> Rets;

  for (MachineBasicBlock &MBB : MF)
    for (MachineInstr &Term : MBB.terminators())
      if (Term.getOpcode() == RetOpc)
        Rets.push_back(&Term);

  bool IndCS =
      MF.getFunction().getParent()->getModuleFlag("indirect_branch_cs_prefix");
  const MCInstrDesc &CS = ST.getInstrInfo()->get(X86::CS_PREFIX);
  const MCInstrDesc &JMP = ST.getInstrInfo()->get(X86::TAILJMPd);

  for (MachineInstr *Ret : Rets) {
    if (IndCS)
      BuildMI(Ret->getParent(), Ret->getDebugLoc(), CS);
    BuildMI(Ret->getParent(), Ret->getDebugLoc(), JMP)
        .addExternalSymbol(ThunkName.data());
    Ret->eraseFromParent();
    Modified = true;
  }

  return Modified;
}

INITIALIZE_PASS(X86ReturnThunks, PASS_KEY, "X86 Return Thunks", false, false)

FunctionPass *toolchain::createX86ReturnThunksPass() {
  return new X86ReturnThunks();
}
