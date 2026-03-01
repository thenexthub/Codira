//===-- NVPTXCtorDtorLowering.cpp - Handle global ctors and dtors --------===//
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

#include "NVPTXCtorDtorLowering.h"
#include "MCTargetDesc/NVPTXBaseInfo.h"
#include "NVPTX.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/IR/CallingConv.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/MD5.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "nvptx-lower-ctor-dtor"

static cl::opt<std::string>
    GlobalStr("nvptx-lower-global-ctor-dtor-id",
              cl::desc("Override unique ID of ctor/dtor globals."),
              cl::init(""), cl::Hidden);

static cl::opt<bool>
    CreateKernels("nvptx-emit-init-fini-kernel",
                  cl::desc("Emit kernels to call ctor/dtor globals."),
                  cl::init(true), cl::Hidden);

namespace {

static std::string getHash(StringRef Str) {
  toolchain::MD5 Hasher;
  toolchain::MD5::MD5Result Hash;
  Hasher.update(Str);
  Hasher.final(Hash);
  return toolchain::utohexstr(Hash.low(), /*LowerCase=*/true);
}

static void addKernelAttrs(Function *F) {
  F->addFnAttr("nvvm.maxclusterrank", "1");
  F->addFnAttr("nvvm.maxntid", "1");
  F->setCallingConv(CallingConv::PTX_Kernel);
}

static Function *createInitOrFiniKernelFunction(Module &M, bool IsCtor) {
  StringRef InitOrFiniKernelName =
      IsCtor ? "nvptx$device$init" : "nvptx$device$fini";
  if (M.getFunction(InitOrFiniKernelName))
    return nullptr;

  Function *InitOrFiniKernel = Function::createWithDefaultAttr(
      FunctionType::get(Type::getVoidTy(M.getContext()), false),
      GlobalValue::WeakODRLinkage, 0, InitOrFiniKernelName, &M);
  addKernelAttrs(InitOrFiniKernel);

  return InitOrFiniKernel;
}

// We create the IR required to call each callback in this section. This is
// equivalent to the following code. Normally, the linker would provide us with
// the definitions of the init and fini array sections. The 'nvlink' linker does
// not do this so initializing these values is done by the runtime.
//
// extern "C" void **__init_array_start = nullptr;
// extern "C" void **__init_array_end = nullptr;
// extern "C" void **__fini_array_start = nullptr;
// extern "C" void **__fini_array_end = nullptr;
//
// using InitCallback = void();
// using FiniCallback = void();
//
// void call_init_array_callbacks() {
//   for (auto start = __init_array_start; start != __init_array_end; ++start)
//     reinterpret_cast<InitCallback *>(*start)();
// }
//
// void call_fini_array_callbacks() {
//   size_t fini_array_size = __fini_array_end - __fini_array_start;
//   for (size_t i = fini_array_size; i > 0; --i)
//     reinterpret_cast<FiniCallback *>(__fini_array_start[i - 1])();
// }
static void createInitOrFiniCalls(Function &F, bool IsCtor) {
  Module &M = *F.getParent();
  LLVMContext &C = M.getContext();

  IRBuilder<> IRB(BasicBlock::Create(C, "entry", &F));
  auto *LoopBB = BasicBlock::Create(C, "while.entry", &F);
  auto *ExitBB = BasicBlock::Create(C, "while.end", &F);
  Type *PtrTy = IRB.getPtrTy(toolchain::ADDRESS_SPACE_GLOBAL);

  auto *Begin = M.getOrInsertGlobal(
      IsCtor ? "__init_array_start" : "__fini_array_start",
      PointerType::get(C, 0), [&]() {
        auto *GV = new GlobalVariable(
            M, PointerType::get(C, 0),
            /*isConstant=*/false, GlobalValue::WeakAnyLinkage,
            Constant::getNullValue(PointerType::get(C, 0)),
            IsCtor ? "__init_array_start" : "__fini_array_start",
            /*InsertBefore=*/nullptr, GlobalVariable::NotThreadLocal,
            /*AddressSpace=*/toolchain::ADDRESS_SPACE_GLOBAL);
        GV->setVisibility(GlobalVariable::ProtectedVisibility);
        return GV;
      });
  auto *End = M.getOrInsertGlobal(
      IsCtor ? "__init_array_end" : "__fini_array_end", PointerType::get(C, 0),
      [&]() {
        auto *GV = new GlobalVariable(
            M, PointerType::get(C, 0),
            /*isConstant=*/false, GlobalValue::WeakAnyLinkage,
            Constant::getNullValue(PointerType::get(C, 0)),
            IsCtor ? "__init_array_end" : "__fini_array_end",
            /*InsertBefore=*/nullptr, GlobalVariable::NotThreadLocal,
            /*AddressSpace=*/toolchain::ADDRESS_SPACE_GLOBAL);
        GV->setVisibility(GlobalVariable::ProtectedVisibility);
        return GV;
      });

  // The constructor type is suppoed to allow using the argument vectors, but
  // for now we just call them with no arguments.
  auto *CallBackTy = FunctionType::get(IRB.getVoidTy(), {});

  // The destructor array must be called in reverse order. Get an expression to
  // the end of the array and iterate backwards in that case.
  Value *BeginVal = IRB.CreateLoad(Begin->getType(), Begin, "begin");
  Value *EndVal = IRB.CreateLoad(Begin->getType(), End, "stop");
  if (!IsCtor) {
    auto *BeginInt = IRB.CreatePtrToInt(BeginVal, IntegerType::getInt64Ty(C));
    auto *EndInt = IRB.CreatePtrToInt(EndVal, IntegerType::getInt64Ty(C));
    auto *SubInst = IRB.CreateSub(EndInt, BeginInt);
    auto *Offset = IRB.CreateAShr(
        SubInst, ConstantInt::get(IntegerType::getInt64Ty(C), 3), "offset",
        /*IsExact=*/true);
    auto *ValuePtr = IRB.CreateGEP(PointerType::get(C, 0), BeginVal,
                                   ArrayRef<Value *>({Offset}));
    EndVal = BeginVal;
    BeginVal =
        IRB.CreateInBoundsGEP(PointerType::get(C, 0), ValuePtr,
                              ArrayRef<Value *>(ConstantInt::getAllOnesValue(
                                  IntegerType::getInt64Ty(C))),
                              "start");
  }
  IRB.CreateCondBr(
      IRB.CreateCmp(IsCtor ? ICmpInst::ICMP_NE : ICmpInst::ICMP_UGE, BeginVal,
                    EndVal),
      LoopBB, ExitBB);
  IRB.SetInsertPoint(LoopBB);
  auto *CallBackPHI = IRB.CreatePHI(PtrTy, 2, "ptr");
  auto *CallBack = IRB.CreateLoad(IRB.getPtrTy(F.getAddressSpace()),
                                  CallBackPHI, "callback");
  IRB.CreateCall(CallBackTy, CallBack);
  auto *NewCallBack =
      IRB.CreateConstGEP1_64(PtrTy, CallBackPHI, IsCtor ? 1 : -1, "next");
  auto *EndCmp = IRB.CreateCmp(IsCtor ? ICmpInst::ICMP_EQ : ICmpInst::ICMP_ULT,
                               NewCallBack, EndVal, "end");
  CallBackPHI->addIncoming(BeginVal, &F.getEntryBlock());
  CallBackPHI->addIncoming(NewCallBack, LoopBB);
  IRB.CreateCondBr(EndCmp, ExitBB, LoopBB);
  IRB.SetInsertPoint(ExitBB);
  IRB.CreateRetVoid();
}

static bool createInitOrFiniGlobals(Module &M, GlobalVariable *GV,
                                    bool IsCtor) {
  ConstantArray *GA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!GA || GA->getNumOperands() == 0)
    return false;

