//===-- MachineFunctionSplitter.cpp - Split machine functions //-----------===//
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
// \file
// Uses profile information to split out cold blocks.
//
// This pass splits out cold machine basic blocks from the parent function. This
// implementation leverages the basic block section framework. Blocks marked
// cold by this pass are grouped together in a separate section prefixed with
// ".text.unlikely.*". The linker can then group these together as a cold
// section. The split part of the function is a contiguous region identified by
// the symbol "foo.cold". Grouping all cold blocks across functions together
// decreases fragmentation and improves icache and itlb utilization. Note that
// the overall changes to the binary size are negligible; only a small number of
// additional jump instructions may be introduced.
//
// For the original RFC of this pass please see
// https://groups.google.com/d/msg/toolchain-dev/RUegaMg-iqc/wFAVxa6fCgAJ
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/BranchProbabilityInfo.h"
#include "vm/core/Analysis/EHUtils.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/CodeGen/BasicBlockSectionUtils.h"
#include "vm/core/CodeGen/BasicBlockSectionsProfileReader.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include <optional>

using namespace vm::core;

// FIXME: This cutoff value is CPU dependent and should be moved to
// TargetTransformInfo once we consider enabling this on other platforms.
// The value is expressed as a ProfileSummaryInfo integer percentile cutoff.
// Defaults to 999950, i.e. all blocks colder than 99.995 percentile are split.
// The default was empirically determined to be optimal when considering cutoff
// values between 99%-ile to 100%-ile with respect to iTLB and icache metrics on
// Intel CPUs.
static cl::opt<unsigned>
    PercentileCutoff("mfs-psi-cutoff",
                     cl::desc("Percentile profile summary cutoff used to "
                              "determine cold blocks. Unused if set to zero."),
                     cl::init(999950), cl::Hidden);

static cl::opt<unsigned> ColdCountThreshold(
    "mfs-count-threshold",
    cl::desc(
        "Minimum number of times a block must be executed to be retained."),
    cl::init(1), cl::Hidden);

static cl::opt<bool> SplitAllEHCode(
    "mfs-split-ehcode",
    cl::desc("Splits all EH code and it's descendants by default."),
    cl::init(false), cl::Hidden);

namespace {

class MachineFunctionSplitter : public MachineFunctionPass {
public:
  static char ID;
  MachineFunctionSplitter() : MachineFunctionPass(ID) {
    initializeMachineFunctionSplitterPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Machine Function Splitter Transformation";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &F) override;
};
} // end anonymous namespace

/// setDescendantEHBlocksCold - This splits all EH pads and blocks reachable
/// only by EH pad as cold. This will help mark EH pads statically cold
/// instead of relying on profile data.
static void setDescendantEHBlocksCold(MachineFunction &MF) {
  DenseSet<MachineBasicBlock *> EHBlocks;
  computeEHOnlyBlocks(MF, EHBlocks);
  for (auto Block : EHBlocks) {
    Block->setSectionID(MBBSectionID::ColdSectionID);
  }
}

static void finishAdjustingBasicBlocksAndLandingPads(MachineFunction &MF) {
  auto Comparator = [](const MachineBasicBlock &X, const MachineBasicBlock &Y) {
    return X.getSectionID().Type < Y.getSectionID().Type;
  };
  toolchain::sortBasicBlocksAndUpdateBranches(MF, Comparator);
  toolchain::avoidZeroOffsetLandingPad(MF);
}

static bool isColdBlock(const MachineBasicBlock &MBB,
                        const MachineBlockFrequencyInfo *MBFI,
                        ProfileSummaryInfo *PSI) {
  std::optional<uint64_t> Count = MBFI->getBlockProfileCount(&MBB);
  // For instrumentation profiles and sample profiles, we use different ways
  // to judge whether a block is cold and should be split.
  if (PSI->hasInstrumentationProfile() || PSI->hasCSInstrumentationProfile()) {
    // If using instrument profile, which is deemed "accurate", no count means
    // cold.
    if (!Count)
      return true;
    if (PercentileCutoff > 0)
      return PSI->isColdCountNthPercentile(PercentileCutoff, *Count);
    // Fallthrough to end of function.
  } else if (PSI->hasSampleProfile()) {
    // For sample profile, no count means "do not judege coldness".
    if (!Count)
      return false;
  }

  return (*Count < ColdCountThreshold);
}

bool MachineFunctionSplitter::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  // Do not split functions when -basic-block-sections=all is specified.
  if (MF.getTarget().getBBSectionsType() == toolchain::BasicBlockSection::All)
    return false;
  // We target functions with profile data. Static information in the form
  // of exception handling code may be split to cold if user passes the
  // mfs-split-ehcode flag.
  bool UseProfileData = MF.getFunction().hasProfileData();
  if (!UseProfileData && !SplitAllEHCode)
    return false;

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  if (!TII.isFunctionSafeToSplit(MF))
    return false;

