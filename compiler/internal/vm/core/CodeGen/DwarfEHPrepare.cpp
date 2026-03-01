//===- DwarfEHPrepare - Prepare exception handling for code generation ----===//
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
// This pass mulches exception handling code into a form adapted to code
// generation. Required if using dwarf exception handling.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/DwarfEHPrepare.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/CFG.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/EHPersonalities.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/Triple.h"
#include "vm/core/Transforms/Utils/Local.h"
#include <cstddef>

using namespace vm::core;

#define DEBUG_TYPE "dwarf-eh-prepare"

STATISTIC(NumResumesLowered, "Number of resume calls lowered");
STATISTIC(NumCleanupLandingPadsUnreachable,
          "Number of cleanup landing pads found unreachable");
STATISTIC(NumCleanupLandingPadsRemaining,
          "Number of cleanup landing pads remaining");
STATISTIC(NumNoUnwind, "Number of functions with nounwind");
STATISTIC(NumUnwind, "Number of functions with unwind");

namespace {

class DwarfEHPrepare {
  CodeGenOptLevel OptLevel;

  Function &F;
  const TargetLowering &TLI;
  DomTreeUpdater *DTU;
  const TargetTransformInfo *TTI;
  const Triple &TargetTriple;

  /// Return the exception object from the value passed into
  /// the 'resume' instruction (typically an aggregate). Clean up any dead
  /// instructions, including the 'resume' instruction.
  Value *GetExceptionObject(ResumeInst *RI);

  /// Replace resumes that are not reachable from a cleanup landing pad with
  /// unreachable and then simplify those blocks.
  size_t
  pruneUnreachableResumes(SmallVectorImpl<ResumeInst *> &Resumes,
                          SmallVectorImpl<LandingPadInst *> &CleanupLPads);

  /// Convert the ResumeInsts that are still present
  /// into calls to the appropriate _Unwind_Resume function.
  bool InsertUnwindResumeCalls();

public:
  DwarfEHPrepare(CodeGenOptLevel OptLevel_, Function &F_,
                 const TargetLowering &TLI_, DomTreeUpdater *DTU_,
                 const TargetTransformInfo *TTI_, const Triple &TargetTriple_)
      : OptLevel(OptLevel_), F(F_), TLI(TLI_), DTU(DTU_), TTI(TTI_),
        TargetTriple(TargetTriple_) {}

  bool run();
};

} // namespace

Value *DwarfEHPrepare::GetExceptionObject(ResumeInst *RI) {
  Value *V = RI->getOperand(0);
  Value *ExnObj = nullptr;
  InsertValueInst *SelIVI = dyn_cast<InsertValueInst>(V);
  LoadInst *SelLoad = nullptr;
  InsertValueInst *ExcIVI = nullptr;
  bool EraseIVIs = false;

  if (SelIVI) {
    if (SelIVI->getNumIndices() == 1 && *SelIVI->idx_begin() == 1) {
      ExcIVI = dyn_cast<InsertValueInst>(SelIVI->getOperand(0));
      if (ExcIVI && isa<UndefValue>(ExcIVI->getOperand(0)) &&
          ExcIVI->getNumIndices() == 1 && *ExcIVI->idx_begin() == 0) {
        ExnObj = ExcIVI->getOperand(1);
        SelLoad = dyn_cast<LoadInst>(SelIVI->getOperand(1));
        EraseIVIs = true;
      }
    }
  }

  if (!ExnObj)
    ExnObj = ExtractValueInst::Create(RI->getOperand(0), 0, "exn.obj",
                                      RI->getIterator());

  RI->eraseFromParent();

  if (EraseIVIs) {
    if (SelIVI->use_empty())
      SelIVI->eraseFromParent();
    if (ExcIVI->use_empty())
      ExcIVI->eraseFromParent();
    if (SelLoad && SelLoad->use_empty())
      SelLoad->eraseFromParent();
  }

  return ExnObj;
}

size_t DwarfEHPrepare::pruneUnreachableResumes(
    SmallVectorImpl<ResumeInst *> &Resumes,
    SmallVectorImpl<LandingPadInst *> &CleanupLPads) {
  assert(DTU && "Should have DomTreeUpdater here.");

  BitVector ResumeReachable(Resumes.size());
  size_t ResumeIndex = 0;
  for (auto *RI : Resumes) {
    for (auto *LP : CleanupLPads) {
      if (isPotentiallyReachable(LP, RI, nullptr, &DTU->getDomTree())) {
        ResumeReachable.set(ResumeIndex);
        break;
      }
    }
    ++ResumeIndex;
  }

  // If everything is reachable, there is no change.
  if (ResumeReachable.all())
    return Resumes.size();

  LLVMContext &Ctx = F.getContext();

  // Otherwise, insert unreachable instructions and call simplifycfg.
  size_t ResumesLeft = 0;
  for (size_t I = 0, E = Resumes.size(); I < E; ++I) {
    ResumeInst *RI = Resumes[I];
    if (ResumeReachable[I]) {
      Resumes[ResumesLeft++] = RI;
    } else {
      BasicBlock *BB = RI->getParent();
      new UnreachableInst(Ctx, RI->getIterator());
      RI->eraseFromParent();
      simplifyCFG(BB, *TTI, DTU);
    }
  }
  Resumes.resize(ResumesLeft);
  return ResumesLeft;
}

