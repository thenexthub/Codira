//===---------- speculation.cpp - Utilities for Speculation ----------===//
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

#include "vm/core/ExecutionEngine/Orc/Speculation.h"

#include "vm/core/ExecutionEngine/Orc/AbsoluteSymbols.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Verifier.h"

namespace vm::core {

namespace orc {

// ImplSymbolMap methods
void ImplSymbolMap::trackImpls(SymbolAliasMap ImplMaps, JITDylib *SrcJD) {
  assert(SrcJD && "Tracking on Null Source .impl dylib");
  std::lock_guard<std::mutex> Lockit(ConcurrentAccess);
  for (auto &I : ImplMaps) {
    auto It = Maps.insert({I.first, {I.second.Aliasee, SrcJD}});
    // check rationale when independent dylibs have same symbol name?
    assert(It.second && "ImplSymbols are already tracked for this Symbol?");
    (void)(It);
  }
}

// Trigger Speculative Compiles.
void Speculator::speculateForEntryPoint(Speculator *Ptr, uint64_t StubId) {
  assert(Ptr && " Null Address Received in orc_speculate_for ");
  Ptr->speculateFor(ExecutorAddr(StubId));
}

Error Speculator::addSpeculationRuntime(JITDylib &JD,
                                        MangleAndInterner &Mangle) {
  ExecutorSymbolDef ThisPtr(ExecutorAddr::fromPtr(this),
                            JITSymbolFlags::Exported);
  ExecutorSymbolDef SpeculateForEntryPtr(
      ExecutorAddr::fromPtr(&speculateForEntryPoint), JITSymbolFlags::Exported);
  return JD.define(absoluteSymbols({
      {Mangle("__orc_speculator"), ThisPtr},                // Data Symbol
      {Mangle("__orc_speculate_for"), SpeculateForEntryPtr} // Callable Symbol
  }));
}

// If two modules, share the same LLVMContext, different threads must
// not access them concurrently without locking the associated LLVMContext
// this implementation follows this contract.
void IRSpeculationLayer::emit(std::unique_ptr<MaterializationResponsibility> R,
                              ThreadSafeModule TSM) {

  assert(TSM && "Speculation Layer received Null Module ?");

  // Instrumentation of runtime calls, lock the Module
  TSM.withModuleDo([this, &R](Module &M) {
    auto &MContext = M.getContext();
    auto SpeculatorVTy = StructType::create(MContext, "Class.Speculator");
    auto RuntimeCallTy = FunctionType::get(
        Type::getVoidTy(MContext),
        {PointerType::getUnqual(MContext), Type::getInt64Ty(MContext)}, false);
    auto RuntimeCall =
        Function::Create(RuntimeCallTy, Function::LinkageTypes::ExternalLinkage,
                         "__orc_speculate_for", &M);
    auto SpeclAddr = new GlobalVariable(
        M, SpeculatorVTy, false, GlobalValue::LinkageTypes::ExternalLinkage,
        nullptr, "__orc_speculator");

    IRBuilder<> Mutator(MContext);

    // QueryAnalysis allowed to transform the IR source, one such example is
    // Simplify CFG helps the static branch prediction heuristics!
    for (auto &Fn : M.getFunctionList()) {
      if (!Fn.isDeclaration()) {

        auto IRNames = QueryAnalysis(Fn);
        // Instrument and register if Query has result
        if (IRNames) {

          // Emit globals for each function.
          auto LoadValueTy = Type::getInt8Ty(MContext);
          auto SpeculatorGuard = new GlobalVariable(
              M, LoadValueTy, false, GlobalValue::LinkageTypes::InternalLinkage,
              ConstantInt::get(LoadValueTy, 0),
              "__orc_speculate.guard.for." + Fn.getName());
          SpeculatorGuard->setAlignment(Align(1));
          SpeculatorGuard->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);

          BasicBlock &ProgramEntry = Fn.getEntryBlock();
          // Create BasicBlocks before the program's entry basicblock
          BasicBlock *SpeculateBlock = BasicBlock::Create(
              MContext, "__orc_speculate.block", &Fn, &ProgramEntry);
          BasicBlock *SpeculateDecisionBlock = BasicBlock::Create(
              MContext, "__orc_speculate.decision.block", &Fn, SpeculateBlock);

          assert(SpeculateDecisionBlock == &Fn.getEntryBlock() &&
                 "SpeculateDecisionBlock not updated?");
          Mutator.SetInsertPoint(SpeculateDecisionBlock);

          auto LoadGuard =
              Mutator.CreateLoad(LoadValueTy, SpeculatorGuard, "guard.value");
          // if just loaded value equal to 0,return true.
          auto CanSpeculate =
              Mutator.CreateICmpEQ(LoadGuard, ConstantInt::get(LoadValueTy, 0),
                                   "compare.to.speculate");
          Mutator.CreateCondBr(CanSpeculate, SpeculateBlock, &ProgramEntry);

          Mutator.SetInsertPoint(SpeculateBlock);
          auto ImplAddrToUint =
              Mutator.CreatePtrToInt(&Fn, Type::getInt64Ty(MContext));
          Mutator.CreateCall(RuntimeCallTy, RuntimeCall,
                             {SpeclAddr, ImplAddrToUint});
          Mutator.CreateStore(ConstantInt::get(LoadValueTy, 1),
                              SpeculatorGuard);
          Mutator.CreateBr(&ProgramEntry);

          assert(Mutator.GetInsertBlock()->getParent() == &Fn &&
                 "IR builder association mismatch?");
          S.registerSymbols(internToJITSymbols(*IRNames),
                            &R->getTargetJITDylib());
        }
      }
    }
  });

  assert(!TSM.withModuleDo([](const Module &M) { return verifyModule(M); }) &&
         "Speculation Instrumentation breaks IR?");

  NextLayer.emit(std::move(R), std::move(TSM));
}

} // namespace orc
} // namespace vm::core
