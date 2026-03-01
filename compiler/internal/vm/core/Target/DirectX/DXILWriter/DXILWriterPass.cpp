//===- DXILWriterPass.cpp - Bitcode writing pass --------------------------===//
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
// DXILWriterPass implementation.
//
//===----------------------------------------------------------------------===//

#include "DXILWriterPass.h"
#include "DXILBitcodeWriter.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Analysis/ModuleSummaryAnalysis.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Alignment.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;
using namespace vm::core::dxil;

namespace {
class WriteDXILPass : public toolchain::ModulePass {
  raw_ostream &OS; // raw_ostream to print on

public:
  static char ID; // Pass identification, replacement for typeid
  WriteDXILPass() : ModulePass(ID), OS(dbgs()) {
    initializeWriteDXILPassPass(*PassRegistry::getPassRegistry());
  }

  explicit WriteDXILPass(raw_ostream &o) : ModulePass(ID), OS(o) {
    initializeWriteDXILPassPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "Bitcode Writer"; }

  bool runOnModule(Module &M) override {
    WriteDXILToFile(M, OS);
    return false;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

static void legalizeLifetimeIntrinsics(Module &M) {
  LLVMContext &Ctx = M.getContext();
  Type *I64Ty = IntegerType::get(Ctx, 64);
  Type *PtrTy = PointerType::get(Ctx, 0);
  Intrinsic::ID LifetimeIIDs[2] = {Intrinsic::lifetime_start,
                                   Intrinsic::lifetime_end};
  for (Intrinsic::ID &IID : LifetimeIIDs) {
    Function *F = M.getFunction(Intrinsic::getName(IID, {PtrTy}, &M));
    if (!F)
      continue;

    // Get or insert an LLVM 3.7-compliant lifetime intrinsic function of the
    // form `void @toolchain.lifetime.[start/end](i64, ptr)` with the NoUnwind
    // attribute
    AttributeList Attr;
    Attr = Attr.addFnAttribute(Ctx, Attribute::NoUnwind);
    FunctionCallee LifetimeCallee = M.getOrInsertFunction(
        Intrinsic::getBaseName(IID), Attr, Type::getVoidTy(Ctx), I64Ty, PtrTy);

    // Replace all calls to lifetime intrinsics with calls to the
    // LLVM 3.7-compliant version of the lifetime intrinsic
    for (User *U : make_early_inc_range(F->users())) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI &&
             "Expected user of a lifetime intrinsic function to be a CallInst");

      // LLVM 3.7 lifetime intrinics require an i8* operand, so we insert
      // a bitcast to ensure that is the case
      Value *PtrOperand = CI->getArgOperand(0);
      PointerType *PtrOpPtrTy = cast<PointerType>(PtrOperand->getType());
      Value *NoOpBitCast = CastInst::Create(Instruction::BitCast, PtrOperand,
                                            PtrOpPtrTy, "", CI->getIterator());

      // LLVM 3.7 lifetime intrinsics have an explicit size operand, whose value
      // we can obtain from the pointer operand which must be an AllocaInst (as
      // of https://github.com/toolchain/toolchain-project/pull/149310)
      AllocaInst *AI = dyn_cast<AllocaInst>(PtrOperand);
      assert(AI &&
             "The pointer operand of a lifetime intrinsic call must be an "
             "AllocaInst");
      std::optional<TypeSize> AllocSize =
          AI->getAllocationSize(CI->getDataLayout());
      assert(AllocSize.has_value() &&
             "Expected the allocation size of AllocaInst to be known");
      CallInst *NewCI = CallInst::Create(
          LifetimeCallee,
          {ConstantInt::get(I64Ty, AllocSize.value().getFixedValue()),
           NoOpBitCast},
          "", CI->getIterator());
      for (Attribute ParamAttr : CI->getParamAttributes(0))
        NewCI->addParamAttr(1, ParamAttr);

      CI->eraseFromParent();
    }

    F->eraseFromParent();
  }
}

static void removeLifetimeIntrinsics(Module &M) {
  Intrinsic::ID LifetimeIIDs[2] = {Intrinsic::lifetime_start,
                                   Intrinsic::lifetime_end};
  for (Intrinsic::ID &IID : LifetimeIIDs) {
    Function *F = M.getFunction(Intrinsic::getBaseName(IID));
    if (!F)
      continue;

    for (User *U : make_early_inc_range(F->users())) {
      CallInst *CI = dyn_cast<CallInst>(U);
      assert(CI && "Expected user of lifetime function to be a CallInst");
      BitCastInst *BCI = dyn_cast<BitCastInst>(CI->getArgOperand(1));
      assert(BCI && "Expected pointer operand of CallInst to be a BitCastInst");
      CI->eraseFromParent();
      BCI->eraseFromParent();
    }
    F->eraseFromParent();
  }
}

class EmbedDXILPass : public toolchain::ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  EmbedDXILPass() : ModulePass(ID) {
    initializeEmbedDXILPassPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "DXIL Embedder"; }

  bool runOnModule(Module &M) override {
    std::string Data;
    toolchain::raw_string_ostream OS(Data);

    // Perform late legalization of lifetime intrinsics that would otherwise
    // fail the Module Verifier if performed in an earlier pass
    legalizeLifetimeIntrinsics(M);

    WriteDXILToFile(M, OS);

    // We no longer need lifetime intrinsics after bitcode serialization, so we
    // simply remove them to keep the Module Verifier happy after our
    // not-so-legal legalizations
    removeLifetimeIntrinsics(M);

    Constant *ModuleConstant =
        ConstantDataArray::get(M.getContext(), arrayRefFromStringRef(Data));
    auto *GV = new toolchain::GlobalVariable(M, ModuleConstant->getType(), true,
                                        GlobalValue::PrivateLinkage,
                                        ModuleConstant, "dx.dxil");
    GV->setSection("DXIL");
    GV->setAlignment(Align(4));
    appendToCompilerUsed(M, {GV});
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};
} // namespace

char WriteDXILPass::ID = 0;
INITIALIZE_PASS_BEGIN(WriteDXILPass, "dxil-write-bitcode", "Write Bitcode",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(ModuleSummaryIndexWrapperPass)
INITIALIZE_PASS_END(WriteDXILPass, "dxil-write-bitcode", "Write Bitcode", false,
                    true)

ModulePass *toolchain::createDXILWriterPass(raw_ostream &Str) {
  return new WriteDXILPass(Str);
}

char EmbedDXILPass::ID = 0;
INITIALIZE_PASS(EmbedDXILPass, "dxil-embed", "Embed DXIL", false, true)

ModulePass *toolchain::createDXILEmbedderPass() { return new EmbedDXILPass(); }