  // Do not split functions with BasicBlockSections profiles as they will
  // be split by the BasicBlockSections pass.
  auto BBSectionsProfile =
      getAnalysisIfAvailable<BasicBlockSectionsProfileReaderWrapperPass>();
  if (BBSectionsProfile != nullptr &&
      BBSectionsProfile->getBBSPR().isFunctionHot(MF.getName()))
    return false;

  // Renumbering blocks here preserves the order of the blocks as
  // sortBasicBlocksAndUpdateBranches uses the numeric identifier to sort
  // blocks. Preserving the order of blocks is essential to retaining decisions
  // made by prior passes such as MachineBlockPlacement.
  MF.RenumberBlocks();
  MF.setBBSectionsType(BasicBlockSection::Preset);

  MachineBlockFrequencyInfo *MBFI = nullptr;
  ProfileSummaryInfo *PSI = nullptr;
  if (UseProfileData) {
    MBFI = &getAnalysis<MachineBlockFrequencyInfoWrapperPass>().getMBFI();
    PSI = &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
    // If we don't have a good profile (sample profile is not deemed
    // as a "good profile") and the function is not hot, then early
    // return. (Because we can only trust hot functions when profile
    // quality is not good.)
    if (PSI->hasSampleProfile() && !PSI->isFunctionHotInCallGraph(&MF, *MBFI)) {
      // Split all EH code and it's descendant statically by default.
      if (SplitAllEHCode)
        setDescendantEHBlocksCold(MF);
      finishAdjustingBasicBlocksAndLandingPads(MF);
      return true;
    }
  }

  SmallVector<MachineBasicBlock *, 2> LandingPads;
  for (auto &MBB : MF) {
    if (MBB.isEntryBlock())
      continue;

    if (MBB.isEHPad())
      LandingPads.push_back(&MBB);
    else if (UseProfileData && isColdBlock(MBB, MBFI, PSI) &&
             TII.isMBBSafeToSplitToCold(MBB) && !SplitAllEHCode)
      MBB.setSectionID(MBBSectionID::ColdSectionID);
  }

  // Split all EH code and it's descendant statically by default.
  if (SplitAllEHCode)
    setDescendantEHBlocksCold(MF);
  // We only split out eh pads if all of them are cold.
  else {
    // Here we have UseProfileData == true.
    bool HasHotLandingPads = false;
    for (const MachineBasicBlock *LP : LandingPads) {
      if (!isColdBlock(*LP, MBFI, PSI) || !TII.isMBBSafeToSplitToCold(*LP))
        HasHotLandingPads = true;
    }
    if (!HasHotLandingPads) {
      for (MachineBasicBlock *LP : LandingPads)
        LP->setSectionID(MBBSectionID::ColdSectionID);
    }
  }

  finishAdjustingBasicBlocksAndLandingPads(MF);
  return true;
}

void MachineFunctionSplitter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineModuleInfoWrapperPass>();
  AU.addRequired<MachineBlockFrequencyInfoWrapperPass>();
  AU.addRequired<ProfileSummaryInfoWrapperPass>();
  AU.addUsedIfAvailable<BasicBlockSectionsProfileReaderWrapperPass>();
}

char MachineFunctionSplitter::ID = 0;
INITIALIZE_PASS(MachineFunctionSplitter, "machine-function-splitter",
                "Split machine functions using profile information", false,
                false)

MachineFunctionPass *toolchain::createMachineFunctionSplitterPass() {
  return new MachineFunctionSplitter();
}
