//===- RegionInfo.cpp - SESE region detection analysis --------------------===//
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
// Detects single entry single exit regions in the control flow graph.
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/RegionInfo.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/DominanceFrontier.h"
#include "vm/core/InitializePasses.h"
#ifndef NDEBUG
#include "vm/core/Analysis/RegionPrinter.h"
#endif
#include "vm/core/Analysis/Passes.h"
#include "vm/core/Analysis/RegionInfoImpl.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

#define DEBUG_TYPE "region"

namespace vm::core {

template class RegionBase<RegionTraits<Function>>;
template class RegionNodeBase<RegionTraits<Function>>;
template class RegionInfoBase<RegionTraits<Function>>;

} // end namespace vm::core

STATISTIC(numRegions,       "The # of regions");
STATISTIC(numSimpleRegions, "The # of simple regions");

// Always verify if expensive checking is enabled.

static cl::opt<bool,true>
VerifyRegionInfoX(
  "verify-region-info",
  cl::location(RegionInfoBase<RegionTraits<Function>>::VerifyRegionInfo),
  cl::desc("Verify region info (time consuming)"));

static cl::opt<Region::PrintStyle, true> printStyleX("print-region-style",
  cl::location(RegionInfo::printStyle),
  cl::Hidden,
  cl::desc("style of printing regions"),
  cl::values(
    clEnumValN(Region::PrintNone, "none",  "print no details"),
    clEnumValN(Region::PrintBB, "bb",
               "print regions in detail with block_iterator"),
    clEnumValN(Region::PrintRN, "rn",
               "print regions in detail with element_iterator")));

//===----------------------------------------------------------------------===//
// Region implementation
//

Region::Region(BasicBlock *Entry, BasicBlock *Exit,
               RegionInfo* RI,
               DominatorTree *DT, Region *Parent) :
  RegionBase<RegionTraits<Function>>(Entry, Exit, RI, DT, Parent) {

}

Region::~Region() = default;

//===----------------------------------------------------------------------===//
// RegionInfo implementation
//

RegionInfo::RegionInfo() = default;

RegionInfo::~RegionInfo() = default;

bool RegionInfo::invalidate(Function &F, const PreservedAnalyses &PA,
                            FunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis, all analyses on functions, or the function's
  // CFG has been preserved.
  auto PAC = PA.getChecker<RegionInfoAnalysis>();
  return !(PAC.preserved() || PAC.preservedSet<AllAnalysesOn<Function>>() ||
           PAC.preservedSet<CFGAnalyses>());
}

void RegionInfo::updateStatistics(Region *R) {
  ++numRegions;

  // TODO: Slow. Should only be enabled if -stats is used.
  if (R->isSimple())
    ++numSimpleRegions;
}

void RegionInfo::recalculate(Function &F, DominatorTree *DT_,
                             PostDominatorTree *PDT_, DominanceFrontier *DF_) {
  DT = DT_;
  PDT = PDT_;
  DF = DF_;

  TopLevelRegion = new Region(&F.getEntryBlock(), nullptr,
                              this, DT, nullptr);
  updateStatistics(TopLevelRegion);
  calculate(F);
}

#ifndef NDEBUG
void RegionInfo::view() { viewRegion(this); }

void RegionInfo::viewOnly() { viewRegionOnly(this); }
#endif

//===----------------------------------------------------------------------===//
// RegionInfoPass implementation
//

RegionInfoPass::RegionInfoPass() : FunctionPass(ID) {}

RegionInfoPass::~RegionInfoPass() = default;

bool RegionInfoPass::runOnFunction(Function &F) {
  releaseMemory();

  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  auto DF = &getAnalysis<DominanceFrontierWrapperPass>().getDominanceFrontier();

  RI.recalculate(F, DT, PDT, DF);
  return false;
}

void RegionInfoPass::releaseMemory() {
  RI.releaseMemory();
}

void RegionInfoPass::verifyAnalysis() const {
    RI.verifyAnalysis();
}

void RegionInfoPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<DominanceFrontierWrapperPass>();
}

void RegionInfoPass::print(raw_ostream &OS, const Module *) const {
  RI.print(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void RegionInfoPass::dump() const {
  RI.dump();
}
#endif

char RegionInfoPass::ID = 0;

INITIALIZE_PASS_BEGIN(RegionInfoPass, "regions",
                "Detect single entry single exit regions", true, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominanceFrontierWrapperPass)
INITIALIZE_PASS_END(RegionInfoPass, "regions",
                "Detect single entry single exit regions", true, true)

// Create methods available outside of this file, to use them
// "include/toolchain/LinkAllPasses.h". Otherwise the pass would be deleted by
// the link time optimization.

namespace vm::core {

  FunctionPass *createRegionInfoPass() {
    return new RegionInfoPass();
  }

} // end namespace vm::core

//===----------------------------------------------------------------------===//
// RegionInfoAnalysis implementation
//

AnalysisKey RegionInfoAnalysis::Key;

RegionInfo RegionInfoAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
  RegionInfo RI;
  auto *DT = &AM.getResult<DominatorTreeAnalysis>(F);
  auto *PDT = &AM.getResult<PostDominatorTreeAnalysis>(F);
  auto *DF = &AM.getResult<DominanceFrontierAnalysis>(F);

  RI.recalculate(F, DT, PDT, DF);
  return RI;
}

RegionInfoPrinterPass::RegionInfoPrinterPass(raw_ostream &OS)
  : OS(OS) {}

PreservedAnalyses RegionInfoPrinterPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  OS << "Region Tree for function: " << F.getName() << "\n";
  AM.getResult<RegionInfoAnalysis>(F).print(OS);

  return PreservedAnalyses::all();
}

PreservedAnalyses RegionInfoVerifierPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {
  AM.getResult<RegionInfoAnalysis>(F).verifyAnalysis();

  return PreservedAnalyses::all();
}
