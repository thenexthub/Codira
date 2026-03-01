//===- CycleAnalysis.cpp - Compute CycleInfo for LLVM IR ------------------===//
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

#include "vm/core/Analysis/CycleAnalysis.h"
#include "vm/core/IR/CFG.h" // for successors found by ADL in GenericCycleImpl.h
#include "vm/core/InitializePasses.h"

using namespace vm::core;

namespace vm::core {
class Module;
} // namespace vm::core

CycleInfo CycleAnalysis::run(Function &F, FunctionAnalysisManager &) {
  CycleInfo CI;
  CI.compute(F);
  return CI;
}

AnalysisKey CycleAnalysis::Key;

CycleInfoPrinterPass::CycleInfoPrinterPass(raw_ostream &OS) : OS(OS) {}

PreservedAnalyses CycleInfoPrinterPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  OS << "CycleInfo for function: " << F.getName() << "\n";
  AM.getResult<CycleAnalysis>(F).print(OS);

  return PreservedAnalyses::all();
}

PreservedAnalyses CycleInfoVerifierPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  CycleInfo &CI = AM.getResult<CycleAnalysis>(F);
  CI.verify();
  return PreservedAnalyses::all();
}

//===----------------------------------------------------------------------===//
//  CycleInfoWrapperPass Implementation
//===----------------------------------------------------------------------===//
//
// The implementation details of the wrapper pass that holds a CycleInfo
// suitable for use with the legacy pass manager.
//
//===----------------------------------------------------------------------===//

char CycleInfoWrapperPass::ID = 0;

CycleInfoWrapperPass::CycleInfoWrapperPass() : FunctionPass(ID) {}

INITIALIZE_PASS_BEGIN(CycleInfoWrapperPass, "cycles", "Cycle Info Analysis",
                      true, true)
INITIALIZE_PASS_END(CycleInfoWrapperPass, "cycles", "Cycle Info Analysis", true,
                    true)

void CycleInfoWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

bool CycleInfoWrapperPass::runOnFunction(Function &Func) {
  CI.clear();

  F = &Func;
  CI.compute(Func);
  return false;
}

void CycleInfoWrapperPass::print(raw_ostream &OS, const Module *) const {
  OS << "CycleInfo for function: " << F->getName() << "\n";
  CI.print(OS);
}

void CycleInfoWrapperPass::releaseMemory() {
  CI.clear();
  F = nullptr;
}
