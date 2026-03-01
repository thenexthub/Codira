//=== lib/CodeGen/GlobalISel/MipsPreLegalizerCombiner.cpp --------------===//
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
// before the legalizer.
//
//===----------------------------------------------------------------------===//

#include "MipsLegalizerInfo.h"
#include "MipsTargetMachine.h"
#include "vm/core/CodeGen/GlobalISel/Combiner.h"
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"
#include "vm/core/CodeGen/GlobalISel/CombinerInfo.h"
#include "vm/core/CodeGen/GlobalISel/GISelValueTracking.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/TargetPassConfig.h"

#define DEBUG_TYPE "mips-prelegalizer-combiner"

using namespace vm::core;

namespace {
struct MipsPreLegalizerCombinerInfo : public CombinerInfo {
public:
  MipsPreLegalizerCombinerInfo()
      : CombinerInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, /*EnableOpt*/ false,
                     /*EnableOptSize*/ false, /*EnableMinSize*/ false) {}
};

class MipsPreLegalizerCombinerImpl : public Combiner {
protected:
  const MipsSubtarget &STI;
  const CombinerHelper Helper;

public:
  MipsPreLegalizerCombinerImpl(MachineFunction &MF, CombinerInfo &CInfo,
                               const TargetPassConfig *TPC,
                               GISelValueTracking &VT, GISelCSEInfo *CSEInfo,
                               const MipsSubtarget &STI,
                               MachineDominatorTree *MDT,
                               const LegalizerInfo *LI)
      : Combiner(MF, CInfo, TPC, &VT, CSEInfo), STI(STI),
        Helper(Observer, B, /*IsPreLegalize*/ true, &VT, MDT, LI) {}

  static const char *getName() { return "MipsPreLegalizerCombiner"; }

  void setupGeneratedPerFunctionState(MachineFunction &MF) override {
    // TODO: TableGen-erate this class' impl.
  }

  bool tryCombineAll(MachineInstr &MI) const override {
    switch (MI.getOpcode()) {
    default:
      return false;
    case TargetOpcode::G_MEMCPY_INLINE:
      return Helper.tryEmitMemcpyInline(MI);
    case TargetOpcode::G_LOAD:
    case TargetOpcode::G_SEXTLOAD:
    case TargetOpcode::G_ZEXTLOAD: {
      // Don't attempt to combine non power of 2 loads or unaligned loads when
      // subtarget doesn't support them.
      auto MMO = *MI.memoperands_begin();
      const MipsSubtarget &STI = MI.getMF()->getSubtarget<MipsSubtarget>();
      if (!MMO->getSize().hasValue() ||
          !isPowerOf2_64(MMO->getSize().getValue()))
        return false;
      bool isUnaligned = MMO->getAlign() < MMO->getSize().getValue();
      if (!STI.systemSupportsUnalignedAccess() && isUnaligned)
        return false;

      return Helper.tryCombineExtendingLoads(MI);
    }
    }

    return false;
  }
};

// Pass boilerplate
// ================

class MipsPreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;

  MipsPreLegalizerCombiner();

  StringRef getPassName() const override { return "MipsPreLegalizerCombiner"; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // end anonymous namespace

void MipsPreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.addRequired<GISelValueTrackingAnalysisLegacy>();
  AU.addPreserved<GISelValueTrackingAnalysisLegacy>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  MachineFunctionPass::getAnalysisUsage(AU);
}

MipsPreLegalizerCombiner::MipsPreLegalizerCombiner()
    : MachineFunctionPass(ID) {}

bool MipsPreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasFailedISel())
    return false;

  auto *TPC = &getAnalysis<TargetPassConfig>();
  const MipsSubtarget &ST = MF.getSubtarget<MipsSubtarget>();
  const MipsLegalizerInfo *LI =
      static_cast<const MipsLegalizerInfo *>(ST.getLegalizerInfo());

  GISelValueTracking *VT =
      &getAnalysis<GISelValueTrackingAnalysisLegacy>().get(MF);
  MipsPreLegalizerCombinerInfo PCInfo;
  MipsPreLegalizerCombinerImpl Impl(MF, PCInfo, TPC, *VT, /*CSEInfo*/ nullptr,
                                    ST, /*MDT*/ nullptr, LI);
  return Impl.combineMachineInstrs();
}

char MipsPreLegalizerCombiner::ID = 0;
INITIALIZE_PASS_BEGIN(MipsPreLegalizerCombiner, DEBUG_TYPE,
                      "Combine Mips machine instrs before legalization", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelValueTrackingAnalysisLegacy)
INITIALIZE_PASS_END(MipsPreLegalizerCombiner, DEBUG_TYPE,
                    "Combine Mips machine instrs before legalization", false,
                    false)

FunctionPass *toolchain::createMipsPreLegalizeCombiner() {
  return new MipsPreLegalizerCombiner();
}
