//===- SPIRVLegalizeZeroSizeArrays.cpp - Legalize zero-size arrays -------===//
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
// SPIR-V does not support zero-size arrays unless it is within a shader. This
// pass legalizes zero-size arrays ([0 x T]) in unsupported cases.
//
//===----------------------------------------------------------------------===//

#include "SPIRVLegalizeZeroSizeArrays.h"
#include "SPIRV.h"
#include "SPIRVTargetMachine.h"
#include "SPIRVUtils.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "spirv-legalize-zero-size-arrays"

using namespace vm::core;

namespace {

bool hasZeroSizeArray(const Type *Ty) {
  if (const ArrayType *ArrTy = dyn_cast<ArrayType>(Ty)) {
    if (ArrTy->getNumElements() == 0)
      return true;
    return hasZeroSizeArray(ArrTy->getElementType());
  }

  if (const StructType *StructTy = dyn_cast<StructType>(Ty)) {
    for (Type *ElemTy : StructTy->elements()) {
      if (hasZeroSizeArray(ElemTy))
        return true;
    }
  }

  return false;
}

bool shouldLegalizeInstType(const Type *Ty) {
  // This recursive function will always terminate because we only look inside
  // array types, and those can't be recursive.
  if (const ArrayType *ArrTy = dyn_cast_if_present<ArrayType>(Ty)) {
    return ArrTy->getNumElements() == 0 ||
           shouldLegalizeInstType(ArrTy->getElementType());
  }
  return false;
}

class SPIRVLegalizeZeroSizeArraysImpl
    : public InstVisitor<SPIRVLegalizeZeroSizeArraysImpl> {
  friend class InstVisitor<SPIRVLegalizeZeroSizeArraysImpl>;

public:
  SPIRVLegalizeZeroSizeArraysImpl(const SPIRVTargetMachine &TM)
      : InstVisitor(), TM(TM) {}
  bool runOnModule(Module &M);

  // TODO: Handle GEP, PHI.
  void visitAllocaInst(AllocaInst &AI);
  void visitLoadInst(LoadInst &LI);
  void visitStoreInst(StoreInst &SI);
  void visitSelectInst(SelectInst &Sel);
  void visitExtractValueInst(ExtractValueInst &EVI);
  void visitInsertValueInst(InsertValueInst &IVI);

private:
  Type *legalizeType(Type *Ty);
  Constant *legalizeConstant(Constant *C);

  const SPIRVTargetMachine &TM;
  DenseMap<Type *, Type *> TypeMap;
  DenseMap<GlobalVariable *, GlobalVariable *> GlobalMap;
  SmallVector<Instruction *, 16> ToErase;
  bool Modified = false;
};

class SPIRVLegalizeZeroSizeArraysLegacy : public ModulePass {
public:
  static char ID;
  SPIRVLegalizeZeroSizeArraysLegacy(const SPIRVTargetMachine &TM)
      : ModulePass(ID), TM(TM) {}
  StringRef getPassName() const override {
    return "SPIRV Legalize Zero-Size Arrays";
  }
  bool runOnModule(Module &M) override {
    SPIRVLegalizeZeroSizeArraysImpl Impl(TM);
    return Impl.runOnModule(M);
  }

private:
  const SPIRVTargetMachine &TM;
};

// Legalize a type. There are only two cases we need to care about:
// arrays and structs.
//
// For arrays, we just replace the entire array type with a ptr.
//
// For structs, we create a new type with any members containing
// nested arrays legalized.

Type *SPIRVLegalizeZeroSizeArraysImpl::legalizeType(Type *Ty) {
  auto It = TypeMap.find(Ty);
  if (It != TypeMap.end())
    return It->second;

  Type *LegalizedTy = Ty;

  if (isa<ArrayType>(Ty)) {
    LegalizedTy = PointerType::get(
        Ty->getContext(),
        storageClassToAddressSpace(SPIRV::StorageClass::Generic));

  } else if (StructType *StructTy = dyn_cast<StructType>(Ty)) {
    SmallVector<Type *, 8> ElemTypes;
    bool Changed = false;
    for (Type *ElemTy : StructTy->elements()) {
      Type *LegalizedElemTy = legalizeType(ElemTy);
      ElemTypes.push_back(LegalizedElemTy);
      Changed |= LegalizedElemTy != ElemTy;
    }
    if (Changed) {
      LegalizedTy =
          StructTy->hasName()
              ? StructType::create(StructTy->getContext(), ElemTypes,
                                   (StructTy->getName() + ".legalized").str(),
                                   StructTy->isPacked())
              : StructType::get(StructTy->getContext(), ElemTypes,
                                StructTy->isPacked());
    }
  }

  TypeMap[Ty] = LegalizedTy;
  return LegalizedTy;
}

Constant *SPIRVLegalizeZeroSizeArraysImpl::legalizeConstant(Constant *C) {
  if (!C || !hasZeroSizeArray(C->getType()))
    return C;

  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(C)) {
    if (GlobalVariable *NewGV = GlobalMap.lookup(GV))
      return NewGV;
    return C;
  }

