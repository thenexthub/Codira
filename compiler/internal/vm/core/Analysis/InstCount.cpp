//===-- InstCount.cpp - Collects the count of all instructions ------------===//
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
// This pass collects the count of all instructions and reports them
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/InstCount.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "instcount"

STATISTIC(TotalInsts, "Number of instructions (of all types)");
STATISTIC(TotalBlocks, "Number of basic blocks");
STATISTIC(TotalFuncs, "Number of non-external functions");

#define HANDLE_INST(N, OPCODE, CLASS)                                          \
  STATISTIC(Num##OPCODE##Inst, "Number of " #OPCODE " insts");

#include "vm/core/IR/Instruction.def"

namespace {
class InstCount : public InstVisitor<InstCount> {
  friend class InstVisitor<InstCount>;

  void visitFunction(Function &F) { ++TotalFuncs; }
  void visitBasicBlock(BasicBlock &BB) { ++TotalBlocks; }

#define HANDLE_INST(N, OPCODE, CLASS)                                          \
  void visit##OPCODE(CLASS &) {                                                \
    ++Num##OPCODE##Inst;                                                       \
    ++TotalInsts;                                                              \
  }

#include "vm/core/IR/Instruction.def"

  void visitInstruction(Instruction &I) {
    errs() << "Instruction Count does not know about " << I;
    llvm_unreachable(nullptr);
  }
};
} // namespace

PreservedAnalyses InstCountPass::run(Function &F,
                                     FunctionAnalysisManager &FAM) {
  LLVM_DEBUG(dbgs() << "INSTCOUNT: running on function " << F.getName()
                    << "\n");
  InstCount().visit(F);

  return PreservedAnalyses::all();
}
