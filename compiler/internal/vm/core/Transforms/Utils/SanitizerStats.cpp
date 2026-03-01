//===- SanitizerStats.cpp - Sanitizer statistics gathering ----------------===//
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
// Implements code generation for sanitizer statistics gathering.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/SanitizerStats.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

SanitizerStatReport::SanitizerStatReport(Module *M) : M(M) {
  StatTy = ArrayType::get(PointerType::getUnqual(M->getContext()), 2);
  EmptyModuleStatsTy = makeModuleStatsTy();

  ModuleStatsGV = new GlobalVariable(*M, EmptyModuleStatsTy, false,
                                     GlobalValue::InternalLinkage, nullptr);
}

ArrayType *SanitizerStatReport::makeModuleStatsArrayTy() {
  return ArrayType::get(StatTy, Inits.size());
}

StructType *SanitizerStatReport::makeModuleStatsTy() {
  return StructType::get(M->getContext(),
                         {PointerType::getUnqual(M->getContext()),
                          Type::getInt32Ty(M->getContext()),
                          makeModuleStatsArrayTy()});
}

void SanitizerStatReport::create(IRBuilder<> &B, SanitizerStatKind SK) {
  Function *F = B.GetInsertBlock()->getParent();
  Module *M = F->getParent();
  PointerType *PtrTy = B.getPtrTy();
  IntegerType *IntPtrTy = B.getIntPtrTy(M->getDataLayout());
  ArrayType *StatTy = ArrayType::get(PtrTy, 2);

  Inits.push_back(ConstantArray::get(
      StatTy,
      {Constant::getNullValue(PtrTy),
       ConstantExpr::getIntToPtr(
           ConstantInt::get(IntPtrTy, uint64_t(SK) << (IntPtrTy->getBitWidth() -
                                                       kSanitizerStatKindBits)),
           PtrTy)}));

  FunctionType *StatReportTy = FunctionType::get(B.getVoidTy(), PtrTy, false);
  FunctionCallee StatReport =
      M->getOrInsertFunction("__sanitizer_stat_report", StatReportTy);

  auto InitAddr = ConstantExpr::getGetElementPtr(
      EmptyModuleStatsTy, ModuleStatsGV,
      ArrayRef<Constant *>{
          ConstantInt::get(IntPtrTy, 0), ConstantInt::get(B.getInt32Ty(), 2),
          ConstantInt::get(IntPtrTy, Inits.size() - 1),
      });
  B.CreateCall(StatReport, InitAddr);
}

void SanitizerStatReport::finish() {
  if (Inits.empty()) {
    ModuleStatsGV->eraseFromParent();
    return;
  }

  PointerType *Int8PtrTy = PointerType::getUnqual(M->getContext());
  IntegerType *Int32Ty = Type::getInt32Ty(M->getContext());
  Type *VoidTy = Type::getVoidTy(M->getContext());

  // Create a new ModuleStatsGV to replace the old one. We can't just set the
  // old one's initializer because its type is different.
  auto NewModuleStatsGV = new GlobalVariable(
      *M, makeModuleStatsTy(), false, GlobalValue::InternalLinkage,
      ConstantStruct::getAnon(
          {Constant::getNullValue(Int8PtrTy),
           ConstantInt::get(Int32Ty, Inits.size()),
           ConstantArray::get(makeModuleStatsArrayTy(), Inits)}));
  ModuleStatsGV->replaceAllUsesWith(NewModuleStatsGV);
  ModuleStatsGV->eraseFromParent();

  // Create a global constructor to register NewModuleStatsGV.
  auto F = Function::Create(FunctionType::get(VoidTy, false),
                            GlobalValue::InternalLinkage, "", M);
  auto BB = BasicBlock::Create(M->getContext(), "", F);
  IRBuilder<> B(BB);

  FunctionType *StatInitTy = FunctionType::get(VoidTy, Int8PtrTy, false);
  FunctionCallee StatInit =
      M->getOrInsertFunction("__sanitizer_stat_init", StatInitTy);

  B.CreateCall(StatInit, NewModuleStatsGV);
  B.CreateRetVoid();

  appendToGlobalCtors(*M, F, 0);
}
