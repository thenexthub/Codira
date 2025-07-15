//===--- SILSROA.cpp - Scalar Replacement of Aggregates  ------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Change aggregate values into scalar values. Currently it takes every
// allocation and chops them up into their smallest non-captured pieces.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-sroa"
#include "language/Basic/Assertions.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Range.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/Projection.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILUndef.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Debug.h"
#include <type_traits>

using namespace language;

STATISTIC(NumEscapingAllocas, "Number of aggregate allocas not chopped up "
          "due to uses.");
STATISTIC(NumChoppedAllocas, "Number of chopped up aggregate allocas.");
STATISTIC(NumUnhandledAllocas, "Number of non struct, tuple allocas.");

namespace {

class SROAMemoryUseAnalyzer {
  // The allocation we are analyzing.
  AllocStackInst *AI;

  // Loads from AI.
  toolchain::SmallVector<LoadInst *, 4> Loads;
  // Stores to AI.
  toolchain::SmallVector<StoreInst *, 4> Stores;
  // Instructions which extract from aggregates.
  toolchain::SmallVector<SingleValueInstruction *, 4> ExtractInsts;

  // TupleType if we are visiting a tuple.
  TupleType *TT = nullptr;
  // StructDecl if we are visiting a struct.
  StructDecl *SD = nullptr;
public:
  SROAMemoryUseAnalyzer(AllocStackInst *AI) : AI(AI) {
    assert(AI && "AI should never be null here.");
  }

  bool analyze();
  void chopUpAlloca(std::vector<AllocStackInst *> &Worklist);

private:
  SILValue createAgg(SILBuilder &B, SILLocation Loc, SILType Ty,
                     ArrayRef<SILValue> Elements);
  unsigned getEltNoForProjection(SILInstruction *Inst);
  void createAllocas(toolchain::SmallVector<AllocStackInst *, 4> &NewAllocations);
};

} // end anonymous namespace

SILValue 
SROAMemoryUseAnalyzer::createAgg(SILBuilder &B, SILLocation Loc,
                                 SILType Ty,
                                 ArrayRef<SILValue> Elements) {
  if (TT)
    return B.createTuple(Loc, Ty, Elements);

  assert(SD && "SD must not be null here since it or TT must be set to call"
         " this method.");
  return B.createStruct(Loc, Ty, Elements);
}

unsigned SROAMemoryUseAnalyzer::getEltNoForProjection(SILInstruction *Inst) {
  if (TT)
    return cast<TupleElementAddrInst>(Inst)->getFieldIndex();

  assert(SD && "SD should not be null since either it or TT must be set at "
         "this point.");
  StructElementAddrInst *SEA = cast<StructElementAddrInst>(Inst);
  VarDecl *Field = SEA->getField();
  unsigned EltNo = 0;
  for (auto *D : SD->getStoredProperties()) {
    if (D == Field)
      return EltNo;
    ++EltNo;
  }
  toolchain_unreachable("Unknown field.");
}

bool SROAMemoryUseAnalyzer::analyze() {
  // We only know how to split structs and tuples... So if we have a scalar or a
  // different sort of aggregate, bail.
  SILType Type = AI->getType();

  TT = Type.getAs<TupleType>();
  SD = Type.getStructOrBoundGenericStruct();
  bool HasUnrefField = AI->getElementType().aggregateHasUnreferenceableStorage();

  // Check that the allocated type is a struct or a tuple and that there are
  // no unreferenced fields.
  if (HasUnrefField || (!TT && !SD)) {
    ++NumUnhandledAllocas;
    return false;
  }

  bool hasBenefit = false;

  // Go through uses of the memory allocation of AI...
  for (auto *Operand : getNonDebugUses(SILValue(AI))) {
    SILInstruction *User = Operand->getUser();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Visiting use: " << *User);

    // If we store the alloca pointer, we cannot analyze its uses so bail...
    // It is ok if we store into the alloca pointer though.
    if (auto *SI = dyn_cast<StoreInst>(User)) {
      if (SI->getDest() == AI) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found a store into the "
                                   "projection.\n");
        Stores.push_back(SI);
        SILValue Src = SI->getSrc();
        if (isa<StructInst>(Src) || isa<TupleInst>(Src))
          hasBenefit = true;
        continue;
      } else {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found a store of the "
                                   "projection pointer. Escapes!.\n");
        ++NumEscapingAllocas;
        return false;
      }
    }

    // If the use is a load, keep track of it for splitting later...
    if (auto *LI = dyn_cast<LoadInst>(User)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found a load of the projection.\n");
      Loads.push_back(LI);
      for (auto useIter = LI->use_begin(), End = LI->use_end();
           !hasBenefit && useIter != End; ++useIter) {
        hasBenefit = (isa<StructExtractInst>(useIter->get()) ||
                      isa<TupleExtractInst>(useIter->get()));
      }
      continue;
    }

    // If the use is a struct_element_addr, add it to the worklist so we check
    // if it or one of its descendants escape.
    if (auto *ASI = dyn_cast<StructElementAddrInst>(User)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found a struct subprojection!\n");
      ExtractInsts.push_back(ASI);
      hasBenefit = true;
      continue;
    }

    // If the use is a tuple_element_addr, add it to the worklist so we check
    // if it or one of its descendants escape.
    if (auto *TSI = dyn_cast<TupleElementAddrInst>(User)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found a tuple subprojection!\n");
      ExtractInsts.push_back(TSI);
      hasBenefit = true;
      continue;
    }

    if (isa<DeallocStackInst>(User)) {
      // We can ignore the dealloc_stack.
      continue;
    }
    
    // Otherwise we do not understand this instruction, so bail.
    TOOLCHAIN_DEBUG(toolchain::dbgs() <<"        Found unknown user, pointer escapes!\n");
    ++NumEscapingAllocas;
    return false;
  }

  // Analysis was successful. We can break up this allocation!
  ++NumChoppedAllocas;
  return hasBenefit;
}