bool DwarfEHPrepare::InsertUnwindResumeCalls() {
  SmallVector<ResumeInst *, 16> Resumes;
  SmallVector<LandingPadInst *, 16> CleanupLPads;
  if (F.doesNotThrow())
    NumNoUnwind++;
  else
    NumUnwind++;
  for (BasicBlock &BB : F) {
    if (auto *RI = dyn_cast<ResumeInst>(BB.getTerminator()))
      Resumes.push_back(RI);
    if (auto *LP = BB.getLandingPadInst())
      if (LP->isCleanup())
        CleanupLPads.push_back(LP);
  }

  NumCleanupLandingPadsRemaining += CleanupLPads.size();

  if (Resumes.empty())
    return false;

  // Check the personality, don't do anything if it's scope-based.
  EHPersonality Pers = classifyEHPersonality(F.getPersonalityFn());
  if (isScopedEHPersonality(Pers))
    return false;

  LLVMContext &Ctx = F.getContext();

  size_t ResumesLeft = Resumes.size();
  if (OptLevel != CodeGenOptLevel::None) {
    ResumesLeft = pruneUnreachableResumes(Resumes, CleanupLPads);
#if LLVM_ENABLE_STATS
    unsigned NumRemainingLPs = 0;
    for (BasicBlock &BB : F) {
      if (auto *LP = BB.getLandingPadInst())
        if (LP->isCleanup())
          NumRemainingLPs++;
    }
    NumCleanupLandingPadsUnreachable += CleanupLPads.size() - NumRemainingLPs;
    NumCleanupLandingPadsRemaining -= CleanupLPads.size() - NumRemainingLPs;
#endif
  }

  if (ResumesLeft == 0)
    return true; // We pruned them all.

  // RewindFunction - _Unwind_Resume or the target equivalent.
  FunctionCallee RewindFunction;
  CallingConv::ID RewindFunctionCallingConv;
  FunctionType *FTy;
  const char *RewindName;
  bool DoesRewindFunctionNeedExceptionObject;

  if ((Pers == EHPersonality::GNU_CXX || Pers == EHPersonality::GNU_CXX_SjLj) &&
      TargetTriple.isTargetEHABICompatible()) {
    RewindName = TLI.getLibcallName(RTLIB::CXA_END_CLEANUP);
    FTy = FunctionType::get(Type::getVoidTy(Ctx), false);
    RewindFunctionCallingConv =
        TLI.getLibcallCallingConv(RTLIB::CXA_END_CLEANUP);
    DoesRewindFunctionNeedExceptionObject = false;
  } else {
    RewindName = TLI.getLibcallName(RTLIB::UNWIND_RESUME);
    FTy = FunctionType::get(Type::getVoidTy(Ctx), PointerType::getUnqual(Ctx),
                            false);
    RewindFunctionCallingConv = TLI.getLibcallCallingConv(RTLIB::UNWIND_RESUME);
    DoesRewindFunctionNeedExceptionObject = true;
  }
  RewindFunction = F.getParent()->getOrInsertFunction(RewindName, FTy);

  // Create the basic block where the _Unwind_Resume call will live.
  if (ResumesLeft == 1) {
    // Instead of creating a new BB and PHI node, just append the call to
    // _Unwind_Resume to the end of the single resume block.
    ResumeInst *RI = Resumes.front();
    BasicBlock *UnwindBB = RI->getParent();
    Value *ExnObj = GetExceptionObject(RI);
    toolchain::SmallVector<Value *, 1> RewindFunctionArgs;
    if (DoesRewindFunctionNeedExceptionObject)
      RewindFunctionArgs.push_back(ExnObj);

    // Call the rewind function.
    CallInst *CI =
        CallInst::Create(RewindFunction, RewindFunctionArgs, "", UnwindBB);
    // The verifier requires that all calls of debug-info-bearing functions
    // from debug-info-bearing functions have a debug location (for inlining
    // purposes). Assign a dummy location to satisfy the constraint.
    Function *RewindFn = dyn_cast<Function>(RewindFunction.getCallee());
    if (RewindFn && RewindFn->getSubprogram())
      if (DISubprogram *SP = F.getSubprogram())
        CI->setDebugLoc(DILocation::get(SP->getContext(), 0, 0, SP));
    CI->setCallingConv(RewindFunctionCallingConv);

    // We never expect _Unwind_Resume to return.
    CI->setDoesNotReturn();
    new UnreachableInst(Ctx, UnwindBB);
    return true;
  }

  std::vector<DominatorTree::UpdateType> Updates;
  Updates.reserve(Resumes.size());

  toolchain::SmallVector<Value *, 1> RewindFunctionArgs;

  BasicBlock *UnwindBB = BasicBlock::Create(Ctx, "unwind_resume", &F);
  PHINode *PN = PHINode::Create(PointerType::getUnqual(Ctx), ResumesLeft,
                                "exn.obj", UnwindBB);

  // Extract the exception object from the ResumeInst and add it to the PHI node
  // that feeds the _Unwind_Resume call.
  for (ResumeInst *RI : Resumes) {
    BasicBlock *Parent = RI->getParent();
    BranchInst::Create(UnwindBB, Parent);
    Updates.push_back({DominatorTree::Insert, Parent, UnwindBB});

    Value *ExnObj = GetExceptionObject(RI);
    PN->addIncoming(ExnObj, Parent);

    ++NumResumesLowered;
  }

  if (DoesRewindFunctionNeedExceptionObject)
    RewindFunctionArgs.push_back(PN);

  // Call the function.
  CallInst *CI =
      CallInst::Create(RewindFunction, RewindFunctionArgs, "", UnwindBB);
  // The verifier requires that all calls of debug-info-bearing functions
  // from debug-info-bearing functions have a debug location (for inlining
  // purposes). Assign a dummy location to satisfy the constraint.
  Function *RewindFn = dyn_cast<Function>(RewindFunction.getCallee());
  if (RewindFn && RewindFn->getSubprogram())
    if (DISubprogram *SP = F.getSubprogram())
      CI->setDebugLoc(DILocation::get(SP->getContext(), 0, 0, SP));
  CI->setCallingConv(RewindFunctionCallingConv);

  // We never expect _Unwind_Resume to return.
  CI->setDoesNotReturn();
  new UnreachableInst(Ctx, UnwindBB);

  if (DTU)
    DTU->applyUpdates(Updates);

  return true;
}

