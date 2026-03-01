//===- BlockExtractor.cpp - Extracts blocks into their own functions ------===//
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
// This pass extracts the specified basic blocks from the module into their
// own functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/IPO/BlockExtractor.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Transforms/IPO.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/CodeExtractor.h"

using namespace vm::core;

#define DEBUG_TYPE "block-extractor"

STATISTIC(NumExtracted, "Number of basic blocks extracted");

static cl::opt<std::string> BlockExtractorFile(
    "extract-blocks-file", cl::value_desc("filename"),
    cl::desc("A file containing list of basic blocks to extract"), cl::Hidden);

static cl::opt<bool>
    BlockExtractorEraseFuncs("extract-blocks-erase-funcs",
                             cl::desc("Erase the existing functions"),
                             cl::Hidden);
namespace {
class BlockExtractor {
public:
  BlockExtractor(bool EraseFunctions) : EraseFunctions(EraseFunctions) {}
  bool runOnModule(Module &M);
  void
  init(const std::vector<std::vector<BasicBlock *>> &GroupsOfBlocksToExtract) {
    GroupsOfBlocks = GroupsOfBlocksToExtract;
    if (!BlockExtractorFile.empty())
      loadFile();
  }

private:
  std::vector<std::vector<BasicBlock *>> GroupsOfBlocks;
  bool EraseFunctions;
  /// Map a function name to groups of blocks.
  SmallVector<std::pair<std::string, SmallVector<std::string, 4>>, 4>
      BlocksByName;

  void loadFile();
  void splitLandingPadPreds(Function &F);
};

} // end anonymous namespace

/// Gets all of the blocks specified in the input file.
void BlockExtractor::loadFile() {
  auto ErrOrBuf = MemoryBuffer::getFile(BlockExtractorFile);
  if (ErrOrBuf.getError())
    report_fatal_error("BlockExtractor couldn't load the file.");
  // Read the file.
  auto &Buf = *ErrOrBuf;
  SmallVector<StringRef, 16> Lines;
  Buf->getBuffer().split(Lines, '\n', /*MaxSplit=*/-1,
                         /*KeepEmpty=*/false);
  for (const auto &Line : Lines) {
    SmallVector<StringRef, 4> LineSplit;
    Line.split(LineSplit, ' ', /*MaxSplit=*/-1,
               /*KeepEmpty=*/false);
    if (LineSplit.empty())
      continue;
    if (LineSplit.size()!=2)
      reportFatalUsageError(
          "Invalid line format, expecting lines like: 'funcname bb1[;bb2..]'");
    SmallVector<StringRef, 4> BBNames;
    LineSplit[1].split(BBNames, ';', /*MaxSplit=*/-1,
                       /*KeepEmpty=*/false);
    if (BBNames.empty())
      report_fatal_error("Missing bbs name");
    BlocksByName.push_back(
        {std::string(LineSplit[0]), {BBNames.begin(), BBNames.end()}});
  }
}

/// Extracts the landing pads to make sure all of them have only one
/// predecessor.
void BlockExtractor::splitLandingPadPreds(Function &F) {
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (!isa<InvokeInst>(&I))
        continue;
      InvokeInst *II = cast<InvokeInst>(&I);
      BasicBlock *Parent = II->getParent();
      BasicBlock *LPad = II->getUnwindDest();

      // Look through the landing pad's predecessors. If one of them ends in an
      // 'invoke', then we want to split the landing pad.
      bool Split = false;
      for (auto *PredBB : predecessors(LPad)) {
        if (PredBB->isLandingPad() && PredBB != Parent &&
            isa<InvokeInst>(Parent->getTerminator())) {
          Split = true;
          break;
        }
      }

      if (!Split)
        continue;

      SmallVector<BasicBlock *, 2> NewBBs;
      SplitLandingPadPredecessors(LPad, Parent, ".1", ".2", NewBBs);
    }
  }
}

bool BlockExtractor::runOnModule(Module &M) {
  bool Changed = false;

  // Get all the functions.
  SmallVector<Function *, 4> Functions;
  for (Function &F : M) {
    splitLandingPadPreds(F);
    Functions.push_back(&F);
  }

  // Get all the blocks specified in the input file.
  unsigned NextGroupIdx = GroupsOfBlocks.size();
  GroupsOfBlocks.resize(NextGroupIdx + BlocksByName.size());
  for (const auto &BInfo : BlocksByName) {
    Function *F = M.getFunction(BInfo.first);
    if (!F)
      reportFatalUsageError(
          "Invalid function name specified in the input file");
    for (const auto &BBInfo : BInfo.second) {
      auto Res = toolchain::find_if(
          *F, [&](const BasicBlock &BB) { return BB.getName() == BBInfo; });
      if (Res == F->end())
        reportFatalUsageError("Invalid block name specified in the input file");
      GroupsOfBlocks[NextGroupIdx].push_back(&*Res);
    }
    ++NextGroupIdx;
  }

  // Extract each group of basic blocks.
  for (auto &BBs : GroupsOfBlocks) {
    SmallVector<BasicBlock *, 32> BlocksToExtractVec;
    for (BasicBlock *BB : BBs) {
      // Check if the module contains BB.
      if (BB->getParent()->getParent() != &M)
        reportFatalUsageError("Invalid basic block");
      LLVM_DEBUG(dbgs() << "BlockExtractor: Extracting "
                        << BB->getParent()->getName() << ":" << BB->getName()
                        << "\n");
      BlocksToExtractVec.push_back(BB);
      if (const InvokeInst *II = dyn_cast<InvokeInst>(BB->getTerminator()))
        BlocksToExtractVec.push_back(II->getUnwindDest());
      ++NumExtracted;
      Changed = true;
    }
    CodeExtractorAnalysisCache CEAC(*BBs[0]->getParent());
    Function *F = CodeExtractor(BlocksToExtractVec).extractCodeRegion(CEAC);
    if (F)
      LLVM_DEBUG(dbgs() << "Extracted group '" << (*BBs.begin())->getName()
                        << "' in: " << F->getName() << '\n');
    else
      LLVM_DEBUG(dbgs() << "Failed to extract for group '"
                        << (*BBs.begin())->getName() << "'\n");
  }

  // Erase the functions.
  if (EraseFunctions || BlockExtractorEraseFuncs) {
    for (Function *F : Functions) {
      LLVM_DEBUG(dbgs() << "BlockExtractor: Trying to delete " << F->getName()
                        << "\n");
      F->deleteBody();
    }
    // Set linkage as ExternalLinkage to avoid erasing unreachable functions.
    for (Function &F : M)
      F.setLinkage(GlobalValue::ExternalLinkage);
    Changed = true;
  }

  return Changed;
}

BlockExtractorPass::BlockExtractorPass(
    std::vector<std::vector<BasicBlock *>> &&GroupsOfBlocks,
    bool EraseFunctions)
    : GroupsOfBlocks(std::move(GroupsOfBlocks)),
      EraseFunctions(EraseFunctions) {}

PreservedAnalyses BlockExtractorPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  BlockExtractor BE(EraseFunctions);
  BE.init(GroupsOfBlocks);
  return BE.runOnModule(M) ? PreservedAnalyses::none()
                           : PreservedAnalyses::all();
}