  Type *NewTy = legalizeType(C->getType());
  if (isa<UndefValue>(C))
    return PoisonValue::get(NewTy);
  if (isa<ConstantAggregateZero>(C))
    return Constant::getNullValue(NewTy);
  if (ConstantArray *CA = dyn_cast<ConstantArray>(C)) {
    SmallVector<Constant *, 8> Elems;
    for (Use &U : CA->operands())
      Elems.push_back(legalizeConstant(cast<Constant>(U)));
    return ConstantArray::get(cast<ArrayType>(NewTy), Elems);
  }

  if (ConstantStruct *CS = dyn_cast<ConstantStruct>(C)) {
    SmallVector<Constant *, 8> Fields;
    for (Use &U : CS->operands())
      Fields.push_back(legalizeConstant(cast<Constant>(U)));
    return ConstantStruct::get(cast<StructType>(NewTy), Fields);
  }

  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(C)) {
    // Don't legalize GEP constant expressions, the backend deals with them
    // fine.
    if (CE->getOpcode() == Instruction::GetElementPtr)
      return CE;
    SmallVector<Constant *, 4> Ops;
    bool Changed = false;
    for (Use &U : CE->operands()) {
      Constant *LegalizedOp = legalizeConstant(cast<Constant>(U));
      Ops.push_back(LegalizedOp);
      Changed |= LegalizedOp != cast<Constant>(U.get());
    }
    if (Changed)
      return CE->getWithOperands(Ops);
  }

  return C;
}

void SPIRVLegalizeZeroSizeArraysImpl::visitAllocaInst(AllocaInst &AI) {
  if (!hasZeroSizeArray(AI.getAllocatedType()))
    return;

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy = dyn_cast<ArrayType>(AI.getAllocatedType());
  if (shouldLegalizeInstType(ArrTy)) {
    // Allocate a generic pointer instead of an empty array.
    IRBuilder<> Builder(&AI);
    AllocaInst *NewAI = Builder.CreateAlloca(
        PointerType::get(
            ArrTy->getContext(),
            storageClassToAddressSpace(SPIRV::StorageClass::Generic)),
        /*ArraySize=*/nullptr, AI.getName());
    NewAI->setAlignment(AI.getAlign());
    NewAI->setDebugLoc(AI.getDebugLoc());
    AI.replaceAllUsesWith(NewAI);
    ToErase.push_back(&AI);
    Modified = true;
  }
}

void SPIRVLegalizeZeroSizeArraysImpl::visitLoadInst(LoadInst &LI) {
  if (!hasZeroSizeArray(LI.getType()))
    return;

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy = dyn_cast<ArrayType>(LI.getType());
  if (shouldLegalizeInstType(ArrTy)) {
    LI.replaceAllUsesWith(PoisonValue::get(LI.getType()));
    ToErase.push_back(&LI);
    Modified = true;
  }
}

void SPIRVLegalizeZeroSizeArraysImpl::visitStoreInst(StoreInst &SI) {
  Type *StoreTy = SI.getValueOperand()->getType();

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy = dyn_cast<ArrayType>(StoreTy);
  if (shouldLegalizeInstType(ArrTy)) {
    ToErase.push_back(&SI);
    Modified = true;
  }
}

