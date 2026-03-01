//===-- XCoreLowerThreadLocal - Lower thread local variables --------------===//
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
///
/// \file
/// This file contains a pass that lowers thread local variables on the
///        XCore.
///
//===----------------------------------------------------------------------===//

#include "XCore.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/IntrinsicsXCore.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/ValueHandle.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"

#define DEBUG_TYPE "xcore-lower-thread-local"

using namespace vm::core;

static cl::opt<unsigned> MaxThreads(
  "xcore-max-threads", cl::Optional,
  cl::desc("Maximum number of threads (for emulation thread-local storage)"),
  cl::Hidden, cl::value_desc("number"), cl::init(8));

namespace {
  /// Lowers thread local variables on the XCore. Each thread local variable is
  /// expanded to an array of n elements indexed by the thread ID where n is the
  /// fixed number hardware threads supported by the device.
  struct XCoreLowerThreadLocal : public ModulePass {
    static char ID;

    XCoreLowerThreadLocal() : ModulePass(ID) {}

    bool lowerGlobal(GlobalVariable *GV);

    bool runOnModule(Module &M) override;
  };
}

char XCoreLowerThreadLocal::ID = 0;

INITIALIZE_PASS(XCoreLowerThreadLocal, "xcore-lower-thread-local",
                "Lower thread local variables", false, false)

ModulePass *toolchain::createXCoreLowerThreadLocalPass() {
  return new XCoreLowerThreadLocal();
}

static ArrayType *createLoweredType(Type *OriginalType) {
  return ArrayType::get(OriginalType, MaxThreads);
}

static Constant *
createLoweredInitializer(ArrayType *NewType, Constant *OriginalInitializer) {
  SmallVector<Constant *, 8> Elements(MaxThreads);
  for (unsigned i = 0; i != MaxThreads; ++i) {
    Elements[i] = OriginalInitializer;
  }
  return ConstantArray::get(NewType, Elements);
}


static bool replaceConstantExprOp(ConstantExpr *CE, Pass *P) {
  do {
    SmallVector<WeakTrackingVH, 8> WUsers(CE->users());
    toolchain::sort(WUsers);
    WUsers.erase(toolchain::unique(WUsers), WUsers.end());
    while (!WUsers.empty())
      if (WeakTrackingVH WU = WUsers.pop_back_val()) {
        if (PHINode *PN = dyn_cast<PHINode>(WU)) {
          for (int I = 0, E = PN->getNumIncomingValues(); I < E; ++I)
            if (PN->getIncomingValue(I) == CE) {
              BasicBlock *PredBB = PN->getIncomingBlock(I);
              if (PredBB->getTerminator()->getNumSuccessors() > 1)
                PredBB = SplitEdge(PredBB, PN->getParent());
              BasicBlock::iterator InsertPos =
                  PredBB->getTerminator()->getIterator();
              Instruction *NewInst = CE->getAsInstruction();
              NewInst->insertBefore(*PredBB, InsertPos);
              PN->setOperand(I, NewInst);
            }
        } else if (Instruction *Instr = dyn_cast<Instruction>(WU)) {
          Instruction *NewInst = CE->getAsInstruction();
          NewInst->insertBefore(*Instr->getParent(), Instr->getIterator());
          Instr->replaceUsesOfWith(CE, NewInst);
        } else {
          ConstantExpr *CExpr = dyn_cast<ConstantExpr>(WU);
          if (!CExpr || !replaceConstantExprOp(CExpr, P))
            return false;
        }
      }
  } while (CE->hasNUsesOrMore(1)); // We need to check because a recursive
  // sibling may have used 'CE' when getAsInstruction was called.
  CE->destroyConstant();
  return true;
}

static bool rewriteNonInstructionUses(GlobalVariable *GV, Pass *P) {
  SmallVector<WeakTrackingVH, 8> WUsers;
  for (User *U : GV->users())
    if (!isa<Instruction>(U))
      WUsers.push_back(WeakTrackingVH(U));
  while (!WUsers.empty())
    if (WeakTrackingVH WU = WUsers.pop_back_val()) {
      ConstantExpr *CE = dyn_cast<ConstantExpr>(WU);
      if (!CE || !replaceConstantExprOp(CE, P))
        return false;
    }
  return true;
}

static bool isZeroLengthArray(Type *Ty) {
  ArrayType *AT = dyn_cast<ArrayType>(Ty);
  return AT && (AT->getNumElements() == 0);
}

bool XCoreLowerThreadLocal::lowerGlobal(GlobalVariable *GV) {
  Module *M = GV->getParent();
  if (!GV->isThreadLocal())
    return false;

  // Skip globals that we can't lower and leave it for the backend to error.
  if (!rewriteNonInstructionUses(GV, this) ||
      !GV->getType()->isSized() || isZeroLengthArray(GV->getType()))
    return false;

  // Create replacement global.
  ArrayType *NewType = createLoweredType(GV->getValueType());
  Constant *NewInitializer = nullptr;
  if (GV->hasInitializer())
    NewInitializer = createLoweredInitializer(NewType,
                                              GV->getInitializer());
  GlobalVariable *NewGV =
    new GlobalVariable(*M, NewType, GV->isConstant(), GV->getLinkage(),
                       NewInitializer, "", nullptr,
                       GlobalVariable::NotThreadLocal,
                       GV->getType()->getAddressSpace(),
                       GV->isExternallyInitialized());

  // Update uses.
  SmallVector<User *, 16> Users(GV->users());
  for (User *U : Users) {
    Instruction *Inst = cast<Instruction>(U);
    IRBuilder<> Builder(Inst);
    Value *ThreadID = Builder.CreateIntrinsic(Intrinsic::xcore_getid, {});
    Value *Addr = Builder.CreateInBoundsGEP(NewGV->getValueType(), NewGV,
                                            {Builder.getInt64(0), ThreadID});
    U->replaceUsesOfWith(GV, Addr);
  }

  // Remove old global.
  NewGV->takeName(GV);
  GV->eraseFromParent();
  return true;
}

bool XCoreLowerThreadLocal::runOnModule(Module &M) {
  // Find thread local globals.
  bool MadeChange = false;
  SmallVector<GlobalVariable *, 16> ThreadLocalGlobals;
  for (GlobalVariable &GV : M.globals())
    if (GV.isThreadLocal())
      ThreadLocalGlobals.push_back(&GV);
  for (GlobalVariable *GV : ThreadLocalGlobals)
    MadeChange |= lowerGlobal(GV);
  return MadeChange;
}
