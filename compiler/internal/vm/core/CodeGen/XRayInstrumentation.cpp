//===- XRayInstrumentation.cpp - Adds XRay instrumentation to functions. --===//
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
// This file implements a MachineFunctionPass that inserts the appropriate
// XRay instrumentation instructions. We look for XRay-specific attributes
// on the function to determine whether we should insert the replacement
// operations.
//
//===---------------------------------------------------------------------===//

#include "vm/core/CodeGen/XRayInstrumentation.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionAnalysis.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineLoopInfo.h"
#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Attributes.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

namespace {

struct InstrumentationOptions {
  // Whether to emit PATCHABLE_TAIL_CALL.
  bool HandleTailcall;

  // Whether to emit PATCHABLE_RET/PATCHABLE_FUNCTION_EXIT for all forms of
  // return, e.g. conditional return.
  bool HandleAllReturns;
};

struct XRayInstrumentationLegacy : public MachineFunctionPass {
  static char ID;

  XRayInstrumentationLegacy() : MachineFunctionPass(ID) {
    initializeXRayInstrumentationLegacyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<MachineLoopInfoWrapperPass>();
    AU.addPreserved<MachineDominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

struct XRayInstrumentation {
  XRayInstrumentation(MachineDominatorTree *MDT, MachineLoopInfo *MLI)
      : MDT(MDT), MLI(MLI) {}

  bool run(MachineFunction &MF);

  // Methods for use in the NPM and legacy passes, can be removed once migration
  // is complete.
  static bool alwaysInstrument(Function &F) {
    auto InstrAttr = F.getFnAttribute("function-instrument");
    return InstrAttr.isStringAttribute() &&
           InstrAttr.getValueAsString() == "xray-always";
  }

  static bool needMDTAndMLIAnalyses(Function &F) {
    auto IgnoreLoopsAttr = F.getFnAttribute("xray-ignore-loops");
    auto AlwaysInstrument = XRayInstrumentation::alwaysInstrument(F);
    return !AlwaysInstrument && !IgnoreLoopsAttr.isValid();
  }

private:
  // Replace the original RET instruction with the exit sled code ("patchable
  //   ret" pseudo-instruction), so that at runtime XRay can replace the sled
  //   with a code jumping to XRay trampoline, which calls the tracing handler
  //   and, in the end, issues the RET instruction.
  // This is the approach to go on CPUs which have a single RET instruction,
  //   like x86/x86_64.
  void replaceRetWithPatchableRet(MachineFunction &MF,
                                  const TargetInstrInfo *TII,
                                  InstrumentationOptions);

  // Prepend the original return instruction with the exit sled code ("patchable
  //   function exit" pseudo-instruction), preserving the original return
  //   instruction just after the exit sled code.
  // This is the approach to go on CPUs which have multiple options for the
  //   return instruction, like ARM. For such CPUs we can't just jump into the
  //   XRay trampoline and issue a single return instruction there. We rather
  //   have to call the trampoline and return from it to the original return
  //   instruction of the function being instrumented.
  void prependRetWithPatchableExit(MachineFunction &MF,
                                   const TargetInstrInfo *TII,
                                   InstrumentationOptions);

  MachineDominatorTree *MDT;
  MachineLoopInfo *MLI;
};

} // end anonymous namespace

void XRayInstrumentation::replaceRetWithPatchableRet(
    MachineFunction &MF, const TargetInstrInfo *TII,
    InstrumentationOptions op) {
  // We look for *all* terminators and returns, then replace those with
  // PATCHABLE_RET instructions.
  SmallVector<MachineInstr *, 4> Terminators;
  for (auto &MBB : MF) {
    for (auto &T : MBB.terminators()) {
      unsigned Opc = 0;
      if (T.isReturn() &&
          (op.HandleAllReturns || T.getOpcode() == TII->getReturnOpcode())) {
        // Replace return instructions with:
        //   PATCHABLE_RET <Opcode>, <Operand>...
        Opc = TargetOpcode::PATCHABLE_RET;
      }
      if (TII->isTailCall(T) && op.HandleTailcall) {
        // Treat the tail call as a return instruction, which has a
        // different-looking sled than the normal return case.
        Opc = TargetOpcode::PATCHABLE_TAIL_CALL;
      }
      if (Opc != 0) {
        auto MIB = BuildMI(MBB, T, T.getDebugLoc(), TII->get(Opc))
                       .addImm(T.getOpcode());
        for (auto &MO : T.operands())
          MIB.add(MO);
        Terminators.push_back(&T);
        if (T.shouldUpdateAdditionalCallInfo())
          MF.eraseAdditionalCallInfo(&T);
      }
    }
  }

  for (auto &I : Terminators)
    I->eraseFromParent();
}

void XRayInstrumentation::prependRetWithPatchableExit(
    MachineFunction &MF, const TargetInstrInfo *TII,
    InstrumentationOptions op) {
  for (auto &MBB : MF)
    for (auto &T : MBB.terminators()) {
      unsigned Opc = 0;
      if (T.isReturn() &&
          (op.HandleAllReturns || T.getOpcode() == TII->getReturnOpcode())) {
        Opc = TargetOpcode::PATCHABLE_FUNCTION_EXIT;
      }
      if (TII->isTailCall(T) && op.HandleTailcall) {
        Opc = TargetOpcode::PATCHABLE_TAIL_CALL;
      }
      if (Opc != 0) {
        // Prepend the return instruction with PATCHABLE_FUNCTION_EXIT or
        //   PATCHABLE_TAIL_CALL .
        BuildMI(MBB, T, T.getDebugLoc(), TII->get(Opc));
      }
    }
}

PreservedAnalyses
XRayInstrumentationPass::run(MachineFunction &MF,
                             MachineFunctionAnalysisManager &MFAM) {
  MachineDominatorTree *MDT = nullptr;
  MachineLoopInfo *MLI = nullptr;

  if (XRayInstrumentation::needMDTAndMLIAnalyses(MF.getFunction())) {
    MDT = MFAM.getCachedResult<MachineDominatorTreeAnalysis>(MF);
    MLI = MFAM.getCachedResult<MachineLoopAnalysis>(MF);
  }

  if (!XRayInstrumentation(MDT, MLI).run(MF))
    return PreservedAnalyses::all();

  auto PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool XRayInstrumentationLegacy::runOnMachineFunction(MachineFunction &MF) {
  MachineDominatorTree *MDT = nullptr;
  MachineLoopInfo *MLI = nullptr;
  if (XRayInstrumentation::needMDTAndMLIAnalyses(MF.getFunction())) {
    auto *MDTWrapper =
        getAnalysisIfAvailable<MachineDominatorTreeWrapperPass>();
    MDT = MDTWrapper ? &MDTWrapper->getDomTree() : nullptr;
    auto *MLIWrapper = getAnalysisIfAvailable<MachineLoopInfoWrapperPass>();
    MLI = MLIWrapper ? &MLIWrapper->getLI() : nullptr;
  }
  return XRayInstrumentation(MDT, MLI).run(MF);
}

bool XRayInstrumentation::run(MachineFunction &MF) {
  auto &F = MF.getFunction();
  auto InstrAttr = F.getFnAttribute("function-instrument");
  bool AlwaysInstrument = alwaysInstrument(F);
  bool NeverInstrument = InstrAttr.isStringAttribute() &&
                         InstrAttr.getValueAsString() == "xray-never";
  if (NeverInstrument && !AlwaysInstrument)
    return false;
  auto IgnoreLoopsAttr = F.getFnAttribute("xray-ignore-loops");

  uint64_t XRayThreshold = 0;
  if (!AlwaysInstrument) {
    bool IgnoreLoops = IgnoreLoopsAttr.isValid();
    XRayThreshold = F.getFnAttributeAsParsedInteger(
        "xray-instruction-threshold", std::numeric_limits<uint64_t>::max());
    if (XRayThreshold == std::numeric_limits<uint64_t>::max())
      return false;

    // Count the number of MachineInstr`s in MachineFunction
    uint64_t MICount = 0;
    for (const auto &MBB : MF)
      MICount += MBB.size();

    bool TooFewInstrs = MICount < XRayThreshold;

    if (!IgnoreLoops) {
      // Get MachineDominatorTree or compute it on the fly if it's unavailable
      MachineDominatorTree ComputedMDT;
      if (!MDT) {
        ComputedMDT.recalculate(MF);
        MDT = &ComputedMDT;
      }

      // Get MachineLoopInfo or compute it on the fly if it's unavailable
      MachineLoopInfo ComputedMLI;
      if (!MLI) {
        ComputedMLI.analyze(*MDT);
        MLI = &ComputedMLI;
      }

      // Check if we have a loop.
      // FIXME: Maybe make this smarter, and see whether the loops are dependent
      // on inputs or side-effects?
      if (MLI->empty() && TooFewInstrs)
        return false; // Function is too small and has no loops.
    } else if (TooFewInstrs) {
      // Function is too small
      return false;
    }
  }

  // We look for the first non-empty MachineBasicBlock, so that we can insert
  // the function instrumentation in the appropriate place.
  auto MBI = toolchain::find_if(
      MF, [&](const MachineBasicBlock &MBB) { return !MBB.empty(); });
  if (MBI == MF.end())
    return false; // The function is empty.

  auto *TII = MF.getSubtarget().getInstrInfo();
  auto &FirstMBB = *MBI;
  auto &FirstMI = *FirstMBB.begin();

  if (!MF.getSubtarget().isXRaySupported()) {

    const Function &Fn = FirstMBB.getParent()->getFunction();
    Fn.getContext().diagnose(DiagnosticInfoUnsupported(
        Fn, "An attempt to perform XRay instrumentation for an"
            " unsupported target."));

    return false;
  }

  if (!F.hasFnAttribute("xray-skip-entry")) {
    // First, insert an PATCHABLE_FUNCTION_ENTER as the first instruction of the
    // MachineFunction.
    BuildMI(FirstMBB, FirstMI, FirstMI.getDebugLoc(),
            TII->get(TargetOpcode::PATCHABLE_FUNCTION_ENTER));
  }

  if (!F.hasFnAttribute("xray-skip-exit")) {
    switch (MF.getTarget().getTargetTriple().getArch()) {
    case Triple::ArchType::arm:
    case Triple::ArchType::thumb:
    case Triple::ArchType::aarch64:
    case Triple::ArchType::hexagon:
    case Triple::ArchType::loongarch64:
    case Triple::ArchType::mips:
    case Triple::ArchType::mipsel:
    case Triple::ArchType::mips64:
    case Triple::ArchType::mips64el:
    case Triple::ArchType::riscv32:
    case Triple::ArchType::riscv64: {
      // For the architectures which don't have a single return instruction
      InstrumentationOptions op;
      // AArch64 and RISC-V support patching tail calls.
      op.HandleTailcall = MF.getTarget().getTargetTriple().isAArch64() ||
                          MF.getTarget().getTargetTriple().isRISCV();
      op.HandleAllReturns = true;
      prependRetWithPatchableExit(MF, TII, op);
      break;
    }
    case Triple::ArchType::ppc64le:
    case Triple::ArchType::systemz: {
      // PPC has conditional returns. Turn them into branch and plain returns.
      InstrumentationOptions op;
      op.HandleTailcall = false;
      op.HandleAllReturns = true;
      replaceRetWithPatchableRet(MF, TII, op);
      break;
    }
    default: {
      // For the architectures that have a single return instruction (such as
      //   RETQ on x86_64).
      InstrumentationOptions op;
      op.HandleTailcall = true;
      op.HandleAllReturns = false;
      replaceRetWithPatchableRet(MF, TII, op);
      break;
    }
    }
  }
  return true;
}

char XRayInstrumentationLegacy::ID = 0;
char &toolchain::XRayInstrumentationID = XRayInstrumentationLegacy::ID;
INITIALIZE_PASS_BEGIN(XRayInstrumentationLegacy, "xray-instrumentation",
                      "Insert XRay ops", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfoWrapperPass)
INITIALIZE_PASS_END(XRayInstrumentationLegacy, "xray-instrumentation",
                    "Insert XRay ops", false, false)
