//===- LiveDebugValues.cpp - Tracking Debug Value MIs ---------------------===//
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

#include "LiveDebugValues.h"

#include "vm/core/CodeGen/LiveDebugValuesPass.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/Triple.h"

/// \file LiveDebugValues.cpp
///
/// The LiveDebugValues pass extends the range of variable locations
/// (specified by DBG_VALUE instructions) from single blocks to successors
/// and any other code locations where the variable location is valid.
/// There are currently two implementations: the "VarLoc" implementation
/// explicitly tracks the location of a variable, while the "InstrRef"
/// implementation tracks the values defined by instructions through locations.
///
/// This file implements neither; it merely registers the pass, allows the
/// user to pick which implementation will be used to propagate variable
/// locations.

#define DEBUG_TYPE "livedebugvalues"

using namespace vm::core;

static cl::opt<bool>
    ForceInstrRefLDV("force-instr-ref-livedebugvalues", cl::Hidden,
                     cl::desc("Use instruction-ref based LiveDebugValues with "
                              "normal DBG_VALUE inputs"),
                     cl::init(false));

static cl::opt<cl::boolOrDefault> ValueTrackingVariableLocations(
    "experimental-debug-variable-locations",
    cl::desc("Use experimental new value-tracking variable locations"));

// Options to prevent pathological compile-time behavior. If InputBBLimit and
// InputDbgValueLimit are both exceeded, range extension is disabled.
static cl::opt<unsigned> InputBBLimit(
    "livedebugvalues-input-bb-limit",
    cl::desc("Maximum input basic blocks before DBG_VALUE limit applies"),
    cl::init(10000), cl::Hidden);
static cl::opt<unsigned> InputDbgValueLimit(
    "livedebugvalues-input-dbg-value-limit",
    cl::desc(
        "Maximum input DBG_VALUE insts supported by debug range extension"),
    cl::init(50000), cl::Hidden);

namespace {
/// Generic LiveDebugValues pass. Calls through to VarLocBasedLDV or
/// InstrRefBasedLDV to perform location propagation, via the LDVImpl
/// base class.
class LiveDebugValuesLegacy : public MachineFunctionPass {
public:
  static char ID;

  LiveDebugValuesLegacy();
  ~LiveDebugValuesLegacy() override = default;

  /// Calculate the liveness information for the given machine function.
  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<TargetPassConfig>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

struct LiveDebugValues {
  LiveDebugValues();
  ~LiveDebugValues() = default;
  bool run(MachineFunction &MF, bool ShouldEmitDebugEntryValues);

private:
  std::unique_ptr<LDVImpl> InstrRefImpl;
  std::unique_ptr<LDVImpl> VarLocImpl;
  MachineDominatorTree MDT;
};
} // namespace

char LiveDebugValuesLegacy::ID = 0;

char &toolchain::LiveDebugValuesID = LiveDebugValuesLegacy::ID;

INITIALIZE_PASS(LiveDebugValuesLegacy, DEBUG_TYPE, "Live DEBUG_VALUE analysis",
                false, false)

/// Default construct and initialize the pass.
LiveDebugValuesLegacy::LiveDebugValuesLegacy() : MachineFunctionPass(ID) {
  initializeLiveDebugValuesLegacyPass(*PassRegistry::getPassRegistry());
}

LiveDebugValues::LiveDebugValues() {
  InstrRefImpl =
      std::unique_ptr<LDVImpl>(toolchain::makeInstrRefBasedLiveDebugValues());
  VarLocImpl = std::unique_ptr<LDVImpl>(toolchain::makeVarLocBasedLiveDebugValues());
}

PreservedAnalyses
LiveDebugValuesPass::run(MachineFunction &MF,
                         MachineFunctionAnalysisManager &MFAM) {
  if (!LiveDebugValues().run(MF, ShouldEmitDebugEntryValues))
    return PreservedAnalyses::all();
  auto PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

void LiveDebugValuesPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  OS << MapClassName2PassName(name());
  if (ShouldEmitDebugEntryValues)
    OS << "<emit-debug-entry-values>";
}

bool LiveDebugValuesLegacy::runOnMachineFunction(MachineFunction &MF) {
  auto *TPC = &getAnalysis<TargetPassConfig>();
  return LiveDebugValues().run(
      MF, TPC->getTM<TargetMachine>().Options.ShouldEmitDebugEntryValues());
}

bool LiveDebugValues::run(MachineFunction &MF,
                          bool ShouldEmitDebugEntryValues) {
  bool InstrRefBased = MF.useDebugInstrRef();
  // Allow the user to force selection of InstrRef LDV.
  InstrRefBased |= ForceInstrRefLDV;

  LDVImpl *TheImpl = &*VarLocImpl;

  MachineDominatorTree *DomTree = nullptr;
  if (InstrRefBased) {
    DomTree = &MDT;
    MDT.recalculate(MF);
    TheImpl = &*InstrRefImpl;
  }

  return TheImpl->ExtendRanges(MF, DomTree, ShouldEmitDebugEntryValues,
                               InputBBLimit, InputDbgValueLimit);
}

bool toolchain::debuginfoShouldUseDebugInstrRef(const Triple &T) {
  // Enable by default on x86_64, disable if explicitly turned off on cmdline.
  if (T.getArch() == toolchain::Triple::x86_64 &&
      ValueTrackingVariableLocations != cl::boolOrDefault::BOU_FALSE)
    return true;

  // Enable if explicitly requested on command line.
  return ValueTrackingVariableLocations == cl::boolOrDefault::BOU_TRUE;
}
