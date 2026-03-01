//===---- KCFI.cpp - Implements Kernel Control-Flow Integrity (KCFI) ------===//
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
// This pass implements Kernel Control-Flow Integrity (KCFI) indirect call
// check lowering. For each call instruction with a cfi-type attribute, it
// emits an arch-specific check before the call, and bundles the check and
// the call to prevent unintentional modifications.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBundle.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "kcfi"
#define KCFI_PASS_NAME "Insert KCFI indirect call checks"

STATISTIC(NumKCFIChecksAdded, "Number of indirect call checks added");

namespace {
class KCFI : public MachineFunctionPass {
public:
  static char ID;

  KCFI() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return KCFI_PASS_NAME; }
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  /// Machine instruction info used throughout the class.
  const TargetInstrInfo *TII = nullptr;

  /// Target lowering for arch-specific parts.
  const TargetLowering *TLI = nullptr;

  /// Emits a KCFI check before an indirect call.
  /// \returns true if the check was added and false otherwise.
  bool emitCheck(MachineBasicBlock &MBB,
                 MachineBasicBlock::instr_iterator I) const;
};

char KCFI::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(KCFI, DEBUG_TYPE, KCFI_PASS_NAME, false, false)

FunctionPass *toolchain::createKCFIPass() { return new KCFI(); }

bool KCFI::emitCheck(MachineBasicBlock &MBB,
                     MachineBasicBlock::instr_iterator MBBI) const {
  assert(TII && "Target instruction info was not initialized");
  assert(TLI && "Target lowering was not initialized");

  // If the call instruction is bundled, we can only emit a check safely if
  // it's the first instruction in the bundle.
  if (MBBI->isBundled() && !std::prev(MBBI)->isBundle())
    report_fatal_error("Cannot emit a KCFI check for a bundled call");

  // Emit a KCFI check for the call instruction at MBBI. The implementation
  // must unfold memory operands if applicable.
  MachineInstr *Check = TLI->EmitKCFICheck(MBB, MBBI, TII);

  // Clear the original call's CFI type.
  assert(MBBI->isCall() && "Unexpected instruction type");
  MBBI->setCFIType(*MBB.getParent(), 0);

  // If not already bundled, bundle the check and the call to prevent
  // further changes.
  if (!MBBI->isBundled())
    finalizeBundle(MBB, Check->getIterator(), std::next(MBBI->getIterator()));

  ++NumKCFIChecksAdded;
  return true;
}

bool KCFI::runOnMachineFunction(MachineFunction &MF) {
  const Module *M = MF.getFunction().getParent();
  if (!M->getModuleFlag("kcfi"))
    return false;

  const auto &SubTarget = MF.getSubtarget();
  TII = SubTarget.getInstrInfo();
  TLI = SubTarget.getTargetLowering();

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    // Use instr_iterator because we don't want to skip bundles.
    for (MachineBasicBlock::instr_iterator MII = MBB.instr_begin(),
                                           MIE = MBB.instr_end();
         MII != MIE; ++MII) {
      if (MII->isCall() && MII->getCFIType())
        Changed |= emitCheck(MBB, MII);
    }
  }

  return Changed;
}