bool DwarfEHPrepare::run() {
  bool Changed = InsertUnwindResumeCalls();

  return Changed;
}

static bool prepareDwarfEH(CodeGenOptLevel OptLevel, Function &F,
                           const TargetLowering &TLI, DominatorTree *DT,
                           const TargetTransformInfo *TTI,
                           const Triple &TargetTriple) {
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  return DwarfEHPrepare(OptLevel, F, TLI, DT ? &DTU : nullptr, TTI,
                        TargetTriple)
      .run();
}

namespace {

class DwarfEHPrepareLegacyPass : public FunctionPass {

  CodeGenOptLevel OptLevel;

public:
  static char ID; // Pass identification, replacement for typeid.

  DwarfEHPrepareLegacyPass(CodeGenOptLevel OptLevel = CodeGenOptLevel::Default)
      : FunctionPass(ID), OptLevel(OptLevel) {}

  bool runOnFunction(Function &F) override {
    const TargetMachine &TM =
        getAnalysis<TargetPassConfig>().getTM<TargetMachine>();
    const TargetLowering &TLI = *TM.getSubtargetImpl(F)->getTargetLowering();
    DominatorTree *DT = nullptr;
    const TargetTransformInfo *TTI = nullptr;
    if (auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>())
      DT = &DTWP->getDomTree();
    if (OptLevel != CodeGenOptLevel::None) {
      if (!DT)
        DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
      TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    }
    return prepareDwarfEH(OptLevel, F, TLI, DT, TTI, TM.getTargetTriple());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    if (OptLevel != CodeGenOptLevel::None) {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<TargetTransformInfoWrapperPass>();
    }
    AU.addPreserved<DominatorTreeWrapperPass>();
  }

  StringRef getPassName() const override {
    return "Exception handling preparation";
  }
};

} // end anonymous namespace

PreservedAnalyses DwarfEHPreparePass::run(Function &F,
                                          FunctionAnalysisManager &FAM) {
  const auto &TLI = *TM->getSubtargetImpl(F)->getTargetLowering();
  auto *DT = FAM.getCachedResult<DominatorTreeAnalysis>(F);
  const TargetTransformInfo *TTI = nullptr;
  auto OptLevel = TM->getOptLevel();
  if (OptLevel != CodeGenOptLevel::None) {
    if (!DT)
      DT = &FAM.getResult<DominatorTreeAnalysis>(F);
    TTI = &FAM.getResult<TargetIRAnalysis>(F);
  }
  bool Changed =
      prepareDwarfEH(OptLevel, F, TLI, DT, TTI, TM->getTargetTriple());

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}

char DwarfEHPrepareLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(DwarfEHPrepareLegacyPass, DEBUG_TYPE,
                      "Prepare DWARF exceptions", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(DwarfEHPrepareLegacyPass, DEBUG_TYPE,
                    "Prepare DWARF exceptions", false, false)

FunctionPass *toolchain::createDwarfEHPass(CodeGenOptLevel OptLevel) {
  return new DwarfEHPrepareLegacyPass(OptLevel);
}
