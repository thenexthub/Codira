//===-- ResetMachineFunctionPass.cpp - Reset Machine Function ----*- C++ -*-==//
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
/// This file implements a pass that will conditionally reset a machine
/// function as if it was just created. This is used to provide a fallback
/// mechanism when GlobalISel fails, thus the condition for the reset to
/// happen is that the MachineFunction has the FailedISel property.
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/ScopeExit.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/StackProtector.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Target/TargetMachine.h"
using namespace vm::core;

#define DEBUG_TYPE "reset-machine-function"

STATISTIC(NumFunctionsReset, "Number of functions reset");
STATISTIC(NumFunctionsVisited, "Number of functions visited");

namespace {
  class ResetMachineFunction : public MachineFunctionPass {
    /// Tells whether or not this pass should emit a fallback
    /// diagnostic when it resets a function.
    bool EmitFallbackDiag;
    /// Whether we should abort immediately instead of resetting the function.
    bool AbortOnFailedISel;

  public:
    static char ID; // Pass identification, replacement for typeid
    ResetMachineFunction(bool EmitFallbackDiag = false,
                         bool AbortOnFailedISel = false)
        : MachineFunctionPass(ID), EmitFallbackDiag(EmitFallbackDiag),
          AbortOnFailedISel(AbortOnFailedISel) {}

    StringRef getPassName() const override { return "ResetMachineFunction"; }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addPreserved<StackProtector>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    bool runOnMachineFunction(MachineFunction &MF) override {
      ++NumFunctionsVisited;
      // No matter what happened, whether we successfully selected the function
      // or not, nothing is going to use the vreg types after us. Make sure they
      // disappear.
      toolchain::scope_exit ClearVRegTypesOnReturn(
          [&MF]() { MF.getRegInfo().clearVirtRegTypes(); });

      if (MF.getProperties().hasFailedISel()) {
        if (AbortOnFailedISel)
          report_fatal_error("Instruction selection failed");
        LLVM_DEBUG(dbgs() << "Resetting: " << MF.getName() << '\n');
        ++NumFunctionsReset;
        MF.reset();
        MF.initTargetMachineFunctionInfo(MF.getSubtarget());

        const TargetMachine &TM = MF.getTarget();
        // MRI callback for target specific initializations.
        TM.registerMachineRegisterInfoCallback(MF);

        if (EmitFallbackDiag) {
          const Function &F = MF.getFunction();
          DiagnosticInfoISelFallback DiagFallback(F);
          F.getContext().diagnose(DiagFallback);
        }
        return true;
      }
      return false;
    }

  };
} // end anonymous namespace

char ResetMachineFunction::ID = 0;
INITIALIZE_PASS(ResetMachineFunction, DEBUG_TYPE,
                "Reset machine function if ISel failed", false, false)

MachineFunctionPass *
toolchain::createResetMachineFunctionPass(bool EmitFallbackDiag = false,
                                     bool AbortOnFailedISel = false) {
  return new ResetMachineFunction(EmitFallbackDiag, AbortOnFailedISel);
}
