//===- ReplaceConstant.cpp - Replace LLVM constant expression--------------===//
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
// This file implements a utility function for replacing LLVM constant
// expressions by instructions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/ReplaceConstant.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Instructions.h"

using namespace vm::core;

static bool isExpandableUser(User *U) {
  return isa<ConstantExpr>(U) || isa<ConstantAggregate>(U);
}

static void expandUser(BasicBlock::iterator InsertPt, Constant *C,
                       SmallVector<Instruction *, 4> &NewInsts) {
  NewInsts.clear();
  if (auto *CE = dyn_cast<ConstantExpr>(C)) {
    Instruction *ConstInst = CE->getAsInstruction();
    ConstInst->insertBefore(*InsertPt->getParent(), InsertPt);
    NewInsts.push_back(ConstInst);
  } else if (isa<ConstantStruct>(C) || isa<ConstantArray>(C)) {
    Value *V = PoisonValue::get(C->getType());
    for (auto [Idx, Op] : enumerate(C->operands())) {
      V = InsertValueInst::Create(V, Op, Idx, "", InsertPt);
      NewInsts.push_back(cast<Instruction>(V));
    }
  } else if (isa<ConstantVector>(C)) {
    Type *IdxTy = Type::getInt32Ty(C->getContext());
    Value *V = PoisonValue::get(C->getType());
    for (auto [Idx, Op] : enumerate(C->operands())) {
      V = InsertElementInst::Create(V, Op, ConstantInt::get(IdxTy, Idx), "",
                                    InsertPt);
      NewInsts.push_back(cast<Instruction>(V));
    }
  } else {
    llvm_unreachable("Not an expandable user");
  }
}

bool toolchain::convertUsersOfConstantsToInstructions(ArrayRef<Constant *> Consts,
                                                 Function *RestrictToFunc,
                                                 bool RemoveDeadConstants,
                                                 bool IncludeSelf) {
  // Find all expandable direct users of Consts.
  SmallVector<Constant *> Stack;
  for (Constant *C : Consts) {
    assert(!isa<ConstantData>(C) &&
           "should not be expanding trivial constant users");

    if (IncludeSelf) {
      assert(isExpandableUser(C) && "One of the constants is not expandable");
      Stack.push_back(C);
    } else {
      for (User *U : C->users())
        if (isExpandableUser(U))
          Stack.push_back(cast<Constant>(U));
    }
  }

  // Include transitive users.
  SetVector<Constant *> ExpandableUsers;
  while (!Stack.empty()) {
    Constant *C = Stack.pop_back_val();
    if (!ExpandableUsers.insert(C))
      continue;

    for (auto *Nested : C->users())
      if (isExpandableUser(Nested))
        Stack.push_back(cast<Constant>(Nested));
  }

  // Find all instructions that use any of the expandable users
  SetVector<Instruction *> InstructionWorklist;
  for (Constant *C : ExpandableUsers)
    for (User *U : C->users())
      if (auto *I = dyn_cast<Instruction>(U))
        if (!RestrictToFunc || I->getFunction() == RestrictToFunc)
          InstructionWorklist.insert(I);

  // Replace those expandable operands with instructions
  bool Changed = false;
  // We need to cache the instructions we've already expanded to avoid expanding
  // the same constant multiple times in the same basic block, which is
  // problematic when the same constant is used in a phi node multiple times.
  DenseMap<std::pair<Constant *, BasicBlock *>, SmallVector<Instruction *, 4>>
      ConstantToInstructionMap;
  while (!InstructionWorklist.empty()) {
    Instruction *I = InstructionWorklist.pop_back_val();
    DebugLoc Loc = I->getDebugLoc();
    for (Use &U : I->operands()) {
      BasicBlock::iterator BI = I->getIterator();
      if (auto *Phi = dyn_cast<PHINode>(I)) {
        BasicBlock *BB = Phi->getIncomingBlock(U);
        BI = BB->getFirstInsertionPt();
        assert(BI != BB->end() && "Unexpected empty basic block");
      }

      if (auto *C = dyn_cast<Constant>(U.get())) {
        if (ExpandableUsers.contains(C)) {
          Changed = true;
          SmallVector<Instruction *, 4> &NewInsts =
              ConstantToInstructionMap[std::make_pair(C, BI->getParent())];
          // If the cached instruction is after the insertion point, we need to
          // create a new one. We can't simply move the cached instruction
          // because its operands (also expanded instructions) might not
          // dominate the new position.
          if (NewInsts.empty() || BI->comesBefore(NewInsts.front()))
            expandUser(BI, C, NewInsts);
          for (auto *NI : NewInsts)
            NI->setDebugLoc(Loc);
          InstructionWorklist.insert_range(NewInsts);
          U.set(NewInsts.back());
        }
      }
    }
  }

  if (RemoveDeadConstants)
    for (Constant *C : Consts)
      C->removeDeadConstantUsers();

  return Changed;
}
