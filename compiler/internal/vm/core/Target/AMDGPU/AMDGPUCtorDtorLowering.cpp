//===-- AMDGPUCtorDtorLowering.cpp - Handle global ctors and dtors --------===//
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
/// This pass creates a unified init and fini kernel with the required metadata
//===----------------------------------------------------------------------===//

#include "AMDGPUCtorDtorLowering.h"
#include "AMDGPU.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "amdgpu-lower-ctor-dtor"

namespace {

static Function *createInitOrFiniKernelFunction(Module &M, bool IsCtor) {
  StringRef InitOrFiniKernelName = "amdgcn.device.init";
  if (!IsCtor)
    InitOrFiniKernelName = "amdgcn.device.fini";
  if (M.getFunction(InitOrFiniKernelName))
    return nullptr;

  Function *InitOrFiniKernel = Function::createWithDefaultAttr(
      FunctionType::get(Type::getVoidTy(M.getContext()), false),
      GlobalValue::WeakODRLinkage, 0, InitOrFiniKernelName, &M);
  InitOrFiniKernel->setCallingConv(CallingConv::AMDGPU_KERNEL);
  InitOrFiniKernel->addFnAttr("amdgpu-flat-work-group-size", "1,1");
  if (IsCtor)
    InitOrFiniKernel->addFnAttr("device-init");
  else
    InitOrFiniKernel->addFnAttr("device-fini");
  return InitOrFiniKernel;
}

// The linker will provide the associated symbols to allow us to traverse the
// global constructors / destructors in priority order. We create the IR
// required to call each callback in this section. This is equivalent to the
// following code.
//
// extern "C" void * __init_array_start[];
// extern "C" void * __init_array_end[];
// extern "C" void * __fini_array_start[];
// extern "C" void * __fini_array_end[];
//
// using InitCallback = void();
// using FiniCallback = void(void);
//
// void call_init_array_callbacks() {
//   for (auto start = __init_array_start; start != __init_array_end; ++start)
//     reinterpret_cast<InitCallback *>(*start)();
// }
//
// void call_fini_array_callbacks() {
//  size_t fini_array_size = __fini_array_end - __fini_array_start;
//  for (size_t i = fini_array_size; i > 0; --i)
//    reinterpret_cast<FiniCallback *>(__fini_array_start[i - 1])();
// }
static void createInitOrFiniCalls(Function &F, bool IsCtor) {
  Module &M = *F.getParent();
  LLVMContext &C = M.getContext();

  IRBuilder<> IRB(BasicBlock::Create(C, "entry", &F));
  auto *LoopBB = BasicBlock::Create(C, "while.entry", &F);
  auto *ExitBB = BasicBlock::Create(C, "while.end", &F);
  Type *PtrTy = IRB.getPtrTy(AMDGPUAS::GLOBAL_ADDRESS);
  ArrayType *PtrArrayTy = ArrayType::get(PtrTy, 0);

  auto *Begin = M.getOrInsertGlobal(
      IsCtor ? "__init_array_start" : "__fini_array_start", PtrArrayTy, [&]() {
        return new GlobalVariable(
            M, PtrArrayTy,
            /*isConstant=*/true, GlobalValue::ExternalLinkage,
            /*Initializer=*/nullptr,
            IsCtor ? "__init_array_start" : "__fini_array_start",
            /*InsertBefore=*/nullptr, GlobalVariable::NotThreadLocal,
            /*AddressSpace=*/AMDGPUAS::GLOBAL_ADDRESS);
      });
  auto *End = M.getOrInsertGlobal(
      IsCtor ? "__init_array_end" : "__fini_array_end", PtrArrayTy, [&]() {
        return new GlobalVariable(
            M, PtrArrayTy,
            /*isConstant=*/true, GlobalValue::ExternalLinkage,
            /*Initializer=*/nullptr,
            IsCtor ? "__init_array_end" : "__fini_array_end",
            /*InsertBefore=*/nullptr, GlobalVariable::NotThreadLocal,
            /*AddressSpace=*/AMDGPUAS::GLOBAL_ADDRESS);
      });

  // The constructor type is suppoed to allow using the argument vectors, but
  // for now we just call them with no arguments.
  auto *CallBackTy = FunctionType::get(IRB.getVoidTy(), {});

  Value *Start = Begin;
  Value *Stop = End;
  // The destructor array must be called in reverse order. Get a constant
  // expression to the end of the array and iterate backwards instead.
  if (!IsCtor) {
    Type *Int64Ty = IntegerType::getInt64Ty(C);
    auto *EndPtr = IRB.CreatePtrToInt(End, Int64Ty);
    auto *BeginPtr = IRB.CreatePtrToInt(Begin, Int64Ty);
    auto *ByteSize = IRB.CreateSub(EndPtr, BeginPtr, "", /*HasNUW=*/true,
                                   /*HasNSW=*/true);
    auto *Size = IRB.CreateAShr(ByteSize, ConstantInt::get(Int64Ty, 3), "",
                                /*isExact=*/true);
    auto *Offset =
        IRB.CreateSub(Size, ConstantInt::get(Int64Ty, 1), "", /*HasNUW=*/true,
                      /*HasNSW=*/true);
    Start = IRB.CreateInBoundsGEP(
        PtrArrayTy, Begin,
        ArrayRef<Value *>({ConstantInt::get(Int64Ty, 0), Offset}));
    Stop = Begin;
  }

  IRB.CreateCondBr(
      IRB.CreateCmp(IsCtor ? ICmpInst::ICMP_NE : ICmpInst::ICMP_UGE, Start,
                    Stop),
      LoopBB, ExitBB);
  IRB.SetInsertPoint(LoopBB);
  auto *CallBackPHI = IRB.CreatePHI(PtrTy, 2, "ptr");
  auto *CallBack = IRB.CreateLoad(F.getType(), CallBackPHI, "callback");
  IRB.CreateCall(CallBackTy, CallBack);
  auto *NewCallBack =
      IRB.CreateConstGEP1_64(PtrTy, CallBackPHI, IsCtor ? 1 : -1, "next");
  auto *EndCmp = IRB.CreateCmp(IsCtor ? ICmpInst::ICMP_EQ : ICmpInst::ICMP_ULT,
                               NewCallBack, Stop, "end");
  CallBackPHI->addIncoming(Start, &F.getEntryBlock());
  CallBackPHI->addIncoming(NewCallBack, LoopBB);
  IRB.CreateCondBr(EndCmp, ExitBB, LoopBB);
  IRB.SetInsertPoint(ExitBB);
  IRB.CreateRetVoid();
}

static bool createInitOrFiniKernel(Module &M, StringRef GlobalName,
                                   bool IsCtor) {
  GlobalVariable *GV = M.getGlobalVariable(GlobalName);
  if (!GV || !GV->hasInitializer())
    return false;
  ConstantArray *GA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!GA || GA->getNumOperands() == 0)
    return false;

