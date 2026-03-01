//===- NVPTXLowerAggrCopies.cpp - ------------------------------*- C++ -*--===//
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
// Lower aggregate copies, memset, memcpy, memmov intrinsics into loops when
// the size is large or is not a compile-time constant.
//
//===----------------------------------------------------------------------===//

#include "NVPTXLowerAggrCopies.h"
#include "NVPTX.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/StackProtector.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/LowerMemIntrinsics.h"

#define DEBUG_TYPE "nvptx"

using namespace vm::core;

namespace {

// actual analysis class, which is a functionpass
struct NVPTXLowerAggrCopies : public FunctionPass {
  static char ID;

  NVPTXLowerAggrCopies() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<StackProtector>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override;

  static const unsigned MaxAggrCopySize = 128;

  StringRef getPassName() const override {
    return "Lower aggregate copies/intrinsics into loops";
  }
};

char NVPTXLowerAggrCopies::ID = 0;

bool NVPTXLowerAggrCopies::runOnFunction(Function &F) {
  SmallVector<LoadInst *, 4> AggrLoads;
  SmallVector<MemIntrinsic *, 4> MemCalls;

  const DataLayout &DL = F.getDataLayout();
  LLVMContext &Context = F.getParent()->getContext();
  const TargetTransformInfo &TTI =
      getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  // Collect all aggregate loads and mem* calls.
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
        if (!LI->hasOneUse())
          continue;

        if (DL.getTypeStoreSize(LI->getType()) < MaxAggrCopySize)
          continue;

        if (StoreInst *SI = dyn_cast<StoreInst>(LI->user_back())) {
          if (SI->getOperand(0) != LI)
            continue;
          AggrLoads.push_back(LI);
        }
      } else if (MemIntrinsic *IntrCall = dyn_cast<MemIntrinsic>(&I)) {
        // Convert intrinsic calls with variable size or with constant size
        // larger than the MaxAggrCopySize threshold.
        if (ConstantInt *LenCI = dyn_cast<ConstantInt>(IntrCall->getLength())) {
          if (LenCI->getZExtValue() >= MaxAggrCopySize) {
            MemCalls.push_back(IntrCall);
          }
        } else {
          MemCalls.push_back(IntrCall);
        }
      }
    }
  }

  if (AggrLoads.size() == 0 && MemCalls.size() == 0) {
    return false;
  }

  //
  // Do the transformation of an aggr load/copy/set to a loop
  //
  for (LoadInst *LI : AggrLoads) {
    auto *SI = cast<StoreInst>(*LI->user_begin());
    Value *SrcAddr = LI->getOperand(0);
    Value *DstAddr = SI->getOperand(1);
    unsigned NumLoads = DL.getTypeStoreSize(LI->getType());
    ConstantInt *CopyLen =
        ConstantInt::get(Type::getInt32Ty(Context), NumLoads);

    createMemCpyLoopKnownSize(/* ConvertedInst */ SI,
                              /* SrcAddr */ SrcAddr, /* DstAddr */ DstAddr,
                              /* CopyLen */ CopyLen,
                              /* SrcAlign */ LI->getAlign(),
                              /* DestAlign */ SI->getAlign(),
                              /* SrcIsVolatile */ LI->isVolatile(),
                              /* DstIsVolatile */ SI->isVolatile(),
                              /* CanOverlap */ true, TTI);

    SI->eraseFromParent();
    LI->eraseFromParent();
  }

  // Transform mem* intrinsic calls.
  for (MemIntrinsic *MemCall : MemCalls) {
    if (MemCpyInst *Memcpy = dyn_cast<MemCpyInst>(MemCall)) {
      expandMemCpyAsLoop(Memcpy, TTI);
    } else if (MemMoveInst *Memmove = dyn_cast<MemMoveInst>(MemCall)) {
      expandMemMoveAsLoop(Memmove, TTI);
    } else if (MemSetInst *Memset = dyn_cast<MemSetInst>(MemCall)) {
      expandMemSetAsLoop(Memset);
    }
    MemCall->eraseFromParent();
  }

  return true;
}

} // namespace

INITIALIZE_PASS(NVPTXLowerAggrCopies, "nvptx-lower-aggr-copies",
                "Lower aggregate copies, and toolchain.mem* intrinsics into loops",
                false, false)

FunctionPass *toolchain::createLowerAggrCopies() {
  return new NVPTXLowerAggrCopies();
}
