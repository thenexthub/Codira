//===- TypeMetadataUtils.cpp - Utilities related to type metadata ---------===//
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
// This file contains functions that make it easier to manipulate type metadata
// for devirtualization.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/TypeMetadataUtils.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

// Search for virtual calls that call FPtr and add them to DevirtCalls.
static void
findCallsAtConstantOffset(SmallVectorImpl<DevirtCallSite> &DevirtCalls,
                          bool *HasNonCallUses, Value *FPtr, uint64_t Offset,
                          const CallInst *CI, DominatorTree &DT) {
  for (const Use &U : FPtr->uses()) {
    Instruction *User = cast<Instruction>(U.getUser());
    // Ignore this instruction if it is not dominated by the type intrinsic
    // being analyzed. Otherwise we may transform a call sharing the same
    // vtable pointer incorrectly. Specifically, this situation can arise
    // after indirect call promotion and inlining, where we may have uses
    // of the vtable pointer guarded by a function pointer check, and a fallback
    // indirect call.
    if (CI->getFunction() != User->getFunction())
      continue;
    if (!DT.dominates(CI, User))
      continue;
    if (isa<BitCastInst>(User)) {
      findCallsAtConstantOffset(DevirtCalls, HasNonCallUses, User, Offset, CI,
                                DT);
    } else if (auto *CI = dyn_cast<CallInst>(User)) {
      DevirtCalls.push_back({Offset, *CI});
    } else if (auto *II = dyn_cast<InvokeInst>(User)) {
      DevirtCalls.push_back({Offset, *II});
    } else if (HasNonCallUses) {
      *HasNonCallUses = true;
    }
  }
}

// Search for virtual calls that load from VPtr and add them to DevirtCalls.
static void findLoadCallsAtConstantOffset(
    const Module *M, SmallVectorImpl<DevirtCallSite> &DevirtCalls, Value *VPtr,
    int64_t Offset, const CallInst *CI, DominatorTree &DT) {
  if (!VPtr->hasUseList())
    return;

  for (const Use &U : VPtr->uses()) {
    Value *User = U.getUser();
    if (isa<BitCastInst>(User)) {
      findLoadCallsAtConstantOffset(M, DevirtCalls, User, Offset, CI, DT);
    } else if (isa<LoadInst>(User)) {
      findCallsAtConstantOffset(DevirtCalls, nullptr, User, Offset, CI, DT);
    } else if (auto GEP = dyn_cast<GetElementPtrInst>(User)) {
      // Take into account the GEP offset.
      if (VPtr == GEP->getPointerOperand() && GEP->hasAllConstantIndices()) {
        SmallVector<Value *, 8> Indices(drop_begin(GEP->operands()));
        int64_t GEPOffset = M->getDataLayout().getIndexedOffsetInType(
            GEP->getSourceElementType(), Indices);
        findLoadCallsAtConstantOffset(M, DevirtCalls, User, Offset + GEPOffset,
                                      CI, DT);
      }
    } else if (auto *Call = dyn_cast<CallInst>(User)) {
      if (Call->getIntrinsicID() == toolchain::Intrinsic::load_relative) {
        if (auto *LoadOffset = dyn_cast<ConstantInt>(Call->getOperand(1))) {
          findCallsAtConstantOffset(DevirtCalls, nullptr, User,
                                    Offset + LoadOffset->getSExtValue(), CI,
                                    DT);
        }
      }
    }
  }
}

void toolchain::findDevirtualizableCallsForTypeTest(
    SmallVectorImpl<DevirtCallSite> &DevirtCalls,
    SmallVectorImpl<CallInst *> &Assumes, const CallInst *CI,
    DominatorTree &DT) {
  assert(CI->getCalledFunction()->getIntrinsicID() == Intrinsic::type_test ||
         CI->getCalledFunction()->getIntrinsicID() ==
             Intrinsic::public_type_test);

  const Module *M = CI->getParent()->getParent()->getParent();

  // Find toolchain.assume intrinsics for this toolchain.type.test call.
  for (const Use &CIU : CI->uses())
    if (auto *Assume = dyn_cast<AssumeInst>(CIU.getUser()))
      Assumes.push_back(Assume);

  // If we found any, search for virtual calls based on %p and add them to
  // DevirtCalls.
  if (!Assumes.empty())
    findLoadCallsAtConstantOffset(
        M, DevirtCalls, CI->getArgOperand(0)->stripPointerCasts(), 0, CI, DT);
}

void toolchain::findDevirtualizableCallsForTypeCheckedLoad(
    SmallVectorImpl<DevirtCallSite> &DevirtCalls,
    SmallVectorImpl<Instruction *> &LoadedPtrs,
    SmallVectorImpl<Instruction *> &Preds, bool &HasNonCallUses,
    const CallInst *CI, DominatorTree &DT) {
  assert(CI->getCalledFunction()->getIntrinsicID() ==
             Intrinsic::type_checked_load ||
         CI->getCalledFunction()->getIntrinsicID() ==
             Intrinsic::type_checked_load_relative);

  auto *Offset = dyn_cast<ConstantInt>(CI->getArgOperand(1));
  if (!Offset) {
    HasNonCallUses = true;
    return;
  }

  for (const Use &U : CI->uses()) {
    auto CIU = U.getUser();
    if (auto EVI = dyn_cast<ExtractValueInst>(CIU)) {
      if (EVI->getNumIndices() == 1 && EVI->getIndices()[0] == 0) {
        LoadedPtrs.push_back(EVI);
        continue;
      }
      if (EVI->getNumIndices() == 1 && EVI->getIndices()[0] == 1) {
        Preds.push_back(EVI);
        continue;
      }
    }
    HasNonCallUses = true;
  }

  for (Value *LoadedPtr : LoadedPtrs)
    findCallsAtConstantOffset(DevirtCalls, &HasNonCallUses, LoadedPtr,
                              Offset->getZExtValue(), CI, DT);
}

