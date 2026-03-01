//===- CoroAnnotationElide.cpp - Elide attributed safe coroutine calls ----===//
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
// This pass transforms all Call or Invoke instructions that are annotated
// "coro_elide_safe" to call the `.noalloc` variant of coroutine instead.
// The frame of the callee coroutine is allocated inside the caller. A pointer
// to the allocated frame will be passed into the `.noalloc` ramp function.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Coroutines/CoroAnnotationElide.h"

#include "vm/core/Analysis/CGSCCPassManager.h"
#include "vm/core/Analysis/LazyCallGraph.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Support/BranchProbability.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Transforms/Utils/CallGraphUpdater.h"
#include "vm/core/Transforms/Utils/Cloning.h"

#include <cassert>

using namespace vm::core;

#define DEBUG_TYPE "coro-annotation-elide"

static cl::opt<float> CoroElideBranchRatio(
    "coro-elide-branch-ratio", cl::init(0.55), cl::Hidden,
    cl::desc("Minimum BranchProbability to consider a elide a coroutine."));
extern cl::opt<unsigned> MinBlockCounterExecution;

static Instruction *getFirstNonAllocaInTheEntryBlock(Function *F) {
  for (Instruction &I : F->getEntryBlock())
    if (!isa<AllocaInst>(&I))
      return &I;
  llvm_unreachable("no terminator in the entry block");
}

// Create an alloca in the caller, using FrameSize and FrameAlign as the callee
// coroutine's activation frame.
static Value *allocateFrameInCaller(Function *Caller, uint64_t FrameSize,
                                    Align FrameAlign) {
  LLVMContext &C = Caller->getContext();
  BasicBlock::iterator InsertPt =
      getFirstNonAllocaInTheEntryBlock(Caller)->getIterator();
  const DataLayout &DL = Caller->getDataLayout();
  auto FrameTy = ArrayType::get(Type::getInt8Ty(C), FrameSize);
  auto *Frame = new AllocaInst(FrameTy, DL.getAllocaAddrSpace(), "", InsertPt);
  Frame->setAlignment(FrameAlign);
  return Frame;
}

// Given a call or invoke instruction to the elide safe coroutine, this function
// does the following:
//  - Allocate a frame for the callee coroutine in the caller using alloca.
//  - Replace the old CB with a new Call or Invoke to `NewCallee`, with the
//    pointer to the frame as an additional argument to NewCallee.
static void processCall(CallBase *CB, Function *Caller, Function *NewCallee,
                        uint64_t FrameSize, Align FrameAlign) {
  // TODO: generate the lifetime intrinsics for the new frame. This will require
  // introduction of two pesudo lifetime intrinsics in the frontend around the
  // `co_await` expression and convert them to real lifetime intrinsics here.
  auto *FramePtr = allocateFrameInCaller(Caller, FrameSize, FrameAlign);
  auto NewCBInsertPt = CB->getIterator();
  toolchain::CallBase *NewCB = nullptr;
  SmallVector<Value *, 4> NewArgs;
  NewArgs.append(CB->arg_begin(), CB->arg_end());
  NewArgs.push_back(FramePtr);

  if (auto *CI = dyn_cast<CallInst>(CB)) {
    auto *NewCI = CallInst::Create(NewCallee->getFunctionType(), NewCallee,
                                   NewArgs, "", NewCBInsertPt);
    NewCI->setTailCallKind(CI->getTailCallKind());
    NewCB = NewCI;
  } else if (auto *II = dyn_cast<InvokeInst>(CB)) {
    NewCB = InvokeInst::Create(NewCallee->getFunctionType(), NewCallee,
                               II->getNormalDest(), II->getUnwindDest(),
                               NewArgs, {}, "", NewCBInsertPt);
  } else {
    llvm_unreachable("CallBase should either be Call or Invoke!");
  }

  NewCB->setCalledFunction(NewCallee->getFunctionType(), NewCallee);
  NewCB->setCallingConv(CB->getCallingConv());
  NewCB->setAttributes(CB->getAttributes());
  NewCB->setDebugLoc(CB->getDebugLoc());
  std::copy(CB->bundle_op_info_begin(), CB->bundle_op_info_end(),
            NewCB->bundle_op_info_begin());

  NewCB->removeFnAttr(toolchain::Attribute::CoroElideSafe);
  CB->replaceAllUsesWith(NewCB);

  InlineFunctionInfo IFI;
  InlineResult IR = InlineFunction(*NewCB, IFI);
  if (IR.isSuccess()) {
    CB->eraseFromParent();
  } else {
    NewCB->replaceAllUsesWith(CB);
    NewCB->eraseFromParent();
  }
}