  Function *InitOrFiniKernel = createInitOrFiniKernelFunction(M, IsCtor);
  if (!InitOrFiniKernel)
    return false;

  createInitOrFiniCalls(*InitOrFiniKernel, IsCtor);

  appendToUsed(M, {InitOrFiniKernel});
  return true;
}

static bool lowerCtorsAndDtors(Module &M) {
  bool Modified = false;
  Modified |= createInitOrFiniKernel(M, "toolchain.global_ctors", /*IsCtor =*/true);
  Modified |= createInitOrFiniKernel(M, "toolchain.global_dtors", /*IsCtor =*/false);
  return Modified;
}

class AMDGPUCtorDtorLoweringLegacy final : public ModulePass {
public:
  static char ID;
  AMDGPUCtorDtorLoweringLegacy() : ModulePass(ID) {}
  bool runOnModule(Module &M) override { return lowerCtorsAndDtors(M); }
};

} // End anonymous namespace

PreservedAnalyses AMDGPUCtorDtorLoweringPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  return lowerCtorsAndDtors(M) ? PreservedAnalyses::none()
                               : PreservedAnalyses::all();
}

char AMDGPUCtorDtorLoweringLegacy::ID = 0;
char &toolchain::AMDGPUCtorDtorLoweringLegacyPassID =
    AMDGPUCtorDtorLoweringLegacy::ID;
INITIALIZE_PASS(AMDGPUCtorDtorLoweringLegacy, DEBUG_TYPE,
                "Lower ctors and dtors for AMDGPU", false, false)

ModulePass *toolchain::createAMDGPUCtorDtorLoweringLegacyPass() {
  return new AMDGPUCtorDtorLoweringLegacy();
}
