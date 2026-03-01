//===- Context.cpp - The Context class of Sandbox IR ----------------------===//
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

#include "vm/core/SandboxIR/Context.h"
#include "vm/core/IR/InlineAsm.h"
#include "vm/core/SandboxIR/Function.h"
#include "vm/core/SandboxIR/Instruction.h"
#include "vm/core/SandboxIR/Module.h"

namespace vm::core::sandboxir {

std::unique_ptr<Value> Context::detachLLVMValue(toolchain::Value *V) {
  std::unique_ptr<Value> Erased;
  auto It = LLVMValueToValueMap.find(V);
  if (It != LLVMValueToValueMap.end()) {
    auto *Val = It->second.release();
    Erased = std::unique_ptr<Value>(Val);
    LLVMValueToValueMap.erase(It);
  }
  return Erased;
}

std::unique_ptr<Value> Context::detach(Value *V) {
  assert(V->getSubclassID() != Value::ClassID::Constant &&
         "Can't detach a constant!");
  assert(V->getSubclassID() != Value::ClassID::User && "Can't detach a user!");
  return detachLLVMValue(V->Val);
}

Value *Context::registerValue(std::unique_ptr<Value> &&VPtr) {
  assert(VPtr->getSubclassID() != Value::ClassID::User &&
         "Can't register a user!");

  Value *V = VPtr.get();
  [[maybe_unused]] auto Pair =
      LLVMValueToValueMap.insert({VPtr->Val, std::move(VPtr)});
  assert(Pair.second && "Already exists!");

  // Track creation of instructions.
  // Please note that we don't allow the creation of detached instructions,
  // meaning that the instructions need to be inserted into a block upon
  // creation. This is why the tracker class combines creation and insertion.
  if (auto *I = dyn_cast<Instruction>(V)) {
    getTracker().emplaceIfTracking<CreateAndInsertInst>(I);
    runCreateInstrCallbacks(I);
  }

  return V;
}

Value *Context::getOrCreateValueInternal(toolchain::Value *LLVMV, toolchain::User *U) {
  auto Pair = LLVMValueToValueMap.try_emplace(LLVMV);
  auto It = Pair.first;
  if (!Pair.second)
    return It->second.get();

  // Instruction
  if (auto *LLVMI = dyn_cast<toolchain::Instruction>(LLVMV)) {
    switch (LLVMI->getOpcode()) {
    case toolchain::Instruction::VAArg: {
      auto *LLVMVAArg = cast<toolchain::VAArgInst>(LLVMV);
      It->second = std::unique_ptr<VAArgInst>(new VAArgInst(LLVMVAArg, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Freeze: {
      auto *LLVMFreeze = cast<toolchain::FreezeInst>(LLVMV);
      It->second =
          std::unique_ptr<FreezeInst>(new FreezeInst(LLVMFreeze, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Fence: {
      auto *LLVMFence = cast<toolchain::FenceInst>(LLVMV);
      It->second = std::unique_ptr<FenceInst>(new FenceInst(LLVMFence, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Select: {
      auto *LLVMSel = cast<toolchain::SelectInst>(LLVMV);
      It->second = std::unique_ptr<SelectInst>(new SelectInst(LLVMSel, *this));
      return It->second.get();
    }
    case toolchain::Instruction::ExtractElement: {
      auto *LLVMIns = cast<toolchain::ExtractElementInst>(LLVMV);
      It->second = std::unique_ptr<ExtractElementInst>(
          new ExtractElementInst(LLVMIns, *this));
      return It->second.get();
    }
    case toolchain::Instruction::InsertElement: {
      auto *LLVMIns = cast<toolchain::InsertElementInst>(LLVMV);
      It->second = std::unique_ptr<InsertElementInst>(
          new InsertElementInst(LLVMIns, *this));
      return It->second.get();
    }
    case toolchain::Instruction::ShuffleVector: {
      auto *LLVMIns = cast<toolchain::ShuffleVectorInst>(LLVMV);
      It->second = std::unique_ptr<ShuffleVectorInst>(
          new ShuffleVectorInst(LLVMIns, *this));
      return It->second.get();
    }
    case toolchain::Instruction::ExtractValue: {
      auto *LLVMIns = cast<toolchain::ExtractValueInst>(LLVMV);
      It->second = std::unique_ptr<ExtractValueInst>(
          new ExtractValueInst(LLVMIns, *this));
      return It->second.get();
    }
    case toolchain::Instruction::InsertValue: {
      auto *LLVMIns = cast<toolchain::InsertValueInst>(LLVMV);
      It->second =
          std::unique_ptr<InsertValueInst>(new InsertValueInst(LLVMIns, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Br: {
      auto *LLVMBr = cast<toolchain::BranchInst>(LLVMV);
      It->second = std::unique_ptr<BranchInst>(new BranchInst(LLVMBr, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Load: {
      auto *LLVMLd = cast<toolchain::LoadInst>(LLVMV);
      It->second = std::unique_ptr<LoadInst>(new LoadInst(LLVMLd, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Store: {
      auto *LLVMSt = cast<toolchain::StoreInst>(LLVMV);
      It->second = std::unique_ptr<StoreInst>(new StoreInst(LLVMSt, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Ret: {
      auto *LLVMRet = cast<toolchain::ReturnInst>(LLVMV);
      It->second = std::unique_ptr<ReturnInst>(new ReturnInst(LLVMRet, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Call: {
      auto *LLVMCall = cast<toolchain::CallInst>(LLVMV);
      It->second = std::unique_ptr<CallInst>(new CallInst(LLVMCall, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Invoke: {
      auto *LLVMInvoke = cast<toolchain::InvokeInst>(LLVMV);
      It->second =
          std::unique_ptr<InvokeInst>(new InvokeInst(LLVMInvoke, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CallBr: {
      auto *LLVMCallBr = cast<toolchain::CallBrInst>(LLVMV);
      It->second =
          std::unique_ptr<CallBrInst>(new CallBrInst(LLVMCallBr, *this));
      return It->second.get();
    }
    case toolchain::Instruction::LandingPad: {
      auto *LLVMLPad = cast<toolchain::LandingPadInst>(LLVMV);
      It->second =
          std::unique_ptr<LandingPadInst>(new LandingPadInst(LLVMLPad, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CatchPad: {
      auto *LLVMCPI = cast<toolchain::CatchPadInst>(LLVMV);
      It->second =
          std::unique_ptr<CatchPadInst>(new CatchPadInst(LLVMCPI, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CleanupPad: {
      auto *LLVMCPI = cast<toolchain::CleanupPadInst>(LLVMV);
      It->second =
          std::unique_ptr<CleanupPadInst>(new CleanupPadInst(LLVMCPI, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CatchRet: {
      auto *LLVMCRI = cast<toolchain::CatchReturnInst>(LLVMV);
      It->second =
          std::unique_ptr<CatchReturnInst>(new CatchReturnInst(LLVMCRI, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CleanupRet: {
      auto *LLVMCRI = cast<toolchain::CleanupReturnInst>(LLVMV);
      It->second = std::unique_ptr<CleanupReturnInst>(
          new CleanupReturnInst(LLVMCRI, *this));
      return It->second.get();
    }
    case toolchain::Instruction::GetElementPtr: {
      auto *LLVMGEP = cast<toolchain::GetElementPtrInst>(LLVMV);
      It->second = std::unique_ptr<GetElementPtrInst>(
          new GetElementPtrInst(LLVMGEP, *this));
      return It->second.get();
    }
    case toolchain::Instruction::CatchSwitch: {
      auto *LLVMCatchSwitchInst = cast<toolchain::CatchSwitchInst>(LLVMV);
      It->second = std::unique_ptr<CatchSwitchInst>(
          new CatchSwitchInst(LLVMCatchSwitchInst, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Resume: {
      auto *LLVMResumeInst = cast<toolchain::ResumeInst>(LLVMV);
      It->second =
          std::unique_ptr<ResumeInst>(new ResumeInst(LLVMResumeInst, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Switch: {
      auto *LLVMSwitchInst = cast<toolchain::SwitchInst>(LLVMV);
      It->second =
          std::unique_ptr<SwitchInst>(new SwitchInst(LLVMSwitchInst, *this));
      return It->second.get();
    }
    case toolchain::Instruction::FNeg: {
      auto *LLVMUnaryOperator = cast<toolchain::UnaryOperator>(LLVMV);
      It->second = std::unique_ptr<UnaryOperator>(
          new UnaryOperator(LLVMUnaryOperator, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Add:
    case toolchain::Instruction::FAdd:
    case toolchain::Instruction::Sub:
    case toolchain::Instruction::FSub:
    case toolchain::Instruction::Mul:
    case toolchain::Instruction::FMul:
    case toolchain::Instruction::UDiv:
    case toolchain::Instruction::SDiv:
    case toolchain::Instruction::FDiv:
    case toolchain::Instruction::URem:
    case toolchain::Instruction::SRem:
    case toolchain::Instruction::FRem:
    case toolchain::Instruction::Shl:
    case toolchain::Instruction::LShr:
    case toolchain::Instruction::AShr:
    case toolchain::Instruction::And:
    case toolchain::Instruction::Or:
    case toolchain::Instruction::Xor: {
      auto *LLVMBinaryOperator = cast<toolchain::BinaryOperator>(LLVMV);
      It->second = std::unique_ptr<BinaryOperator>(
          new BinaryOperator(LLVMBinaryOperator, *this));
      return It->second.get();
    }
    case toolchain::Instruction::AtomicRMW: {
      auto *LLVMAtomicRMW = cast<toolchain::AtomicRMWInst>(LLVMV);
      It->second = std::unique_ptr<AtomicRMWInst>(
          new AtomicRMWInst(LLVMAtomicRMW, *this));
      return It->second.get();
    }
    case toolchain::Instruction::AtomicCmpXchg: {
      auto *LLVMAtomicCmpXchg = cast<toolchain::AtomicCmpXchgInst>(LLVMV);
      It->second = std::unique_ptr<AtomicCmpXchgInst>(
          new AtomicCmpXchgInst(LLVMAtomicCmpXchg, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Alloca: {
      auto *LLVMAlloca = cast<toolchain::AllocaInst>(LLVMV);
      It->second =
          std::unique_ptr<AllocaInst>(new AllocaInst(LLVMAlloca, *this));
      return It->second.get();
    }
    case toolchain::Instruction::ZExt:
    case toolchain::Instruction::SExt:
    case toolchain::Instruction::FPToUI:
    case toolchain::Instruction::FPToSI:
    case toolchain::Instruction::FPExt:
    case toolchain::Instruction::PtrToAddr:
    case toolchain::Instruction::PtrToInt:
    case toolchain::Instruction::IntToPtr:
    case toolchain::Instruction::SIToFP:
    case toolchain::Instruction::UIToFP:
    case toolchain::Instruction::Trunc:
    case toolchain::Instruction::FPTrunc:
    case toolchain::Instruction::BitCast:
    case toolchain::Instruction::AddrSpaceCast: {
      auto *LLVMCast = cast<toolchain::CastInst>(LLVMV);
      It->second = std::unique_ptr<CastInst>(new CastInst(LLVMCast, *this));
      return It->second.get();
    }
    case toolchain::Instruction::PHI: {
      auto *LLVMPhi = cast<toolchain::PHINode>(LLVMV);
      It->second = std::unique_ptr<PHINode>(new PHINode(LLVMPhi, *this));
      return It->second.get();
    }
    case toolchain::Instruction::ICmp: {
      auto *LLVMICmp = cast<toolchain::ICmpInst>(LLVMV);
      It->second = std::unique_ptr<ICmpInst>(new ICmpInst(LLVMICmp, *this));
      return It->second.get();
    }
    case toolchain::Instruction::FCmp: {
      auto *LLVMFCmp = cast<toolchain::FCmpInst>(LLVMV);
      It->second = std::unique_ptr<FCmpInst>(new FCmpInst(LLVMFCmp, *this));
      return It->second.get();
    }
    case toolchain::Instruction::Unreachable: {
      auto *LLVMUnreachable = cast<toolchain::UnreachableInst>(LLVMV);
      It->second = std::unique_ptr<UnreachableInst>(
          new UnreachableInst(LLVMUnreachable, *this));
      return It->second.get();
    }
    default:
      break;
    }
    It->second = std::unique_ptr<OpaqueInst>(
        new OpaqueInst(cast<toolchain::Instruction>(LLVMV), *this));
    return It->second.get();
  }
  // Constant
  if (auto *LLVMC = dyn_cast<toolchain::Constant>(LLVMV)) {
    switch (LLVMC->getValueID()) {
    case toolchain::Value::ConstantIntVal:
      It->second = std::unique_ptr<ConstantInt>(
          new ConstantInt(cast<toolchain::ConstantInt>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::ConstantFPVal:
      It->second = std::unique_ptr<ConstantFP>(
          new ConstantFP(cast<toolchain::ConstantFP>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::BlockAddressVal:
      It->second = std::unique_ptr<BlockAddress>(
          new BlockAddress(cast<toolchain::BlockAddress>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::ConstantTokenNoneVal:
      It->second = std::unique_ptr<ConstantTokenNone>(
          new ConstantTokenNone(cast<toolchain::ConstantTokenNone>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::ConstantAggregateZeroVal: {
      auto *CAZ = cast<toolchain::ConstantAggregateZero>(LLVMC);
      It->second = std::unique_ptr<ConstantAggregateZero>(
          new ConstantAggregateZero(CAZ, *this));
      auto *Ret = It->second.get();
      // Must create sandboxir for elements.
      auto EC = CAZ->getElementCount();
      if (EC.isFixed()) {
        for (auto ElmIdx : seq<unsigned>(0, EC.getFixedValue()))
          getOrCreateValueInternal(CAZ->getElementValue(ElmIdx), CAZ);
      }
      return Ret;
    }
    case toolchain::Value::ConstantPointerNullVal:
      It->second = std::unique_ptr<ConstantPointerNull>(new ConstantPointerNull(
          cast<toolchain::ConstantPointerNull>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::PoisonValueVal:
      It->second = std::unique_ptr<PoisonValue>(
          new PoisonValue(cast<toolchain::PoisonValue>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::UndefValueVal:
      It->second = std::unique_ptr<UndefValue>(
          new UndefValue(cast<toolchain::UndefValue>(LLVMC), *this));
      return It->second.get();
    case toolchain::Value::DSOLocalEquivalentVal: {
      auto *DSOLE = cast<toolchain::DSOLocalEquivalent>(LLVMC);
      It->second = std::unique_ptr<DSOLocalEquivalent>(
          new DSOLocalEquivalent(DSOLE, *this));
      auto *Ret = It->second.get();
      getOrCreateValueInternal(DSOLE->getGlobalValue(), DSOLE);
      return Ret;
    }
    case toolchain::Value::ConstantArrayVal:
      It->second = std::unique_ptr<ConstantArray>(
          new ConstantArray(cast<toolchain::ConstantArray>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantStructVal:
      It->second = std::unique_ptr<ConstantStruct>(
          new ConstantStruct(cast<toolchain::ConstantStruct>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantVectorVal:
      It->second = std::unique_ptr<ConstantVector>(
          new ConstantVector(cast<toolchain::ConstantVector>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantDataArrayVal:
      It->second = std::unique_ptr<ConstantDataArray>(
          new ConstantDataArray(cast<toolchain::ConstantDataArray>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantDataVectorVal:
      It->second = std::unique_ptr<ConstantDataVector>(
          new ConstantDataVector(cast<toolchain::ConstantDataVector>(LLVMC), *this));
      break;
    case toolchain::Value::FunctionVal:
      It->second = std::unique_ptr<Function>(
          new Function(cast<toolchain::Function>(LLVMC), *this));
      break;
    case toolchain::Value::GlobalIFuncVal:
      It->second = std::unique_ptr<GlobalIFunc>(
          new GlobalIFunc(cast<toolchain::GlobalIFunc>(LLVMC), *this));
      break;
    case toolchain::Value::GlobalVariableVal:
      It->second = std::unique_ptr<GlobalVariable>(
          new GlobalVariable(cast<toolchain::GlobalVariable>(LLVMC), *this));
      break;
    case toolchain::Value::GlobalAliasVal:
      It->second = std::unique_ptr<GlobalAlias>(
          new GlobalAlias(cast<toolchain::GlobalAlias>(LLVMC), *this));
      break;
    case toolchain::Value::NoCFIValueVal:
      It->second = std::unique_ptr<NoCFIValue>(
          new NoCFIValue(cast<toolchain::NoCFIValue>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantPtrAuthVal:
      It->second = std::unique_ptr<ConstantPtrAuth>(
          new ConstantPtrAuth(cast<toolchain::ConstantPtrAuth>(LLVMC), *this));
      break;
    case toolchain::Value::ConstantExprVal:
      It->second = std::unique_ptr<ConstantExpr>(
          new ConstantExpr(cast<toolchain::ConstantExpr>(LLVMC), *this));
      break;
    default:
      It->second = std::unique_ptr<Constant>(new Constant(LLVMC, *this));
      break;
    }
    auto *NewC = It->second.get();
    for (toolchain::Value *COp : LLVMC->operands())
      getOrCreateValueInternal(COp, LLVMC);
    return NewC;
  }
  // Argument
  if (auto *LLVMArg = dyn_cast<toolchain::Argument>(LLVMV)) {
    It->second = std::unique_ptr<Argument>(new Argument(LLVMArg, *this));
    return It->second.get();
  }
  // BasicBlock
  if (auto *LLVMBB = dyn_cast<toolchain::BasicBlock>(LLVMV)) {
    assert(isa<toolchain::BlockAddress>(U) &&
           "This won't create a SBBB, don't call this function directly!");
    if (auto *SBBB = getValue(LLVMBB))
      return SBBB;
    return nullptr;
  }
  // Metadata
  if (auto *LLVMMD = dyn_cast<toolchain::MetadataAsValue>(LLVMV)) {
    It->second = std::unique_ptr<OpaqueValue>(new OpaqueValue(LLVMMD, *this));
    return It->second.get();
  }
  // InlineAsm
  if (auto *LLVMAsm = dyn_cast<toolchain::InlineAsm>(LLVMV)) {
    It->second = std::unique_ptr<OpaqueValue>(new OpaqueValue(LLVMAsm, *this));
    return It->second.get();
  }
  llvm_unreachable("Unhandled LLVMV type!");
}

Argument *Context::getOrCreateArgument(toolchain::Argument *LLVMArg) {
  auto Pair = LLVMValueToValueMap.try_emplace(LLVMArg);
  auto It = Pair.first;
  if (Pair.second) {
    It->second = std::unique_ptr<Argument>(new Argument(LLVMArg, *this));
    return cast<Argument>(It->second.get());
  }
  return cast<Argument>(It->second.get());
}

Constant *Context::getOrCreateConstant(toolchain::Constant *LLVMC) {
  return cast<Constant>(getOrCreateValueInternal(LLVMC, nullptr));
}

BasicBlock *Context::createBasicBlock(toolchain::BasicBlock *LLVMBB) {
  assert(getValue(LLVMBB) == nullptr && "Already exists!");
  auto NewBBPtr = std::unique_ptr<BasicBlock>(new BasicBlock(LLVMBB, *this));
  auto *BB = cast<BasicBlock>(registerValue(std::move(NewBBPtr)));
  // Create SandboxIR for BB's body.
  BB->buildBasicBlockFromLLVMIR(LLVMBB);
  return BB;
}

VAArgInst *Context::createVAArgInst(toolchain::VAArgInst *SI) {
  auto NewPtr = std::unique_ptr<VAArgInst>(new VAArgInst(SI, *this));
  return cast<VAArgInst>(registerValue(std::move(NewPtr)));
}

FreezeInst *Context::createFreezeInst(toolchain::FreezeInst *SI) {
  auto NewPtr = std::unique_ptr<FreezeInst>(new FreezeInst(SI, *this));
  return cast<FreezeInst>(registerValue(std::move(NewPtr)));
}

FenceInst *Context::createFenceInst(toolchain::FenceInst *SI) {
  auto NewPtr = std::unique_ptr<FenceInst>(new FenceInst(SI, *this));
  return cast<FenceInst>(registerValue(std::move(NewPtr)));
}

SelectInst *Context::createSelectInst(toolchain::SelectInst *SI) {
  auto NewPtr = std::unique_ptr<SelectInst>(new SelectInst(SI, *this));
  return cast<SelectInst>(registerValue(std::move(NewPtr)));
}

ExtractElementInst *
Context::createExtractElementInst(toolchain::ExtractElementInst *EEI) {
  auto NewPtr =
      std::unique_ptr<ExtractElementInst>(new ExtractElementInst(EEI, *this));
  return cast<ExtractElementInst>(registerValue(std::move(NewPtr)));
}

InsertElementInst *
Context::createInsertElementInst(toolchain::InsertElementInst *IEI) {
  auto NewPtr =
      std::unique_ptr<InsertElementInst>(new InsertElementInst(IEI, *this));
  return cast<InsertElementInst>(registerValue(std::move(NewPtr)));
}

ShuffleVectorInst *
Context::createShuffleVectorInst(toolchain::ShuffleVectorInst *SVI) {
  auto NewPtr =
      std::unique_ptr<ShuffleVectorInst>(new ShuffleVectorInst(SVI, *this));
  return cast<ShuffleVectorInst>(registerValue(std::move(NewPtr)));
}

ExtractValueInst *Context::createExtractValueInst(toolchain::ExtractValueInst *EVI) {
  auto NewPtr =
      std::unique_ptr<ExtractValueInst>(new ExtractValueInst(EVI, *this));
  return cast<ExtractValueInst>(registerValue(std::move(NewPtr)));
}

InsertValueInst *Context::createInsertValueInst(toolchain::InsertValueInst *IVI) {
  auto NewPtr =
      std::unique_ptr<InsertValueInst>(new InsertValueInst(IVI, *this));
  return cast<InsertValueInst>(registerValue(std::move(NewPtr)));
}

BranchInst *Context::createBranchInst(toolchain::BranchInst *BI) {
  auto NewPtr = std::unique_ptr<BranchInst>(new BranchInst(BI, *this));
  return cast<BranchInst>(registerValue(std::move(NewPtr)));
}

LoadInst *Context::createLoadInst(toolchain::LoadInst *LI) {
  auto NewPtr = std::unique_ptr<LoadInst>(new LoadInst(LI, *this));
  return cast<LoadInst>(registerValue(std::move(NewPtr)));
}

StoreInst *Context::createStoreInst(toolchain::StoreInst *SI) {
  auto NewPtr = std::unique_ptr<StoreInst>(new StoreInst(SI, *this));
  return cast<StoreInst>(registerValue(std::move(NewPtr)));
}

ReturnInst *Context::createReturnInst(toolchain::ReturnInst *I) {
  auto NewPtr = std::unique_ptr<ReturnInst>(new ReturnInst(I, *this));
  return cast<ReturnInst>(registerValue(std::move(NewPtr)));
}

CallInst *Context::createCallInst(toolchain::CallInst *I) {
  auto NewPtr = std::unique_ptr<CallInst>(new CallInst(I, *this));
  return cast<CallInst>(registerValue(std::move(NewPtr)));
}

InvokeInst *Context::createInvokeInst(toolchain::InvokeInst *I) {
  auto NewPtr = std::unique_ptr<InvokeInst>(new InvokeInst(I, *this));
  return cast<InvokeInst>(registerValue(std::move(NewPtr)));
}

CallBrInst *Context::createCallBrInst(toolchain::CallBrInst *I) {
  auto NewPtr = std::unique_ptr<CallBrInst>(new CallBrInst(I, *this));
  return cast<CallBrInst>(registerValue(std::move(NewPtr)));
}

UnreachableInst *Context::createUnreachableInst(toolchain::UnreachableInst *UI) {
  auto NewPtr =
      std::unique_ptr<UnreachableInst>(new UnreachableInst(UI, *this));
  return cast<UnreachableInst>(registerValue(std::move(NewPtr)));
}
LandingPadInst *Context::createLandingPadInst(toolchain::LandingPadInst *I) {
  auto NewPtr = std::unique_ptr<LandingPadInst>(new LandingPadInst(I, *this));
  return cast<LandingPadInst>(registerValue(std::move(NewPtr)));
}
CatchPadInst *Context::createCatchPadInst(toolchain::CatchPadInst *I) {
  auto NewPtr = std::unique_ptr<CatchPadInst>(new CatchPadInst(I, *this));
  return cast<CatchPadInst>(registerValue(std::move(NewPtr)));
}
CleanupPadInst *Context::createCleanupPadInst(toolchain::CleanupPadInst *I) {
  auto NewPtr = std::unique_ptr<CleanupPadInst>(new CleanupPadInst(I, *this));
  return cast<CleanupPadInst>(registerValue(std::move(NewPtr)));
}
CatchReturnInst *Context::createCatchReturnInst(toolchain::CatchReturnInst *I) {
  auto NewPtr = std::unique_ptr<CatchReturnInst>(new CatchReturnInst(I, *this));
  return cast<CatchReturnInst>(registerValue(std::move(NewPtr)));
}
CleanupReturnInst *
Context::createCleanupReturnInst(toolchain::CleanupReturnInst *I) {
  auto NewPtr =
      std::unique_ptr<CleanupReturnInst>(new CleanupReturnInst(I, *this));
  return cast<CleanupReturnInst>(registerValue(std::move(NewPtr)));
}
GetElementPtrInst *
Context::createGetElementPtrInst(toolchain::GetElementPtrInst *I) {
  auto NewPtr =
      std::unique_ptr<GetElementPtrInst>(new GetElementPtrInst(I, *this));
  return cast<GetElementPtrInst>(registerValue(std::move(NewPtr)));
}
CatchSwitchInst *Context::createCatchSwitchInst(toolchain::CatchSwitchInst *I) {
  auto NewPtr = std::unique_ptr<CatchSwitchInst>(new CatchSwitchInst(I, *this));
  return cast<CatchSwitchInst>(registerValue(std::move(NewPtr)));
}
ResumeInst *Context::createResumeInst(toolchain::ResumeInst *I) {
  auto NewPtr = std::unique_ptr<ResumeInst>(new ResumeInst(I, *this));
  return cast<ResumeInst>(registerValue(std::move(NewPtr)));
}
SwitchInst *Context::createSwitchInst(toolchain::SwitchInst *I) {
  auto NewPtr = std::unique_ptr<SwitchInst>(new SwitchInst(I, *this));
  return cast<SwitchInst>(registerValue(std::move(NewPtr)));
}
UnaryOperator *Context::createUnaryOperator(toolchain::UnaryOperator *I) {
  auto NewPtr = std::unique_ptr<UnaryOperator>(new UnaryOperator(I, *this));
  return cast<UnaryOperator>(registerValue(std::move(NewPtr)));
}
BinaryOperator *Context::createBinaryOperator(toolchain::BinaryOperator *I) {
  auto NewPtr = std::unique_ptr<BinaryOperator>(new BinaryOperator(I, *this));
  return cast<BinaryOperator>(registerValue(std::move(NewPtr)));
}
AtomicRMWInst *Context::createAtomicRMWInst(toolchain::AtomicRMWInst *I) {
  auto NewPtr = std::unique_ptr<AtomicRMWInst>(new AtomicRMWInst(I, *this));
  return cast<AtomicRMWInst>(registerValue(std::move(NewPtr)));
}
AtomicCmpXchgInst *
Context::createAtomicCmpXchgInst(toolchain::AtomicCmpXchgInst *I) {
  auto NewPtr =
      std::unique_ptr<AtomicCmpXchgInst>(new AtomicCmpXchgInst(I, *this));
  return cast<AtomicCmpXchgInst>(registerValue(std::move(NewPtr)));
}
AllocaInst *Context::createAllocaInst(toolchain::AllocaInst *I) {
  auto NewPtr = std::unique_ptr<AllocaInst>(new AllocaInst(I, *this));
  return cast<AllocaInst>(registerValue(std::move(NewPtr)));
}
CastInst *Context::createCastInst(toolchain::CastInst *I) {
  auto NewPtr = std::unique_ptr<CastInst>(new CastInst(I, *this));
  return cast<CastInst>(registerValue(std::move(NewPtr)));
}
PHINode *Context::createPHINode(toolchain::PHINode *I) {
  auto NewPtr = std::unique_ptr<PHINode>(new PHINode(I, *this));
  return cast<PHINode>(registerValue(std::move(NewPtr)));
}
ICmpInst *Context::createICmpInst(toolchain::ICmpInst *I) {
  auto NewPtr = std::unique_ptr<ICmpInst>(new ICmpInst(I, *this));
  return cast<ICmpInst>(registerValue(std::move(NewPtr)));
}
FCmpInst *Context::createFCmpInst(toolchain::FCmpInst *I) {
  auto NewPtr = std::unique_ptr<FCmpInst>(new FCmpInst(I, *this));
  return cast<FCmpInst>(registerValue(std::move(NewPtr)));
}
Value *Context::getValue(toolchain::Value *V) const {
  auto It = LLVMValueToValueMap.find(V);
  if (It != LLVMValueToValueMap.end())
    return It->second.get();
  return nullptr;
}

Context::Context(LLVMContext &LLVMCtx)
    : LLVMCtx(LLVMCtx), IRTracker(*this),
      LLVMIRBuilder(LLVMCtx, ConstantFolder()) {}

Context::~Context() = default;

void Context::clear() {
  // TODO: Ideally we should clear only function-scope objects, and keep global
  // objects, like Constants to avoid recreating them.
  LLVMValueToValueMap.clear();
}

Module *Context::getModule(toolchain::Module *LLVMM) const {
  auto It = LLVMModuleToModuleMap.find(LLVMM);
  if (It != LLVMModuleToModuleMap.end())
    return It->second.get();
  return nullptr;
}

Module *Context::getOrCreateModule(toolchain::Module *LLVMM) {
  auto Pair = LLVMModuleToModuleMap.try_emplace(LLVMM);
  auto It = Pair.first;
  if (!Pair.second)
    return It->second.get();
  It->second = std::unique_ptr<Module>(new Module(*LLVMM, *this));
  return It->second.get();
}

Function *Context::createFunction(toolchain::Function *F) {
  // Create the module if needed before we create the new sandboxir::Function.
  // Note: this won't fully populate the module. The only globals that will be
  // available will be the ones being used within the function.
  getOrCreateModule(F->getParent());

  // There may be a function declaration already defined. Regardless destroy it.
  if (Function *ExistingF = cast_or_null<Function>(getValue(F)))
    detach(ExistingF);

  auto NewFPtr = std::unique_ptr<Function>(new Function(F, *this));
  auto *SBF = cast<Function>(registerValue(std::move(NewFPtr)));
  // Create arguments.
  for (auto &Arg : F->args())
    getOrCreateArgument(&Arg);
  // Create BBs.
  for (auto &BB : *F)
    createBasicBlock(&BB);
  return SBF;
}

Module *Context::createModule(toolchain::Module *LLVMM) {
  auto *M = getOrCreateModule(LLVMM);
  // Create the functions.
  for (auto &LLVMF : *LLVMM)
    createFunction(&LLVMF);
  // Create globals.
  for (auto &Global : LLVMM->globals())
    getOrCreateValue(&Global);
  // Create aliases.
  for (auto &Alias : LLVMM->aliases())
    getOrCreateValue(&Alias);
  // Create ifuncs.
  for (auto &IFunc : LLVMM->ifuncs())
    getOrCreateValue(&IFunc);

  return M;
}

void Context::runEraseInstrCallbacks(Instruction *I) {
  for (const auto &CBEntry : EraseInstrCallbacks)
    CBEntry.second(I);
}

void Context::runCreateInstrCallbacks(Instruction *I) {
  for (auto &CBEntry : CreateInstrCallbacks)
    CBEntry.second(I);
}

void Context::runMoveInstrCallbacks(Instruction *I, const BBIterator &WhereIt) {
  for (auto &CBEntry : MoveInstrCallbacks)
    CBEntry.second(I, WhereIt);
}

void Context::runSetUseCallbacks(const Use &U, Value *NewSrc) {
  for (auto &CBEntry : SetUseCallbacks)
    CBEntry.second(U, NewSrc);
}

// An arbitrary limit, to check for accidental misuse. We expect a small number
// of callbacks to be registered at a time, but we can increase this number if
// we discover we needed more.
[[maybe_unused]] static constexpr int MaxRegisteredCallbacks = 16;

Context::CallbackID Context::registerEraseInstrCallback(EraseInstrCallback CB) {
  assert(EraseInstrCallbacks.size() <= MaxRegisteredCallbacks &&
         "EraseInstrCallbacks size limit exceeded");
  CallbackID ID{NextCallbackID++};
  EraseInstrCallbacks[ID] = CB;
  return ID;
}
void Context::unregisterEraseInstrCallback(CallbackID ID) {
  [[maybe_unused]] bool Erased = EraseInstrCallbacks.erase(ID);
  assert(Erased &&
         "Callback ID not found in EraseInstrCallbacks during deregistration");
}

Context::CallbackID
Context::registerCreateInstrCallback(CreateInstrCallback CB) {
  assert(CreateInstrCallbacks.size() <= MaxRegisteredCallbacks &&
         "CreateInstrCallbacks size limit exceeded");
  CallbackID ID{NextCallbackID++};
  CreateInstrCallbacks[ID] = CB;
  return ID;
}
void Context::unregisterCreateInstrCallback(CallbackID ID) {
  [[maybe_unused]] bool Erased = CreateInstrCallbacks.erase(ID);
  assert(Erased &&
         "Callback ID not found in CreateInstrCallbacks during deregistration");
}

Context::CallbackID Context::registerMoveInstrCallback(MoveInstrCallback CB) {
  assert(MoveInstrCallbacks.size() <= MaxRegisteredCallbacks &&
         "MoveInstrCallbacks size limit exceeded");
  CallbackID ID{NextCallbackID++};
  MoveInstrCallbacks[ID] = CB;
  return ID;
}
void Context::unregisterMoveInstrCallback(CallbackID ID) {
  [[maybe_unused]] bool Erased = MoveInstrCallbacks.erase(ID);
  assert(Erased &&
         "Callback ID not found in MoveInstrCallbacks during deregistration");
}

Context::CallbackID Context::registerSetUseCallback(SetUseCallback CB) {
  assert(SetUseCallbacks.size() <= MaxRegisteredCallbacks &&
         "SetUseCallbacks size limit exceeded");
  CallbackID ID{NextCallbackID++};
  SetUseCallbacks[ID] = CB;
  return ID;
}
void Context::unregisterSetUseCallback(CallbackID ID) {
  [[maybe_unused]] bool Erased = SetUseCallbacks.erase(ID);
  assert(Erased &&
         "Callback ID not found in SetUseCallbacks during deregistration");
}

} // namespace vm::core::sandboxir