Constant *toolchain::getPointerAtOffset(Constant *I, uint64_t Offset, Module &M,
                                   Constant *TopLevelGlobal) {
  // TODO: Ideally it would be the caller who knows if it's appropriate to strip
  // the DSOLocalEquicalent. More generally, it would feel more appropriate to
  // have two functions that handle absolute and relative pointers separately.
  if (auto *Equiv = dyn_cast<DSOLocalEquivalent>(I))
    I = Equiv->getGlobalValue();

  if (I->getType()->isPointerTy()) {
    if (Offset == 0)
      return I;
    return nullptr;
  }

  const DataLayout &DL = M.getDataLayout();

  if (auto *C = dyn_cast<ConstantStruct>(I)) {
    const StructLayout *SL = DL.getStructLayout(C->getType());
    if (Offset >= SL->getSizeInBytes())
      return nullptr;

    unsigned Op = SL->getElementContainingOffset(Offset);
    return getPointerAtOffset(cast<Constant>(I->getOperand(Op)),
                              Offset - SL->getElementOffset(Op), M,
                              TopLevelGlobal);
  }
  if (auto *C = dyn_cast<ConstantArray>(I)) {
    ArrayType *VTableTy = C->getType();
    uint64_t ElemSize = DL.getTypeAllocSize(VTableTy->getElementType());

    unsigned Op = Offset / ElemSize;
    if (Op >= C->getNumOperands())
      return nullptr;

    return getPointerAtOffset(cast<Constant>(I->getOperand(Op)),
                              Offset % ElemSize, M, TopLevelGlobal);
  }

  // Relative-pointer support starts here.
  if (auto *CI = dyn_cast<ConstantInt>(I)) {
    if (Offset == 0 && CI->isZero()) {
      return I;
    }
  }
  if (auto *C = dyn_cast<ConstantExpr>(I)) {
    switch (C->getOpcode()) {
    case Instruction::Trunc:
    case Instruction::PtrToInt:
      return getPointerAtOffset(cast<Constant>(C->getOperand(0)), Offset, M,
                                TopLevelGlobal);
    case Instruction::Sub: {
      auto *Operand0 = cast<Constant>(C->getOperand(0));
      auto *Operand1 = cast<Constant>(C->getOperand(1));

      auto StripGEP = [](Constant *C) {
        auto *CE = dyn_cast<ConstantExpr>(C);
        if (!CE)
          return C;
        if (CE->getOpcode() != Instruction::GetElementPtr)
          return C;
        return CE->getOperand(0);
      };
      auto *Operand1TargetGlobal = StripGEP(getPointerAtOffset(Operand1, 0, M));

      // Check that in the "sub (@a, @b)" expression, @b points back to the top
      // level global (or a GEP thereof) that we're processing. Otherwise bail.
      if (Operand1TargetGlobal != TopLevelGlobal)
        return nullptr;

      return getPointerAtOffset(Operand0, Offset, M, TopLevelGlobal);
    }
    default:
      return nullptr;
    }
  }
  return nullptr;
}

std::pair<Function *, Constant *>
toolchain::getFunctionAtVTableOffset(GlobalVariable *GV, uint64_t Offset,
                                Module &M) {
  Constant *Ptr = getPointerAtOffset(GV->getInitializer(), Offset, M, GV);
  if (!Ptr)
    return std::pair<Function *, Constant *>(nullptr, nullptr);

  auto C = Ptr->stripPointerCasts();
  // Make sure this is a function or alias to a function.
  auto Fn = dyn_cast<Function>(C);
  auto A = dyn_cast<GlobalAlias>(C);
  if (!Fn && A)
    Fn = dyn_cast<Function>(A->getAliasee());

  if (!Fn)
    return std::pair<Function *, Constant *>(nullptr, nullptr);

  return std::pair<Function *, Constant *>(Fn, C);
}

static void replaceRelativePointerUserWithZero(User *U) {
  auto *PtrExpr = dyn_cast<ConstantExpr>(U);
  if (!PtrExpr || PtrExpr->getOpcode() != Instruction::PtrToInt)
    return;

  for (auto *PtrToIntUser : PtrExpr->users()) {
    auto *SubExpr = dyn_cast<ConstantExpr>(PtrToIntUser);
    if (!SubExpr || SubExpr->getOpcode() != Instruction::Sub)
      return;

    SubExpr->replaceNonMetadataUsesWith(
        ConstantInt::get(SubExpr->getType(), 0));
  }
}

void toolchain::replaceRelativePointerUsersWithZero(Constant *C) {
  for (auto *U : C->users()) {
    if (auto *Equiv = dyn_cast<DSOLocalEquivalent>(U))
      replaceRelativePointerUsersWithZero(Equiv);
    else
      replaceRelativePointerUserWithZero(U);
  }
}
