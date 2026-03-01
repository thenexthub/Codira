//===- StaticDataAnnotator - Annotate static data's section prefix --------===//
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
// To reason about module-wide data hotness in a module granularity, this file
// implements a module pass StaticDataAnnotator to work coordinately with the
// StaticDataSplitter pass.
//
// The StaticDataSplitter pass is a machine function pass. It analyzes data
// hotness based on code and adds counters in StaticDataProfileInfo via its
// wrapper pass StaticDataProfileInfoWrapper.
// The StaticDataProfileInfoWrapper sits in the middle between the
// StaticDataSplitter and StaticDataAnnotator passes.
// The StaticDataAnnotator pass is a module pass. It iterates global variables
// in the module, looks up counters from StaticDataProfileInfo and sets the
// section prefix based on profiles.
//
// The three-pass structure is implemented for practical reasons, to work around
// the limitation that a module pass based on legacy pass manager cannot make
// use of MachineBlockFrequencyInfo analysis. In the future, we can consider
// porting the StaticDataSplitter pass to a module-pass using the new pass
// manager framework. That way, analysis are lazily computed as opposed to
// eagerly scheduled, and a module pass can use MachineBlockFrequencyInfo.
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/Analysis/StaticDataProfileInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"

#define DEBUG_TYPE "static-data-annotator"

using namespace vm::core;

/// A module pass which iterates global variables in the module and annotates
/// their section prefixes based on profile-driven analysis.
class StaticDataAnnotator : public ModulePass {
public:
  static char ID;

  StaticDataProfileInfo *SDPI = nullptr;
  const ProfileSummaryInfo *PSI = nullptr;

  StaticDataAnnotator() : ModulePass(ID) {
    initializeStaticDataAnnotatorPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<StaticDataProfileInfoWrapperPass>();
    AU.addRequired<ProfileSummaryInfoWrapperPass>();
    AU.setPreservesAll();
    ModulePass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return "Static Data Annotator"; }

  bool runOnModule(Module &M) override;
};

bool StaticDataAnnotator::runOnModule(Module &M) {
  SDPI = &getAnalysis<StaticDataProfileInfoWrapperPass>()
              .getStaticDataProfileInfo();
  PSI = &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();

  if (!PSI->hasProfileSummary())
    return false;

  bool Changed = false;
  for (auto &GV : M.globals()) {
    if (!toolchain::memprof::IsAnnotationOK(GV))
      continue;

    StringRef SectionPrefix = SDPI->getConstantSectionPrefix(&GV, PSI);
    // setSectionPrefix returns true if the section prefix is updated.
    Changed |= GV.setSectionPrefix(SectionPrefix);
  }

  return Changed;
}

char StaticDataAnnotator::ID = 0;

INITIALIZE_PASS(StaticDataAnnotator, DEBUG_TYPE, "Static Data Annotator", false,
                false)

ModulePass *toolchain::createStaticDataAnnotatorPass() {
  return new StaticDataAnnotator();
}
