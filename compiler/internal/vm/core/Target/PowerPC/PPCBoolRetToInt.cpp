//===- PPCBoolRetToInt.cpp ------------------------------------------------===//
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
// This file implements converting i1 values to i32/i64 if they could be more
// profitably allocated as GPRs rather than CRs. This pass will become totally
// unnecessary if Register Bank Allocation and Global Instruction Selection ever
// go upstream.
//
// Presently, the pass converts i1 Constants, and Arguments to i32/i64 if the
// transitive closure of their uses includes only PHINodes, CallInsts, and
// ReturnInsts. The rational is that arguments are generally passed and returned
// in GPRs rather than CRs, so casting them to i32/i64 at the LLVM IR level will
// actually save casts at the Machine Instruction level.
//
// It might be useful to expand this pass to add bit-wise operations to the list
// of safe transitive closure types. Also, we miss some opportunities when LLVM
// represents logical AND and OR operations with control flow rather than data
// flow. For example by lowering the expression: return (A && B && C)
//
// as: return A ? true : B && C.
//
// There's code in SimplifyCFG that code be used to turn control flow in data
// flow using SelectInsts. Selects are slow on some architectures (P7/P8), so
// this probably isn't good in general, but for the special case of i1, the
// Selects could be further lowered to bit operations that are fast everywhere.
//
//===----------------------------------------------------------------------===//

#include "PPC.h"
#include "PPCTargetMachine.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Argument.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Use.h"
#include "vm/core/IR/User.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Casting.h"
#include <cassert>

using namespace vm::core;

namespace {

#define DEBUG_TYPE "ppc-bool-ret-to-int"

STATISTIC(NumBoolRetPromotion,
          "Number of times a bool feeding a RetInst was promoted to an int");
STATISTIC(NumBoolCallPromotion,
          "Number of times a bool feeding a CallInst was promoted to an int");
STATISTIC(NumBoolToIntPromotion,
          "Total number of times a bool was promoted to an int");

class PPCBoolRetToInt : public FunctionPass {
  static SmallPtrSet<Value *, 8> findAllDefs(Value *V) {
    SmallPtrSet<Value *, 8> Defs;
    SmallVector<Value *, 8> WorkList;
    WorkList.push_back(V);
    Defs.insert(V);
    while (!WorkList.empty()) {
      Value *Curr = WorkList.pop_back_val();
      auto *CurrUser = dyn_cast<User>(Curr);
      // Operands of CallInst/Constant are skipped because they may not be Bool
      // type. For CallInst, their positions are defined by ABI.
      if (CurrUser && !isa<CallInst>(Curr) && !isa<Constant>(Curr))
        for (auto &Op : CurrUser->operands())
          if (Defs.insert(Op).second)
            WorkList.push_back(Op);
    }
    return Defs;
  }

  // Translate a i1 value to an equivalent i32/i64 value:
  Value *translate(Value *V) {
    assert(V->getType() == Type::getInt1Ty(V->getContext()) &&
           "Expect an i1 value");

    Type *IntTy = ST->isPPC64() ? Type::getInt64Ty(V->getContext())
                                : Type::getInt32Ty(V->getContext());

    if (auto *P = dyn_cast<PHINode>(V)) {
      // Temporarily set the operands to 0. We'll fix this later in
      // runOnUse.
      Value *Zero = Constant::getNullValue(IntTy);
      PHINode *Q =
        PHINode::Create(IntTy, P->getNumIncomingValues(), P->getName(), P->getIterator());
      for (unsigned I = 0; I < P->getNumOperands(); ++I)
        Q->addIncoming(Zero, P->getIncomingBlock(I));
      return Q;
    }

    IRBuilder IRB(V->getContext());
    if (auto *I = dyn_cast<Instruction>(V))
      IRB.SetInsertPoint(I->getNextNode());
    else
      IRB.SetInsertPoint(&Func->getEntryBlock(), Func->getEntryBlock().begin());
    return IRB.CreateZExt(V, IntTy);
  }

  typedef SmallPtrSet<const PHINode *, 8> PHINodeSet;