void
SROAMemoryUseAnalyzer::
createAllocas(toolchain::SmallVector<AllocStackInst *, 4> &NewAllocations) {
  SILBuilderWithScope B(AI);
  SILType Type = AI->getType().getObjectType();
  std::optional<SILDebugVariable> AIDebugVarInfo =
      SILDebugVariable::createFromAllocation(AI);

  // Intentionally dropping the debug location.
  SILLocation Loc = RegularLocation::getAutoGeneratedLocation();
  if (TT) {
    for (unsigned EltNo : indices(TT->getElementTypes())) {
      std::optional<SILDebugVariable> NewDebugVarInfo = AIDebugVarInfo;
      if (NewDebugVarInfo)
        NewDebugVarInfo->DIExpr.append(
            SILDebugInfoExpression::createTupleFragment(TT, EltNo));
      SILType EltTy = Type.getTupleElementType(EltNo);
      NewAllocations.push_back(B.createAllocStack(
          Loc, EltTy, NewDebugVarInfo, AI->hasDynamicLifetime(),
          AI->isLexical()));
    }
  } else {
    assert(SD && "SD should not be null since either it or TT must be set at "
           "this point.");
    SILModule &M = AI->getModule();
    for (VarDecl *VD : SD->getStoredProperties()) {
      std::optional<SILDebugVariable> NewDebugVarInfo = AIDebugVarInfo;
      if (NewDebugVarInfo)
        NewDebugVarInfo->DIExpr.append(
            SILDebugInfoExpression::createFragment(VD));
      NewAllocations.push_back(B.createAllocStack(
          Loc, Type.getFieldType(VD, M, TypeExpansionContext(B.getFunction())),
          NewDebugVarInfo, AI->hasDynamicLifetime(), AI->isLexical()));
    }
  }
  if (AIDebugVarInfo && NewAllocations.empty()) {
    // Don't eliminate empty structs, we can use undef as there is no data
    B.createDebugValue(Loc, SILUndef::get(AI), *AIDebugVarInfo);
  }
}

