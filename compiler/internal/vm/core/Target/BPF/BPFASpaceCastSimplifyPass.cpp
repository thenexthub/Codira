//===-- BPFASpaceCastSimplifyPass.cpp - BPF addrspacecast simplications --===//
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

#include "BPF.h"
#include <optional>

#define DEBUG_TYPE "bpf-aspace-simplify"

using namespace vm::core;

namespace {

struct CastGEPCast {
  AddrSpaceCastInst *OuterCast;

  // Match chain of instructions:
  //   %inner = addrspacecast N->M
  //   %gep   = getelementptr %inner, ...
  //   %outer = addrspacecast M->N %gep
  // Where I is %outer.
  static std::optional<CastGEPCast> match(Value *I) {
    auto *OuterCast = dyn_cast<AddrSpaceCastInst>(I);
    if (!OuterCast)
      return std::nullopt;
    auto *GEP = dyn_cast<GetElementPtrInst>(OuterCast->getPointerOperand());
    if (!GEP)
      return std::nullopt;
    auto *InnerCast = dyn_cast<AddrSpaceCastInst>(GEP->getPointerOperand());
    if (!InnerCast)
      return std::nullopt;
    if (InnerCast->getSrcAddressSpace() != OuterCast->getDestAddressSpace())
      return std::nullopt;
    if (InnerCast->getDestAddressSpace() != OuterCast->getSrcAddressSpace())
      return std::nullopt;
    return CastGEPCast{OuterCast};
  }

  static PointerType *changeAddressSpace(PointerType *Ty, unsigned AS) {
    return Ty->get(Ty->getContext(), AS);
  }

  // Assuming match(this->OuterCast) is true, convert:
  //   (addrspacecast M->N (getelementptr (addrspacecast N->M ptr) ...))
  // To:
  //   (getelementptr ptr ...)
  GetElementPtrInst *rewrite() {
    auto *GEP = cast<GetElementPtrInst>(OuterCast->getPointerOperand());
    auto *InnerCast = cast<AddrSpaceCastInst>(GEP->getPointerOperand());
    unsigned AS = OuterCast->getDestAddressSpace();
    auto *NewGEP = cast<GetElementPtrInst>(GEP->clone());
    NewGEP->setName(GEP->getName());
    NewGEP->insertAfter(OuterCast->getIterator());
    NewGEP->setOperand(0, InnerCast->getPointerOperand());
    auto *GEPTy = cast<PointerType>(GEP->getType());
    NewGEP->mutateType(changeAddressSpace(GEPTy, AS));
    OuterCast->replaceAllUsesWith(NewGEP);
    OuterCast->eraseFromParent();
    if (GEP->use_empty())
      GEP->eraseFromParent();
    if (InnerCast->use_empty())
      InnerCast->eraseFromParent();
    return NewGEP;
  }
};

} // anonymous namespace

PreservedAnalyses BPFASpaceCastSimplifyPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  SmallVector<CastGEPCast, 16> WorkList;
  bool Changed = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB)
      if (auto It = CastGEPCast::match(&I))
        WorkList.push_back(It.value());
    Changed |= !WorkList.empty();

    while (!WorkList.empty()) {
      CastGEPCast InsnChain = WorkList.pop_back_val();
      GetElementPtrInst *NewGEP = InsnChain.rewrite();
      for (User *U : NewGEP->users())
        if (auto It = CastGEPCast::match(U))
          WorkList.push_back(It.value());
    }
  }
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
