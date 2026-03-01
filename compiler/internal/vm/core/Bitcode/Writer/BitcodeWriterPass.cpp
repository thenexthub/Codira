//===- BitcodeWriterPass.cpp - Bitcode writing pass -----------------------===//
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
// BitcodeWriterPass implementation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Bitcode/BitcodeWriterPass.h"
#include "vm/core/Analysis/ModuleSummaryAnalysis.h"
#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
using namespace vm::core;

PreservedAnalyses BitcodeWriterPass::run(Module &M, ModuleAnalysisManager &AM) {
  M.removeDebugIntrinsicDeclarations();

  const ModuleSummaryIndex *Index =
      EmitSummaryIndex ? &(AM.getResult<ModuleSummaryIndexAnalysis>(M))
                       : nullptr;
  WriteBitcodeToFile(M, OS, ShouldPreserveUseListOrder, Index, EmitModuleHash);

  return PreservedAnalyses::all();
}

namespace {
  class WriteBitcodePass : public ModulePass {
    raw_ostream &OS; // raw_ostream to print on
    bool ShouldPreserveUseListOrder;

  public:
    static char ID; // Pass identification, replacement for typeid
    WriteBitcodePass() : ModulePass(ID), OS(dbgs()) {
      initializeWriteBitcodePassPass(*PassRegistry::getPassRegistry());
    }

    explicit WriteBitcodePass(raw_ostream &o, bool ShouldPreserveUseListOrder)
        : ModulePass(ID), OS(o),
          ShouldPreserveUseListOrder(ShouldPreserveUseListOrder) {
      initializeWriteBitcodePassPass(*PassRegistry::getPassRegistry());
    }

    StringRef getPassName() const override { return "Bitcode Writer"; }

    bool runOnModule(Module &M) override {
      M.removeDebugIntrinsicDeclarations();

      WriteBitcodeToFile(M, OS, ShouldPreserveUseListOrder, /*Index=*/nullptr,
                         /*EmitModuleHash=*/false);

      return false;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
  };
}

char WriteBitcodePass::ID = 0;
INITIALIZE_PASS_BEGIN(WriteBitcodePass, "write-bitcode", "Write Bitcode", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(ModuleSummaryIndexWrapperPass)
INITIALIZE_PASS_END(WriteBitcodePass, "write-bitcode", "Write Bitcode", false,
                    true)

ModulePass *toolchain::createBitcodeWriterPass(raw_ostream &Str,
                                          bool ShouldPreserveUseListOrder) {
  return new WriteBitcodePass(Str, ShouldPreserveUseListOrder);
}

bool toolchain::isBitcodeWriterPass(Pass *P) {
  return P->getPassID() == (toolchain::AnalysisID)&WriteBitcodePass::ID;
}