  // NVPTX has no way to emit variables at specific sections or support for
  // the traditional constructor sections. Instead, we emit mangled global
  // names so the runtime can build the list manually.
  for (Value *V : GA->operands()) {
    auto *CS = cast<ConstantStruct>(V);
    auto *F = cast<Constant>(CS->getOperand(1));
    uint64_t Priority = cast<ConstantInt>(CS->getOperand(0))->getSExtValue();
    std::string PriorityStr = "." + std::to_string(Priority);
    // We append a semi-unique hash and the priority to the global name.
    std::string GlobalID =
        !GlobalStr.empty() ? GlobalStr : getHash(M.getSourceFileName());
    std::string NameStr =
        ((IsCtor ? "__init_array_object_" : "__fini_array_object_") +
         F->getName() + "_" + GlobalID + "_" + std::to_string(Priority))
            .str();
    // PTX does not support exported names with '.' in them.
    toolchain::transform(NameStr, NameStr.begin(),
                    [](char c) { return c == '.' ? '_' : c; });

    auto *GV = new GlobalVariable(M, F->getType(), /*IsConstant=*/true,
                                  GlobalValue::ExternalLinkage, F, NameStr,
                                  nullptr, GlobalValue::NotThreadLocal,
                                  /*AddressSpace=*/4);
    // This isn't respected by Nvidia, simply put here for clarity.
    GV->setSection(IsCtor ? ".init_array" + PriorityStr
                          : ".fini_array" + PriorityStr);
    GV->setVisibility(GlobalVariable::ProtectedVisibility);
    appendToUsed(M, {GV});
  }

  return true;
}

static bool createInitOrFiniKernel(Module &M, StringRef GlobalName,
                                   bool IsCtor) {
  GlobalVariable *GV = M.getGlobalVariable(GlobalName);
  if (!GV || !GV->hasInitializer())
    return false;

  if (!createInitOrFiniGlobals(M, GV, IsCtor))
    return false;

  if (!CreateKernels)
    return true;

  Function *InitOrFiniKernel = createInitOrFiniKernelFunction(M, IsCtor);
  if (!InitOrFiniKernel)
    return false;

  createInitOrFiniCalls(*InitOrFiniKernel, IsCtor);

  GV->eraseFromParent();
  return true;
}

static bool lowerCtorsAndDtors(Module &M) {
  bool Modified = false;
  Modified |= createInitOrFiniKernel(M, "toolchain.global_ctors", /*IsCtor =*/true);
  Modified |= createInitOrFiniKernel(M, "toolchain.global_dtors", /*IsCtor =*/false);
  return Modified;
}

class NVPTXCtorDtorLoweringLegacy final : public ModulePass {
public:
  static char ID;
  NVPTXCtorDtorLoweringLegacy() : ModulePass(ID) {}
  bool runOnModule(Module &M) override { return lowerCtorsAndDtors(M); }
};

} // End anonymous namespace

PreservedAnalyses NVPTXCtorDtorLoweringPass::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  return lowerCtorsAndDtors(M) ? PreservedAnalyses::none()
                               : PreservedAnalyses::all();
}

char NVPTXCtorDtorLoweringLegacy::ID = 0;
INITIALIZE_PASS(NVPTXCtorDtorLoweringLegacy, DEBUG_TYPE,
                "Lower ctors and dtors for NVPTX", false, false)

ModulePass *toolchain::createNVPTXCtorDtorLoweringLegacyPass() {
  return new NVPTXCtorDtorLoweringLegacy();
}
