//===- TailDuplication.cpp - Duplicate blocks into predecessors' tails ----===//
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
/// \file This pass duplicates basic blocks ending in unconditional branches
/// into the tails of their predecessors, using the TailDuplicator utility
/// class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/TailDuplication.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/CodeGen/LazyMachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MBFIWrapper.h"
#include "vm/core/CodeGen/MachineBranchProbabilityInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/TailDuplicator.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"

using namespace vm::core;

#define DEBUG_TYPE "tailduplication"

namespace {

class TailDuplicateBaseLegacy : public MachineFunctionPass {
  TailDuplicator Duplicator;
  std::unique_ptr<MBFIWrapper> MBFIW;
  bool PreRegAlloc;
public:
  TailDuplicateBaseLegacy(char &PassID, bool PreRegAlloc)
      : MachineFunctionPass(PassID), PreRegAlloc(PreRegAlloc) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineBranchProbabilityInfoWrapperPass>();
    AU.addRequired<LazyMachineBlockFrequencyInfoPass>();
    AU.addRequired<ProfileSummaryInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

class TailDuplicateLegacy : public TailDuplicateBaseLegacy {
public:
  static char ID;
  TailDuplicateLegacy() : TailDuplicateBaseLegacy(ID, false) {
    initializeTailDuplicateLegacyPass(*PassRegistry::getPassRegistry());
  }
};

class EarlyTailDuplicateLegacy : public TailDuplicateBaseLegacy {
public:
  static char ID;
  EarlyTailDuplicateLegacy() : TailDuplicateBaseLegacy(ID, true) {
    initializeEarlyTailDuplicateLegacyPass(*PassRegistry::getPassRegistry());
  }

  MachineFunctionProperties getClearedProperties() const override {
    return MachineFunctionProperties().setNoPHIs();
  }
};

} // end anonymous namespace

char TailDuplicateLegacy::ID;
char EarlyTailDuplicateLegacy::ID;

char &toolchain::TailDuplicateLegacyID = TailDuplicateLegacy::ID;
char &toolchain::EarlyTailDuplicateLegacyID = EarlyTailDuplicateLegacy::ID;

INITIALIZE_PASS(TailDuplicateLegacy, DEBUG_TYPE, "Tail Duplication", false,
                false)
INITIALIZE_PASS(EarlyTailDuplicateLegacy, "early-tailduplication",
                "Early Tail Duplication", false, false)

bool TailDuplicateBaseLegacy::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  auto MBPI = &getAnalysis<MachineBranchProbabilityInfoWrapperPass>().getMBPI();
  auto *PSI = &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
  auto *MBFI = (PSI && PSI->hasProfileSummary()) ?
               &getAnalysis<LazyMachineBlockFrequencyInfoPass>().getBFI() :
               nullptr;
  if (MBFI)
    MBFIW = std::make_unique<MBFIWrapper>(*MBFI);
  Duplicator.initMF(MF, PreRegAlloc, MBPI, MBFI ? MBFIW.get() : nullptr, PSI,
                    /*LayoutMode=*/false);

  bool MadeChange = false;
  while (Duplicator.tailDuplicateBlocks())
    MadeChange = true;

  return MadeChange;
}

template <typename DerivedT, bool PreRegAlloc>
PreservedAnalyses TailDuplicatePassBase<DerivedT, PreRegAlloc>::run(
    MachineFunction &MF, MachineFunctionAnalysisManager &MFAM) {
  MFPropsModifier _(static_cast<DerivedT &>(*this), MF);

  auto *MBPI = &MFAM.getResult<MachineBranchProbabilityAnalysis>(MF);
  auto *PSI = MFAM.getResult<ModuleAnalysisManagerMachineFunctionProxy>(MF)
                  .getCachedResult<ProfileSummaryAnalysis>(
                      *MF.getFunction().getParent());
  auto *MBFI = (PSI && PSI->hasProfileSummary()
                    ? &MFAM.getResult<MachineBlockFrequencyAnalysis>(MF)
                    : nullptr);
  if (MBFI)
    MBFIW = std::make_unique<MBFIWrapper>(*MBFI);

  TailDuplicator Duplicator;
  Duplicator.initMF(MF, PreRegAlloc, MBPI, MBFI ? MBFIW.get() : nullptr, PSI,
                    /*LayoutMode=*/false);
  bool MadeChange = false;
  while (Duplicator.tailDuplicateBlocks())
    MadeChange = true;

  if (!MadeChange)
    return PreservedAnalyses::all();
  return getMachineFunctionPassPreservedAnalyses();
}

template class toolchain::TailDuplicatePassBase<TailDuplicatePass, false>;
template class toolchain::TailDuplicatePassBase<EarlyTailDuplicatePass, true>;
