//===---------------- X86PreLegalizerCombiner.cpp -------------------------===//
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
/// This pass does combining of machine instructions at the generic MI level,
/// before the legalizer.
///
//===----------------------------------------------------------------------===//
#include "X86.h"
#include "X86TargetMachine.h"
#include "vm/core/CodeGen/GlobalISel/CSEInfo.h"
#include "vm/core/CodeGen/GlobalISel/Combiner.h"
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"
#include "vm/core/CodeGen/GlobalISel/CombinerInfo.h"
#include "vm/core/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "vm/core/CodeGen/GlobalISel/GISelValueTracking.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"
#include "vm/core/CodeGen/GlobalISel/MIPatternMatch.h"
#include "vm/core/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "vm/core/CodeGen/GlobalISel/Utils.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Instructions.h"

#define GET_GICOMBINER_DEPS
#include "X86GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "x86-prelegalizer-combiner"

using namespace vm::core;
using namespace MIPatternMatch;

namespace {

#define GET_GICOMBINER_TYPES
#include "X86GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class X86PreLegalizerCombinerImpl : public Combiner {
protected:
  const CombinerHelper Helper;
  const X86PreLegalizerCombinerImplRuleConfig &RuleConfig;
  const X86Subtarget &STI;

public:
  X86PreLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelValueTracking &VT, GISelCSEInfo *CSEInfo,
      const X86PreLegalizerCombinerImplRuleConfig &RuleConfig,
      const X86Subtarget &STI, MachineDominatorTree *MDT,
      const LegalizerInfo *LI);

  static const char *getName() { return "X86PreLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

  bool tryCombineAllImpl(MachineInstr &I) const;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "X86GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "X86GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

X86PreLegalizerCombinerImpl::X86PreLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelValueTracking &VT, GISelCSEInfo *CSEInfo,
    const X86PreLegalizerCombinerImplRuleConfig &RuleConfig,
    const X86Subtarget &STI, MachineDominatorTree *MDT, const LegalizerInfo *LI)
    : Combiner(MF, CInfo, TPC, &VT, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize=*/true, &VT, MDT, LI),
      RuleConfig(RuleConfig), STI(STI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "X86GenPreLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

bool X86PreLegalizerCombinerImpl::tryCombineAll(MachineInstr &MI) const {
  return tryCombineAllImpl(MI);
}

class X86PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  X86PreLegalizerCombiner();

  StringRef getPassName() const override { return "X86PreLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  X86PreLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void X86PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelValueTrackingAnalysisLegacy>();
  AU.addPreserved<GISelValueTrackingAnalysisLegacy>();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  AU.addPreserved<MachineDominatorTreeWrapperPass>();
  AU.addRequired<GISelCSEAnalysisWrapperPass>();
  AU.addPreserved<GISelCSEAnalysisWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

X86PreLegalizerCombiner::X86PreLegalizerCombiner() : MachineFunctionPass(ID) {
  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool X86PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasFailedISel())
    return false;
  auto &TPC = getAnalysis<TargetPassConfig>();

  // Enable CSE.
  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo = &Wrapper.get(TPC.getCSEConfig());

  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  const LegalizerInfo *LI = ST.getLegalizerInfo();

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOptLevel::None && !skipFunction(F);
  GISelValueTracking *VT =
      &getAnalysis<GISelValueTrackingAnalysisLegacy>().get(MF);
  MachineDominatorTree *MDT =
      &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  CombinerInfo CInfo(/*AllowIllegalOps=*/true, /*ShouldLegalizeIllegal=*/false,
                     /*LegalizerInfo=*/LI, EnableOpt, F.hasOptSize(),
                     F.hasMinSize());

  // This is the first Combiner, so the input IR might contain dead
  // instructions.
  CInfo.EnableFullDCE = true;
  X86PreLegalizerCombinerImpl Impl(MF, CInfo, &TPC, *VT, CSEInfo, RuleConfig,
                                   ST, MDT, LI);
  return Impl.combineMachineInstrs();
}

char X86PreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(X86PreLegalizerCombiner, DEBUG_TYPE,
                      "Combine X86 machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelValueTrackingAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(GISelCSEAnalysisWrapperPass)
INITIALIZE_PASS_END(X86PreLegalizerCombiner, DEBUG_TYPE,
                    "Combine X86 machine instrs before legalization", false,
                    false)

namespace vm::core {
FunctionPass *createX86PreLegalizerCombiner() {
  return new X86PreLegalizerCombiner();
}
} // end namespace vm::core
