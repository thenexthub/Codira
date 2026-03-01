//===- JumpTableToSwitch.cpp ----------------------------------------------===//
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

#include "vm/core/Transforms/Scalar/JumpTableToSwitch.h"
#include "vm/core/ADT/DenseSet.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/ConstantFolding.h"
#include "vm/core/Analysis/CtxProfAnalysis.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Analysis/PostDominators.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/ProfDataUtils.h"
#include "vm/core/ProfileData/InstrProf.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include <limits>

using namespace vm::core;

static cl::opt<unsigned>
    JumpTableSizeThreshold("jump-table-to-switch-size-threshold", cl::Hidden,
                           cl::desc("Only split jump tables with size less or "
                                    "equal than JumpTableSizeThreshold."),
                           cl::init(10));

// TODO: Consider adding a cost model for profitability analysis of this
// transformation. Currently we replace a jump table with a switch if all the
// functions in the jump table are smaller than the provided threshold.
static cl::opt<unsigned> FunctionSizeThreshold(
    "jump-table-to-switch-function-size-threshold", cl::Hidden,
    cl::desc("Only split jump tables containing functions whose sizes are less "
             "or equal than this threshold."),
    cl::init(50));

namespace vm::core {
extern cl::opt<bool> ProfcheckDisableMetadataFixes;
} // end namespace vm::core

#define DEBUG_TYPE "jump-table-to-switch"

namespace {
struct JumpTableTy {
  Value *Index;
  SmallVector<Function *, 10> Funcs;
};
} // anonymous namespace

static std::optional<JumpTableTy> parseJumpTable(GetElementPtrInst *GEP,
                                                 PointerType *PtrTy) {
  Constant *Ptr = dyn_cast<Constant>(GEP->getPointerOperand());
  if (!Ptr)
    return std::nullopt;

  GlobalVariable *GV = dyn_cast<GlobalVariable>(Ptr);
  if (!GV || !GV->isConstant() || !GV->hasDefinitiveInitializer())
    return std::nullopt;

  Function &F = *GEP->getParent()->getParent();
  const DataLayout &DL = F.getDataLayout();
  const unsigned BitWidth =
      DL.getIndexSizeInBits(GEP->getPointerAddressSpace());
  SmallMapVector<Value *, APInt, 4> VariableOffsets;
  APInt ConstantOffset(BitWidth, 0);
  if (!GEP->collectOffset(DL, BitWidth, VariableOffsets, ConstantOffset))
    return std::nullopt;
  if (VariableOffsets.size() != 1)
    return std::nullopt;
  // TODO: consider supporting more general patterns
  if (!ConstantOffset.isZero())
    return std::nullopt;
  APInt StrideBytes = VariableOffsets.front().second;
  const uint64_t JumpTableSizeBytes = DL.getTypeAllocSize(GV->getValueType());
  if (JumpTableSizeBytes % StrideBytes.getZExtValue() != 0)
    return std::nullopt;
  const uint64_t N = JumpTableSizeBytes / StrideBytes.getZExtValue();
  if (N > JumpTableSizeThreshold)
    return std::nullopt;

  JumpTableTy JumpTable;
  JumpTable.Index = VariableOffsets.front().first;
  JumpTable.Funcs.reserve(N);
  for (uint64_t Index = 0; Index < N; ++Index) {
    // ConstantOffset is zero.
    APInt Offset = Index * StrideBytes;
    Constant *C =
        ConstantFoldLoadFromConst(GV->getInitializer(), PtrTy, Offset, DL);
    auto *Func = dyn_cast_or_null<Function>(C);
    if (!Func || Func->isDeclaration() ||
        Func->getInstructionCount() > FunctionSizeThreshold)
      return std::nullopt;
    JumpTable.Funcs.push_back(Func);
  }
  return JumpTable;
}

