//=== lib/CodeGen/GlobalISel/MipsPostLegalizerCombiner.cpp ----------------===//
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
// This pass does combining of machine instructions at the generic MI level,
// after the legalizer.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "Mips.h"
#include "MipsLegalizerInfo.h"
#include "MipsSubtarget.h"
#include "vm/core/CodeGen/GlobalISel/Combiner.h"
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"
#include "vm/core/CodeGen/GlobalISel/CombinerInfo.h"
#include "vm/core/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "vm/core/CodeGen/GlobalISel/GISelValueTracking.h"
#include "vm/core/CodeGen/GlobalISel/MIPatternMatch.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/Target/TargetMachine.h"

#define GET_GICOMBINER_DEPS
#include "MipsGenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_DEPS

#define DEBUG_TYPE "mips-postlegalizer-combiner"

using namespace vm::core;
using namespace MIPatternMatch;

namespace {
#define GET_GICOMBINER_TYPES
#include "MipsGenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class MipsPostLegalizerCombinerImpl : public Combiner {
protected:
  const MipsPostLegalizerCombinerImplRuleConfig &RuleConfig;
  const MipsSubtarget &STI;
  const CombinerHelper Helper;

public:
  MipsPostLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelValueTracking &VT, GISelCSEInfo *CSEInfo,
      const MipsPostLegalizerCombinerImplRuleConfig &RuleConfig,
      const MipsSubtarget &STI, MachineDominatorTree *MDT,
      const LegalizerInfo *LI);

  static const char *getName() { return "MipsPostLegalizerCombiner"; }

  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "MipsGenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "MipsGenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_IMPL

MipsPostLegalizerCombinerImpl::MipsPostLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelValueTracking &VT, GISelCSEInfo *CSEInfo,
    const MipsPostLegalizerCombinerImplRuleConfig &RuleConfig,
    const MipsSubtarget &STI, MachineDominatorTree *MDT,
    const LegalizerInfo *LI)
    : Combiner(MF, CInfo, TPC, &VT, CSEInfo), RuleConfig(RuleConfig), STI(STI),
      Helper(Observer, B, /*IsPreLegalize*/ false, &VT, MDT, LI),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "MipsGenPostLegalizeGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

// Pass boilerplate
// ================

class MipsPostLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  MipsPostLegalizerCombiner(bool IsOptNone = false);

  StringRef getPassName() const override { return "MipsPostLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  bool IsOptNone;
  MipsPostLegalizerCombinerImplRuleConfig RuleConfig;
};
} // end anonymous namespace

void MipsPostLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelValueTrackingAnalysisLegacy>();
  AU.addPreserved<GISelValueTrackingAnalysisLegacy>();
  if (!IsOptNone) {
    AU.addRequired<MachineDominatorTreeWrapperPass>();
    AU.addPreserved<MachineDominatorTreeWrapperPass>();
  }
  MachineFunctionPass::getAnalysisUsage(AU);
}

MipsPostLegalizerCombiner::MipsPostLegalizerCombiner(bool IsOptNone)
    : MachineFunctionPass(ID), IsOptNone(IsOptNone) {
  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool MipsPostLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasFailedISel())
    return false;
  auto *TPC = &getAnalysis<TargetPassConfig>();
  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOptLevel::None && !skipFunction(F);

  const MipsSubtarget &ST = MF.getSubtarget<MipsSubtarget>();
  const MipsLegalizerInfo *LI =
      static_cast<const MipsLegalizerInfo *>(ST.getLegalizerInfo());

  GISelValueTracking *VT =
      &getAnalysis<GISelValueTrackingAnalysisLegacy>().get(MF);
  MachineDominatorTree *MDT =
      IsOptNone ? nullptr
                : &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  CombinerInfo CInfo(/*AllowIllegalOps*/ false, /*ShouldLegalizeIllegal*/ true,
                     LI, EnableOpt, F.hasOptSize(), F.hasMinSize());
  MipsPostLegalizerCombinerImpl Impl(MF, CInfo, TPC, *VT, /*CSEInfo*/ nullptr,
                                     RuleConfig, ST, MDT, LI);
  return Impl.combineMachineInstrs();
}

char MipsPostLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(MipsPostLegalizerCombiner, DEBUG_TYPE,
                      "Combine Mips machine instrs after legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelValueTrackingAnalysisLegacy)
INITIALIZE_PASS_END(MipsPostLegalizerCombiner, DEBUG_TYPE,
                    "Combine Mips machine instrs after legalization", false,
                    false)

FunctionPass *toolchain::createMipsPostLegalizeCombiner(bool IsOptNone) {
  return new MipsPostLegalizerCombiner(IsOptNone);
}