  // A PHINode is Promotable if:
  // 1. Its type is i1 AND
  // 2. All of its uses are ReturnInt, CallInst, or PHINode
  // AND
  // 3. All of its operands are Constant or Argument or
  //    CallInst or PHINode AND
  // 4. All of its PHINode uses are Promotable AND
  // 5. All of its PHINode operands are Promotable
  static PHINodeSet getPromotablePHINodes(const Function &F) {
    PHINodeSet Promotable;
    // Condition 1
    for (auto &BB : F)
      for (auto &I : BB)
        if (const auto *P = dyn_cast<PHINode>(&I))
          if (P->getType()->isIntegerTy(1))
            Promotable.insert(P);

    SmallVector<const PHINode *, 8> ToRemove;
    for (const PHINode *P : Promotable) {
      // Condition 2 and 3
      auto IsValidUser = [] (const Value *V) -> bool {
        return isa<ReturnInst>(V) || isa<CallInst>(V) || isa<PHINode>(V);
      };
      auto IsValidOperand = [] (const Value *V) -> bool {
        return isa<Constant>(V) || isa<Argument>(V) || isa<CallInst>(V) ||
        isa<PHINode>(V);
      };
      const auto &Users = P->users();
      const auto &Operands = P->operands();
      if (!toolchain::all_of(Users, IsValidUser) ||
          !toolchain::all_of(Operands, IsValidOperand))
        ToRemove.push_back(P);
    }

    // Iterate to convergence
    auto IsPromotable = [&Promotable] (const Value *V) -> bool {
      const auto *Phi = dyn_cast<PHINode>(V);
      return !Phi || Promotable.count(Phi);
    };
    while (!ToRemove.empty()) {
      for (auto &User : ToRemove)
        Promotable.erase(User);
      ToRemove.clear();

      for (const PHINode *P : Promotable) {
        // Condition 4 and 5
        const auto &Users = P->users();
        const auto &Operands = P->operands();
        if (!toolchain::all_of(Users, IsPromotable) ||
            !toolchain::all_of(Operands, IsPromotable))
          ToRemove.push_back(P);
      }
    }

    return Promotable;
  }

  typedef DenseMap<Value *, Value *> B2IMap;

 public:
  static char ID;

  PPCBoolRetToInt() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    auto &TM = TPC->getTM<PPCTargetMachine>();
    ST = TM.getSubtargetImpl(F);
    Func = &F;

    PHINodeSet PromotablePHINodes = getPromotablePHINodes(F);
    B2IMap Bool2IntMap;
    bool Changed = false;
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *R = dyn_cast<ReturnInst>(&I))
          if (F.getReturnType()->isIntegerTy(1))
            Changed |=
              runOnUse(R->getOperandUse(0), PromotablePHINodes, Bool2IntMap);

        if (auto *CI = dyn_cast<CallInst>(&I))
          for (auto &U : CI->operands())
            if (U->getType()->isIntegerTy(1))
              Changed |= runOnUse(U, PromotablePHINodes, Bool2IntMap);
      }
    }

    return Changed;
  }

  bool runOnUse(Use &U, const PHINodeSet &PromotablePHINodes,
                       B2IMap &BoolToIntMap) {
    auto Defs = findAllDefs(U);

    // If the values are all Constants or Arguments, don't bother
    if (toolchain::none_of(Defs, [](Value *V) { return isa<Instruction>(V); }))
      return false;

    // Presently, we only know how to handle PHINode, Constant, Arguments and
    // CallInst. Potentially, bitwise operations (AND, OR, XOR, NOT) and sign
    // extension could also be handled in the future.
    for (Value *V : Defs)
      if (!isa<PHINode>(V) && !isa<Constant>(V) &&
          !isa<Argument>(V) && !isa<CallInst>(V))
        return false;

    for (Value *V : Defs)
      if (const auto *P = dyn_cast<PHINode>(V))
        if (!PromotablePHINodes.count(P))
          return false;

    if (isa<ReturnInst>(U.getUser()))
      ++NumBoolRetPromotion;
    if (isa<CallInst>(U.getUser()))
      ++NumBoolCallPromotion;
    ++NumBoolToIntPromotion;

    for (Value *V : Defs) {
      auto [It, Inserted] = BoolToIntMap.try_emplace(V);
      if (Inserted)
        It->second = translate(V);
    }

    // Replace the operands of the translated instructions. They were set to
    // zero in the translate function.
    for (auto &Pair : BoolToIntMap) {
      auto *First = dyn_cast<User>(Pair.first);
      auto *Second = dyn_cast<User>(Pair.second);
      assert((!First || Second) && "translated from user to non-user!?");
      // Operands of CallInst/Constant are skipped because they may not be Bool
      // type. For CallInst, their positions are defined by ABI.
      if (First && !isa<CallInst>(First) && !isa<Constant>(First))
        for (unsigned I = 0; I < First->getNumOperands(); ++I)
          Second->setOperand(I, BoolToIntMap[First->getOperand(I)]);
    }

    Value *IntRetVal = BoolToIntMap[U];
    Type *Int1Ty = Type::getInt1Ty(U->getContext());
    auto *I = cast<Instruction>(U.getUser());
    Value *BackToBool =
        new TruncInst(IntRetVal, Int1Ty, "backToBool", I->getIterator());
    U.set(BackToBool);

    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<DominatorTreeWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

private:
  const PPCSubtarget *ST;
  Function *Func;
};

} // end anonymous namespace

char PPCBoolRetToInt::ID = 0;
INITIALIZE_PASS(PPCBoolRetToInt, "ppc-bool-ret-to-int",
                "Convert i1 constants to i32/i64 if they are returned", false,
                false)

FunctionPass *toolchain::createPPCBoolRetToIntPass() { return new PPCBoolRetToInt(); }
