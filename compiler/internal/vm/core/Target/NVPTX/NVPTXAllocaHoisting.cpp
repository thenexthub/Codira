//===-- AllocaHoisting.cpp - Hoist allocas to the entry block --*- C++ -*-===//
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
// Hoist the alloca instructions in the non-entry blocks to the entry blocks.
//
//===----------------------------------------------------------------------===//

#include "NVPTXAllocaHoisting.h"
#include "NVPTX.h"
#include "vm/core/CodeGen/StackProtector.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
using namespace vm::core;

namespace {
// Hoisting the alloca instructions in the non-entry blocks to the entry
// block.
class NVPTXAllocaHoisting : public FunctionPass {
public:
  static char ID; // Pass ID
  NVPTXAllocaHoisting() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<StackProtector>();
  }

  StringRef getPassName() const override {
    return "NVPTX specific alloca hoisting";
  }

  bool runOnFunction(Function &function) override;
};
} // namespace

bool NVPTXAllocaHoisting::runOnFunction(Function &function) {
  bool functionModified = false;
  Function::iterator I = function.begin();
  Instruction *firstTerminatorInst = (I++)->getTerminator();

  for (Function::iterator E = function.end(); I != E; ++I) {
    for (BasicBlock::iterator BI = I->begin(), BE = I->end(); BI != BE;) {
      AllocaInst *allocaInst = dyn_cast<AllocaInst>(BI++);
      if (allocaInst && isa<ConstantInt>(allocaInst->getArraySize())) {
        allocaInst->moveBefore(firstTerminatorInst->getIterator());
        functionModified = true;
      }
    }
  }

  return functionModified;
}

char NVPTXAllocaHoisting::ID = 0;

INITIALIZE_PASS(
    NVPTXAllocaHoisting, "alloca-hoisting",
    "Hoisting alloca instructions in non-entry blocks to the entry block",
    false, false)

FunctionPass *toolchain::createAllocaHoisting() { return new NVPTXAllocaHoisting; }