void SROAMemoryUseAnalyzer::chopUpAlloca(std::vector<AllocStackInst *> &Worklist) {
  // Create allocations for this instruction.
  toolchain::SmallVector<AllocStackInst *, 4> NewAllocations;
  createAllocas(NewAllocations);
  // Add the new allocations to the worklist for recursive processing.
  //
  // TODO: Change this into an assert. For some reason I am running into compile
  // issues when I try it now.
  for (auto *AI : NewAllocations)
    Worklist.push_back(AI);

  // Change any aggregate loads into field loads + aggregate structure.
  for (auto *LI : Loads) {
    SILBuilderWithScope B(LI);
    toolchain::SmallVector<SILValue, 4> Elements;
    for (auto *NewAI : NewAllocations) {
      Elements.push_back(B.emitLoadValueOperation(LI->getLoc(), NewAI,
                                                  LI->getOwnershipQualifier()));
    }
    SILValue Agg = createAgg(B, LI->getLoc(), LI->getType().getObjectType(),
                             Elements);
    LI->replaceAllUsesWith(Agg);
    LI->eraseFromParent();
  }

  // Change any aggregate stores into extracts + field stores.
  for (auto *SI : Stores) {
    SILBuilderWithScope builder(SI);
    SmallVector<SILValue, 8> destructured;
    builder.emitDestructureValueOperation(SI->getLoc(), SI->getSrc(),
                                          destructured);
    for (unsigned eltNo : indices(NewAllocations)) {
      builder.emitStoreValueOperation(SI->getLoc(), destructured[eltNo],
                                      NewAllocations[eltNo],
                                      SI->getOwnershipQualifier());
    }
    SI->eraseFromParent();
  }

  // Forward any field extracts to the new allocation.
  for (auto *Ext : ExtractInsts) {
    AllocStackInst *NewValue = NewAllocations[getEltNoForProjection(Ext)];
    Ext->replaceAllUsesWith(NewValue);
    Ext->eraseFromParent();
  }

  // Find all dealloc instructions for AI and then chop them up.
  toolchain::SmallVector<SILInstruction *, 4> ToRemove;
  for (auto *Operand : getNonDebugUses(SILValue(AI))) {
    SILInstruction *User = Operand->getUser();
    SILBuilderWithScope B(User);

    // If the use is a DSI, add it to our memory analysis so that if we can chop
    // up allocas, we also chop up the relevant dealloc stack insts.
    if (auto *DSI = dyn_cast<DeallocStackInst>(User)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Found DeallocStackInst!\n");
      // Create the allocations in reverse order.
      for (auto *NewAI : toolchain::reverse(NewAllocations))
        B.createDeallocStack(DSI->getLoc(), SILValue(NewAI));
      ToRemove.push_back(DSI);
    }
  }

  for (auto *Operand : getDebugUses(SILValue(AI))) {
    SILInstruction *User = Operand->getUser();
    auto *DVI = dyn_cast<DebugValueInst>(User);
    assert(DVI && "getDebugUses should only return DebugValueInst");
    SILBuilder B(DVI, DVI->getDebugScope());
    std::optional<SILDebugVariable> DVIVarInfo = DVI->getVarInfo();
    assert(DVIVarInfo && "debug_value without debug info");

    for (size_t i : indices(NewAllocations)) {
      auto *NewAI = NewAllocations[i];
      SILDebugVariable VarInfo = *DVIVarInfo;
      if (TT) {
        VarInfo.DIExpr.append(
          SILDebugInfoExpression::createTupleFragment(TT, i));
      } else {
        VarInfo.DIExpr.append(
          SILDebugInfoExpression::createFragment(SD->getStoredProperties()[i]));
      }
      if (!VarInfo.Type)
        VarInfo.Type = AI->getElementType();
      B.createDebugValue(DVI->getLoc(), NewAI, VarInfo);
    }
    if (NewAllocations.empty()) {
      // Don't eliminate empty structs, we can use undef as there is no data
      B.createDebugValue(DVI->getLoc(), SILUndef::get(AI), *DVIVarInfo);
    }
    ToRemove.push_back(DVI);
  }

  // Remove the old DeallocStackInst/DebugValueInst instructions.
  for (auto *DSI : ToRemove) {
      DSI->eraseFromParent();
  }

  AI->eraseFromParent();
}

/// Returns true, if values of \ty should be ignored, because \p ty is known
/// by a high-level SIL optimization. Values of that type must not be split
/// so that those high-level optimizations can analyze the code.
static bool isSemanticType(ASTContext &ctxt, SILType ty) {
  if (ty.getASTType()->isString()) {
    return true;
  }

  return false;
}

static bool runSROAOnFunction(SILFunction &Fn, bool splitSemanticTypes) {
  std::vector<AllocStackInst *> Worklist;
  bool Changed = false;
  ASTContext &ctxt = Fn.getModule().getASTContext();

  // For each basic block BB in Fn...
  for (auto &BB : Fn)
    // For each instruction in BB...
    for (auto &I : BB)
      // If the instruction is an alloc stack inst, add it to the worklist.
      if (auto *AI = dyn_cast<AllocStackInst>(&I)) {
        if (!splitSemanticTypes && isSemanticType(ctxt, AI->getElementType()))
          continue;

        if (shouldExpand(Fn.getModule(), AI->getElementType()))
          Worklist.push_back(AI);
      }

  while (!Worklist.empty()) {
    AllocStackInst *AI = Worklist.back();
    Worklist.pop_back();

    SROAMemoryUseAnalyzer Analyzer(AI);

    if (!Analyzer.analyze())
      continue;

    Changed = true;
    Analyzer.chopUpAlloca(Worklist);
  }
  return Changed;
}

namespace {
class SILSROA : public SILFunctionTransform {

  bool splitSemanticTypes;
  
public:
  SILSROA(bool splitSemanticTypes) : splitSemanticTypes(splitSemanticTypes) { }

  /// The entry point to the transformation.
  void run() override {
    SILFunction *F = getFunction();

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "***** SROA on function: " << F->getName()
                            << " *****\n");

    if (runSROAOnFunction(*F, splitSemanticTypes))
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }

};
} // end anonymous namespace


SILTransform *language::createSROA() {
  return new SILSROA(/*splitSemanticTypes*/ true);
}

SILTransform *language::createEarlySROA() {
  return new SILSROA(/*splitSemanticTypes*/ false);
}