static BasicBlock *
expandToSwitch(CallBase *CB, const JumpTableTy &JT, DomTreeUpdater &DTU,
               OptimizationRemarkEmitter &ORE,
               toolchain::function_ref<GlobalValue::GUID(const Function &)>
                   GetGuidForFunction) {
  const bool IsVoid = CB->getType() == Type::getVoidTy(CB->getContext());

  SmallVector<DominatorTree::UpdateType, 8> DTUpdates;
  BasicBlock *BB = CB->getParent();
  BasicBlock *Tail = SplitBlock(BB, CB, &DTU, nullptr, nullptr,
                                BB->getName() + Twine(".tail"));
  DTUpdates.push_back({DominatorTree::Delete, BB, Tail});
  BB->getTerminator()->eraseFromParent();

  Function &F = *BB->getParent();
  BasicBlock *BBUnreachable = BasicBlock::Create(
      F.getContext(), "default.switch.case.unreachable", &F, Tail);
  IRBuilder<> BuilderUnreachable(BBUnreachable);
  BuilderUnreachable.CreateUnreachable();

  IRBuilder<> Builder(BB);
  SwitchInst *Switch = Builder.CreateSwitch(JT.Index, BBUnreachable);
  DTUpdates.push_back({DominatorTree::Insert, BB, BBUnreachable});

  IRBuilder<> BuilderTail(CB);
  PHINode *PHI =
      IsVoid ? nullptr : BuilderTail.CreatePHI(CB->getType(), JT.Funcs.size());
  const auto *ProfMD = CB->getMetadata(LLVMContext::MD_prof);

  SmallVector<uint64_t> BranchWeights;
  DenseMap<GlobalValue::GUID, uint64_t> GuidToCounter;
  const bool HadProfile = isValueProfileMD(ProfMD);
  if (HadProfile) {
    // The assumptions, coming in, are that the functions in JT.Funcs are
    // defined in this module (from parseJumpTable).
    assert(toolchain::all_of(
        JT.Funcs, [](const Function *F) { return F && !F->isDeclaration(); }));
    BranchWeights.reserve(JT.Funcs.size() + 1);
    // The first is the default target, which is the unreachable block created
    // above.
    BranchWeights.push_back(0U);
    uint64_t TotalCount = 0;
    auto Targets = getValueProfDataFromInst(
        *CB, InstrProfValueKind::IPVK_IndirectCallTarget,
        std::numeric_limits<uint32_t>::max(), TotalCount);

    for (const auto &[G, C] : Targets) {
      [[maybe_unused]] auto It = GuidToCounter.insert({G, C});
      assert(It.second);
    }
  }
  for (auto [Index, Func] : toolchain::enumerate(JT.Funcs)) {
    BasicBlock *B = BasicBlock::Create(Func->getContext(),
                                       "call." + Twine(Index), &F, Tail);
    DTUpdates.push_back({DominatorTree::Insert, BB, B});
    DTUpdates.push_back({DominatorTree::Insert, B, Tail});

    CallBase *Call = cast<CallBase>(CB->clone());
    // The MD_prof metadata (VP kind), if it existed, can be dropped, it doesn't
    // make sense on a direct call. Note that the values are used for the branch
    // weights of the switch.
    Call->setMetadata(LLVMContext::MD_prof, nullptr);
    Call->setCalledFunction(Func);
    Call->insertInto(B, B->end());
    Switch->addCase(
        cast<ConstantInt>(ConstantInt::get(JT.Index->getType(), Index)), B);
    GlobalValue::GUID FctID = GetGuidForFunction(*Func);
    // It'd be OK to _not_ find target functions in GuidToCounter, e.g. suppose
    // just some of the jump targets are taken (for the given profile).
    BranchWeights.push_back(FctID == 0U ? 0U
                                        : GuidToCounter.lookup_or(FctID, 0U));
    BranchInst::Create(Tail, B);
    if (PHI)
      PHI->addIncoming(Call, B);
  }
  DTU.applyUpdates(DTUpdates);
  ORE.emit([&]() {
    return OptimizationRemark(DEBUG_TYPE, "ReplacedJumpTableWithSwitch", CB)
           << "expanded indirect call into switch";
  });
  if (HadProfile && !ProfcheckDisableMetadataFixes) {
    // At least one of the targets must've been taken.
    assert(toolchain::any_of(BranchWeights, not_equal_to(0)));
    setBranchWeights(*Switch, downscaleWeights(BranchWeights),
                     /*IsExpected=*/false);
  } else
    setExplicitlyUnknownBranchWeights(*Switch, DEBUG_TYPE);
  if (PHI)
    CB->replaceAllUsesWith(PHI);
  CB->eraseFromParent();
  return Tail;
}

PreservedAnalyses JumpTableToSwitchPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  OptimizationRemarkEmitter &ORE =
      AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  DominatorTree *DT = AM.getCachedResult<DominatorTreeAnalysis>(F);
  PostDominatorTree *PDT = AM.getCachedResult<PostDominatorTreeAnalysis>(F);
  DomTreeUpdater DTU(DT, PDT, DomTreeUpdater::UpdateStrategy::Lazy);
  bool Changed = false;
  auto FuncToGuid = [&](const Function &Fct) {
    if (Fct.getMetadata(AssignGUIDPass::GUIDMetadataName))
      return AssignGUIDPass::getGUID(Fct);

    return Function::getGUIDAssumingExternalLinkage(getIRPGOFuncName(F, InLTO));
  };

  for (BasicBlock &BB : make_early_inc_range(F)) {
    BasicBlock *CurrentBB = &BB;
    while (CurrentBB) {
      BasicBlock *SplittedOutTail = nullptr;
      for (Instruction &I : make_early_inc_range(*CurrentBB)) {
        auto *Call = dyn_cast<CallInst>(&I);
        if (!Call || Call->getCalledFunction() || Call->isMustTailCall())
          continue;
        auto *L = dyn_cast<LoadInst>(Call->getCalledOperand());
        // Skip atomic or volatile loads.
        if (!L || !L->isSimple())
          continue;
        auto *GEP = dyn_cast<GetElementPtrInst>(L->getPointerOperand());
        if (!GEP)
          continue;
        auto *PtrTy = dyn_cast<PointerType>(L->getType());
        assert(PtrTy && "call operand must be a pointer");
        std::optional<JumpTableTy> JumpTable = parseJumpTable(GEP, PtrTy);
        if (!JumpTable)
          continue;
        SplittedOutTail =
            expandToSwitch(Call, *JumpTable, DTU, ORE, FuncToGuid);
        Changed = true;
        break;
      }
      CurrentBB = SplittedOutTail ? SplittedOutTail : nullptr;
    }
  }

  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  if (DT)
    PA.preserve<DominatorTreeAnalysis>();
  if (PDT)
    PA.preserve<PostDominatorTreeAnalysis>();
  return PA;
}