PreservedAnalyses CoroAnnotationElidePass::run(LazyCallGraph::SCC &C,
                                               CGSCCAnalysisManager &AM,
                                               LazyCallGraph &CG,
                                               CGSCCUpdateResult &UR) {
  bool Changed = false;
  CallGraphUpdater CGUpdater;
  CGUpdater.initialize(CG, C, AM, UR);

  auto &FAM =
      AM.getResult<FunctionAnalysisManagerCGSCCProxy>(C, CG).getManager();

  for (LazyCallGraph::Node &N : C) {
    Function *Callee = &N.getFunction();
    Function *NewCallee = Callee->getParent()->getFunction(
        (Callee->getName() + ".noalloc").str());
    if (!NewCallee)
      continue;

    SmallVector<CallBase *, 4> Users;
    for (auto *U : Callee->users()) {
      if (auto *CB = dyn_cast<CallBase>(U)) {
        if (CB->getCalledFunction() == Callee)
          Users.push_back(CB);
      }
    }
    auto FramePtrArgPosition = NewCallee->arg_size() - 1;
    auto FrameSize =
        NewCallee->getParamDereferenceableBytes(FramePtrArgPosition);
    auto FrameAlign =
        NewCallee->getParamAlign(FramePtrArgPosition).valueOrOne();

    auto &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(*Callee);

    for (auto *CB : Users) {
      auto *Caller = CB->getFunction();
      if (!Caller)
        continue;

      bool IsCallerPresplitCoroutine = Caller->isPresplitCoroutine();
      bool HasAttr = CB->hasFnAttr(toolchain::Attribute::CoroElideSafe);
      if (IsCallerPresplitCoroutine && HasAttr) {
        auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(*Caller);

        auto BlockFreq = BFI.getBlockFreq(CB->getParent()).getFrequency();
        auto EntryFreq = BFI.getEntryFreq().getFrequency();
        uint64_t MinFreq =
            static_cast<uint64_t>(EntryFreq * CoroElideBranchRatio);

        if (BlockFreq < MinFreq) {
          ORE.emit([&]() {
            return OptimizationRemarkMissed(
                       DEBUG_TYPE, "CoroAnnotationElideUnlikely", Caller)
                   << "'" << ore::NV("callee", Callee->getName())
                   << "' not elided in '"
                   << ore::NV("caller", Caller->getName())
                   << "' because of low frequency: "
                   << ore::NV("block_freq", BlockFreq)
                   << " (threshold: " << ore::NV("min_freq", MinFreq) << ")";
          });
          continue;
        }

        auto *CallerN = CG.lookup(*Caller);
        auto *CallerC = CallerN ? CG.lookupSCC(*CallerN) : nullptr;
        // If CallerC is nullptr, it means LazyCallGraph hasn't visited Caller
        // yet. Skip the call graph update.
        auto ShouldUpdateCallGraph = !!CallerC;
        processCall(CB, Caller, NewCallee, FrameSize, FrameAlign);

        ORE.emit([&]() {
          return OptimizationRemark(DEBUG_TYPE, "CoroAnnotationElide", Caller)
                 << "'" << ore::NV("callee", Callee->getName())
                 << "' elided in '" << ore::NV("caller", Caller->getName())
                 << "' (block_freq: " << ore::NV("block_freq", BlockFreq)
                 << ")";
        });

        FAM.invalidate(*Caller, PreservedAnalyses::none());
        Changed = true;
        if (ShouldUpdateCallGraph)
          updateCGAndAnalysisManagerForCGSCCPass(CG, *CallerC, *CallerN, AM, UR,
                                                 FAM);

      } else {
        ORE.emit([&]() {
          return OptimizationRemarkMissed(DEBUG_TYPE, "CoroAnnotationElide",
                                          Caller)
                 << "'" << ore::NV("callee", Callee->getName())
                 << "' not elided in '" << ore::NV("caller", Caller->getName())
                 << "' (caller_presplit="
                 << ore::NV("caller_presplit", IsCallerPresplitCoroutine)
                 << ", elide_safe_attr=" << ore::NV("elide_safe_attr", HasAttr)
                 << ")";
        });
      }
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
