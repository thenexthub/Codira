//===--- WebAssemblyOptimizeLiveIntervals.cpp - LiveInterval processing ---===//
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
/// \file
/// Optimize LiveIntervals for use in a post-RA context.
//
/// LiveIntervals normally runs before register allocation when the code is
/// only recently lowered out of SSA form, so it's uncommon for registers to
/// have multiple defs, and when they do, the defs are usually closely related.
/// Later, after coalescing, tail duplication, and other optimizations, it's
/// more common to see registers with multiple unrelated defs. This pass
/// updates LiveIntervals to distribute the value numbers across separate
/// LiveIntervals.
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "vm/core/CodeGen/LiveIntervals.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-optimize-live-intervals"

namespace {
class WebAssemblyOptimizeLiveIntervals final : public MachineFunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly Optimize Live Intervals";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LiveIntervalsWrapperPass>();
    AU.addPreserved<MachineBlockFrequencyInfoWrapperPass>();
    AU.addPreserved<SlotIndexesWrapperPass>();
    AU.addPreserved<LiveIntervalsWrapperPass>();
    AU.addPreservedID(LiveVariablesID);
    AU.addPreservedID(MachineDominatorsID);
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setTracksLiveness();
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyOptimizeLiveIntervals() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyOptimizeLiveIntervals::ID = 0;
INITIALIZE_PASS(WebAssemblyOptimizeLiveIntervals, DEBUG_TYPE,
                "Optimize LiveIntervals for WebAssembly", false, false)

FunctionPass *toolchain::createWebAssemblyOptimizeLiveIntervals() {
  return new WebAssemblyOptimizeLiveIntervals();
}

bool WebAssemblyOptimizeLiveIntervals::runOnMachineFunction(
    MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** Optimize LiveIntervals **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto &LIS = getAnalysis<LiveIntervalsWrapperPass>().getLIS();

  // We don't preserve SSA form.
  MRI.leaveSSA();

  assert(MRI.tracksLiveness() && "OptimizeLiveIntervals expects liveness");

  // Split multiple-VN LiveIntervals into multiple LiveIntervals.
  SmallVector<LiveInterval *, 4> SplitLIs;
  for (unsigned I = 0, E = MRI.getNumVirtRegs(); I < E; ++I) {
    Register Reg = Register::index2VirtReg(I);
    auto &TRI = *MF.getSubtarget<WebAssemblySubtarget>().getRegisterInfo();

    if (MRI.reg_nodbg_empty(Reg))
      continue;

    LIS.splitSeparateComponents(LIS.getInterval(Reg), SplitLIs);
    if (Reg == TRI.getFrameRegister(MF) && SplitLIs.size() > 0) {
      // The live interval for the frame register was split, resulting in a new
      // VReg. For now we only support debug info output for a single frame base
      // value for the function, so just use the last one. It will certainly be
      // wrong for some part of the function, but until we are able to track
      // values through live-range splitting and stackification, it will have to
      // do.
      MF.getInfo<WebAssemblyFunctionInfo>()->setFrameBaseVreg(
          SplitLIs.back()->reg());
    }
    SplitLIs.clear();
  }

  // In FixIrreducibleControlFlow, we conservatively inserted IMPLICIT_DEF
  // instructions to satisfy LiveIntervals' requirement that all uses be
  // dominated by defs. Now that LiveIntervals has computed which of these
  // defs are actually needed and which are dead, remove the dead ones.
  for (MachineInstr &MI : toolchain::make_early_inc_range(MF.front())) {
    if (MI.isImplicitDef() && MI.getOperand(0).isDead()) {
      LiveInterval &LI = LIS.getInterval(MI.getOperand(0).getReg());
      LIS.removeVRegDefAt(LI, LIS.getInstructionIndex(MI).getRegSlot());
      LIS.RemoveMachineInstrFromMaps(MI);
      MI.eraseFromParent();
    }
  }

  return true;
}