void SPIRVLegalizeZeroSizeArraysImpl::visitSelectInst(SelectInst &Sel) {
  if (!hasZeroSizeArray(Sel.getType()))
    return;

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy = dyn_cast<ArrayType>(Sel.getType());
  if (shouldLegalizeInstType(ArrTy)) {
    Sel.replaceAllUsesWith(PoisonValue::get(Sel.getType()));
    ToErase.push_back(&Sel);
    Modified = true;
  }
}

void SPIRVLegalizeZeroSizeArraysImpl::visitExtractValueInst(
    ExtractValueInst &EVI) {
  if (!hasZeroSizeArray(EVI.getAggregateOperand()->getType()))
    return;

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy = dyn_cast<ArrayType>(EVI.getType());
  if (shouldLegalizeInstType(ArrTy)) {
    EVI.replaceAllUsesWith(PoisonValue::get(EVI.getType()));
    ToErase.push_back(&EVI);
    Modified = true;
  }
}

void SPIRVLegalizeZeroSizeArraysImpl::visitInsertValueInst(
    InsertValueInst &IVI) {
  if (!hasZeroSizeArray(IVI.getAggregateOperand()->getType()))
    return;

  // TODO: Handle structs containing zero-size arrays.
  ArrayType *ArrTy =
      dyn_cast<ArrayType>(IVI.getInsertedValueOperand()->getType());
  if (shouldLegalizeInstType(ArrTy)) {
    IVI.replaceAllUsesWith(IVI.getAggregateOperand());
    ToErase.push_back(&IVI);
    Modified = true;
  }
}

bool SPIRVLegalizeZeroSizeArraysImpl::runOnModule(Module &M) {
  TypeMap.clear();
  GlobalMap.clear();
  ToErase.clear();
  Modified = false;

  // Runtime arrays are allowed for shaders, so we don't need to do anything.
  if (TM.getSubtargetImpl()->isShader())
    return false;

  // First pass: create new globals (legalizing the initializer as needed) and
  // track mapping (don't erase old ones yet).
  SmallVector<GlobalVariable *, 8> OldGlobals;
  for (GlobalVariable &GV : M.globals()) {
    if (!hasZeroSizeArray(GV.getValueType()))
      continue;

    // toolchain.embedded.module is handled by SPIRVPrepareGlobals.
    if (GV.getName() == "toolchain.embedded.module")
      continue;

    Type *NewTy = legalizeType(GV.getValueType());
    Constant *LegalizedInitializer = legalizeConstant(GV.getInitializer());

    // Use an empty name for now, we will update it in the
    // following step.
    GlobalVariable *NewGV = new GlobalVariable(
        M, NewTy, GV.isConstant(), GV.getLinkage(), LegalizedInitializer,
        /*Name=*/"", &GV, GV.getThreadLocalMode(), GV.getAddressSpace(),
        GV.isExternallyInitialized());
    NewGV->copyAttributesFrom(&GV);
    NewGV->copyMetadata(&GV, 0);
    NewGV->setComdat(GV.getComdat());
    NewGV->setAlignment(GV.getAlign());
    GlobalMap[&GV] = NewGV;
    OldGlobals.push_back(&GV);
    Modified = true;
  }

  // Second pass: replace uses, transfer names, and erase old globals.
  for (GlobalVariable *GV : OldGlobals) {
    GlobalVariable *NewGV = GlobalMap[GV];
    GV->replaceAllUsesWith(ConstantExpr::getBitCast(NewGV, GV->getType()));
    NewGV->takeName(GV);
    GV->eraseFromParent();
  }

  for (Function &F : M)
    for (Instruction &I : instructions(F))
      visit(I);

  for (Instruction *I : ToErase)
    I->eraseFromParent();

  return Modified;
}

} // namespace

PreservedAnalyses SPIRVLegalizeZeroSizeArrays::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  SPIRVLegalizeZeroSizeArraysImpl Impl(TM);
  if (Impl.runOnModule(M))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

char SPIRVLegalizeZeroSizeArraysLegacy::ID = 0;

INITIALIZE_PASS(SPIRVLegalizeZeroSizeArraysLegacy,
                "spirv-legalize-zero-size-arrays",
                "Legalize SPIR-V zero-size arrays", false, false)

ModulePass *
toolchain::createSPIRVLegalizeZeroSizeArraysPass(const SPIRVTargetMachine &TM) {
  return new SPIRVLegalizeZeroSizeArraysLegacy(TM);
}
