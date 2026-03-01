//===------------ BPFIRPeephole.cpp - IR Peephole Transformation ----------===//
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
// IR level peephole optimization, specifically removing @toolchain.stacksave() and
// @toolchain.stackrestore().
//
//===----------------------------------------------------------------------===//

#include "BPF.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"

#define DEBUG_TYPE "bpf-ir-peephole"

using namespace vm::core;

namespace {

static bool BPFIRPeepholeImpl(Function &F) {
  LLVM_DEBUG(dbgs() << "******** BPF IR Peephole ********\n");

  bool Changed = false;
  Instruction *ToErase = nullptr;
  for (auto &BB : F) {
    for (auto &I : BB) {
      // The following code pattern is handled:
      //     %3 = call i8* @toolchain.stacksave()
      //     store i8* %3, i8** %saved_stack, align 8
      //     ...
      //     %4 = load i8*, i8** %saved_stack, align 8
      //     call void @toolchain.stackrestore(i8* %4)
      //     ...
      // The goal is to remove the above four instructions,
      // so we won't have instructions with r11 (stack pointer)
      // if eventually there is no variable length stack allocation.
      // InstrCombine also tries to remove the above instructions,
      // if it is proven safe (constant alloca etc.), but depending
      // on code pattern, it may still miss some.
      //
      // With unconditionally removing these instructions, if alloca is
      // constant, we are okay then. Otherwise, SelectionDag will complain
      // since BPF does not support dynamic allocation yet.
      if (ToErase) {
        ToErase->eraseFromParent();
        ToErase = nullptr;
      }

      if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
        if (II->getIntrinsicID() != Intrinsic::stacksave)
          continue;
        if (!II->hasOneUser())
          continue;
        auto *Inst = cast<Instruction>(*II->user_begin());
        LLVM_DEBUG(dbgs() << "Remove:"; I.dump());
        LLVM_DEBUG(dbgs() << "Remove:"; Inst->dump(); dbgs() << '\n');
        Changed = true;
        Inst->eraseFromParent();
        ToErase = &I;
        continue;
      }

      if (auto *LD = dyn_cast<LoadInst>(&I)) {
        if (!LD->hasOneUser())
          continue;
        auto *II = dyn_cast<IntrinsicInst>(*LD->user_begin());
        if (!II)
          continue;
        if (II->getIntrinsicID() != Intrinsic::stackrestore)
          continue;
        LLVM_DEBUG(dbgs() << "Remove:"; I.dump());
        LLVM_DEBUG(dbgs() << "Remove:"; II->dump(); dbgs() << '\n');
        Changed = true;
        II->eraseFromParent();
        ToErase = &I;
      }
    }
  }

  return Changed;
}
} // End anonymous namespace

PreservedAnalyses BPFIRPeepholePass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  return BPFIRPeepholeImpl(F) ? PreservedAnalyses::none()
                              : PreservedAnalyses::all();
}
