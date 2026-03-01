//===- SPIRVLegalizeImplicitBinding.cpp - Legalize implicit bindings ----*- C++
//-*-===//
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
// This pass legalizes the @toolchain.spv.resource.handlefromimplicitbinding
// intrinsic by replacing it with a call to
// @toolchain.spv.resource.handlefrombinding.
//
//===----------------------------------------------------------------------===//

#include "SPIRV.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/IntrinsicsSPIRV.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Pass.h"
#include <algorithm>
#include <vector>

using namespace vm::core;

namespace {
class SPIRVLegalizeImplicitBinding : public ModulePass {
public:
  static char ID;
  SPIRVLegalizeImplicitBinding() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "SPIRV Legalize Implicit Binding";
  }
  bool runOnModule(Module &M) override;

private:
  void collectBindingInfo(Module &M);
  uint32_t getAndReserveFirstUnusedBinding(uint32_t DescSet);
  void replaceImplicitBindingCalls(Module &M);
  void replaceResourceHandleCall(Module &M, CallInst *OldCI,
                                 uint32_t NewBinding);
  void replaceCounterHandleCall(Module &M, CallInst *OldCI,
                                uint32_t NewBinding);
  void verifyUniqueOrderIdPerResource(SmallVectorImpl<CallInst *> &Calls);

  // A map from descriptor set to a bit vector of used binding numbers.
  std::vector<BitVector> UsedBindings;
  // A list of all implicit binding calls, to be sorted by order ID.
  SmallVector<CallInst *, 16> ImplicitBindingCalls;
};

struct BindingInfoCollector : public InstVisitor<BindingInfoCollector> {
  std::vector<BitVector> &UsedBindings;
  SmallVector<CallInst *, 16> &ImplicitBindingCalls;

  BindingInfoCollector(std::vector<BitVector> &UsedBindings,
                       SmallVector<CallInst *, 16> &ImplicitBindingCalls)
      : UsedBindings(UsedBindings), ImplicitBindingCalls(ImplicitBindingCalls) {
  }

  void addBinding(uint32_t DescSet, uint32_t Binding) {
    if (UsedBindings.size() <= DescSet) {
      UsedBindings.resize(DescSet + 1);
      UsedBindings[DescSet].resize(64);
    }
    if (UsedBindings[DescSet].size() <= Binding) {
      UsedBindings[DescSet].resize(2 * Binding + 1);
    }
    UsedBindings[DescSet].set(Binding);
  }

  void visitCallInst(CallInst &CI) {
    if (CI.getIntrinsicID() == Intrinsic::spv_resource_handlefrombinding) {
      const uint32_t DescSet =
          cast<ConstantInt>(CI.getArgOperand(0))->getZExtValue();
      const uint32_t Binding =
          cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue();
      addBinding(DescSet, Binding);
    } else if (CI.getIntrinsicID() ==
               Intrinsic::spv_resource_handlefromimplicitbinding) {
      ImplicitBindingCalls.push_back(&CI);
    } else if (CI.getIntrinsicID() ==
               Intrinsic::spv_resource_counterhandlefrombinding) {
      const uint32_t DescSet =
          cast<ConstantInt>(CI.getArgOperand(2))->getZExtValue();
      const uint32_t Binding =
          cast<ConstantInt>(CI.getArgOperand(1))->getZExtValue();
      addBinding(DescSet, Binding);
    } else if (CI.getIntrinsicID() ==
               Intrinsic::spv_resource_counterhandlefromimplicitbinding) {
      ImplicitBindingCalls.push_back(&CI);
    }
  }
};

static uint32_t getOrderId(const CallInst *CI) {
  uint32_t OrderIdArgIdx = 0;
  switch (CI->getIntrinsicID()) {
  case Intrinsic::spv_resource_handlefromimplicitbinding:
    OrderIdArgIdx = 0;
    break;
  case Intrinsic::spv_resource_counterhandlefromimplicitbinding:
    OrderIdArgIdx = 1;
    break;
  default:
    llvm_unreachable("CallInst is not an implicit binding intrinsic");
  }
  return cast<ConstantInt>(CI->getArgOperand(OrderIdArgIdx))->getZExtValue();
}

static uint32_t getDescSet(const CallInst *CI) {
  uint32_t DescSetArgIdx;
  switch (CI->getIntrinsicID()) {
  case Intrinsic::spv_resource_handlefromimplicitbinding:
  case Intrinsic::spv_resource_handlefrombinding:
    DescSetArgIdx = 1;
    break;
  case Intrinsic::spv_resource_counterhandlefromimplicitbinding:
  case Intrinsic::spv_resource_counterhandlefrombinding:
    DescSetArgIdx = 2;
    break;
  default:
    llvm_unreachable("CallInst is not an implicit binding intrinsic");
  }
  return cast<ConstantInt>(CI->getArgOperand(DescSetArgIdx))->getZExtValue();
}

void SPIRVLegalizeImplicitBinding::collectBindingInfo(Module &M) {
  BindingInfoCollector InfoCollector(UsedBindings, ImplicitBindingCalls);
  InfoCollector.visit(M);

  // Sort the collected calls by their order ID.
  std::sort(ImplicitBindingCalls.begin(), ImplicitBindingCalls.end(),
            [](const CallInst *A, const CallInst *B) {
              return getOrderId(A) < getOrderId(B);
            });
}

void SPIRVLegalizeImplicitBinding::verifyUniqueOrderIdPerResource(
    SmallVectorImpl<CallInst *> &Calls) {
  // Check that the order Id is unique per resource.
  for (uint32_t i = 1; i < Calls.size(); ++i) {
    const uint32_t OrderA = getOrderId(Calls[i - 1]);
    const uint32_t OrderB = getOrderId(Calls[i]);
    if (OrderA == OrderB) {
      const uint32_t DescSetA = getDescSet(Calls[i - 1]);
      const uint32_t DescSetB = getDescSet(Calls[i]);
      if (DescSetA != DescSetB) {
        report_fatal_error("Implicit binding calls with the same order ID must "
                           "have the same descriptor set");
      }
    }
  }
}

uint32_t SPIRVLegalizeImplicitBinding::getAndReserveFirstUnusedBinding(
    uint32_t DescSet) {
  if (UsedBindings.size() <= DescSet) {
    UsedBindings.resize(DescSet + 1);
    UsedBindings[DescSet].resize(64);
  }

  int NewBinding = UsedBindings[DescSet].find_first_unset();
  if (NewBinding == -1) {
    NewBinding = UsedBindings[DescSet].size();
    UsedBindings[DescSet].resize(2 * NewBinding + 1);
  }

  UsedBindings[DescSet].set(NewBinding);
  return NewBinding;
}

void SPIRVLegalizeImplicitBinding::replaceImplicitBindingCalls(Module &M) {
  uint32_t lastOrderId = -1;
  uint32_t lastBindingNumber = -1;

  for (CallInst *OldCI : ImplicitBindingCalls) {
    const uint32_t OrderId = getOrderId(OldCI);
    uint32_t BindingNumber;
    if (OrderId == lastOrderId) {
      BindingNumber = lastBindingNumber;
    } else {
      const uint32_t DescSet = getDescSet(OldCI);
      BindingNumber = getAndReserveFirstUnusedBinding(DescSet);
    }

    if (OldCI->getIntrinsicID() ==
        Intrinsic::spv_resource_handlefromimplicitbinding) {
      replaceResourceHandleCall(M, OldCI, BindingNumber);
    } else {
      assert(OldCI->getIntrinsicID() ==
                 Intrinsic::spv_resource_counterhandlefromimplicitbinding &&
             "Unexpected implicit binding intrinsic");
      replaceCounterHandleCall(M, OldCI, BindingNumber);
    }
    lastOrderId = OrderId;
    lastBindingNumber = BindingNumber;
  }
}

bool SPIRVLegalizeImplicitBinding::runOnModule(Module &M) {
  collectBindingInfo(M);
  if (ImplicitBindingCalls.empty()) {
    return false;
  }
  verifyUniqueOrderIdPerResource(ImplicitBindingCalls);

  replaceImplicitBindingCalls(M);
  return true;
}
} // namespace

char SPIRVLegalizeImplicitBinding::ID = 0;

INITIALIZE_PASS(SPIRVLegalizeImplicitBinding, "legalize-spirv-implicit-binding",
                "Legalize SPIR-V implicit bindings", false, false)

ModulePass *toolchain::createSPIRVLegalizeImplicitBindingPass() {
  return new SPIRVLegalizeImplicitBinding();
}

void SPIRVLegalizeImplicitBinding::replaceResourceHandleCall(
    Module &M, CallInst *OldCI, uint32_t NewBinding) {
  IRBuilder<> Builder(OldCI);
  const uint32_t DescSet =
      cast<ConstantInt>(OldCI->getArgOperand(1))->getZExtValue();

  SmallVector<Value *, 8> Args;
  Args.push_back(Builder.getInt32(DescSet));
  Args.push_back(Builder.getInt32(NewBinding));

  // Copy the remaining arguments from the old call.
  for (uint32_t i = 2; i < OldCI->arg_size(); ++i) {
    Args.push_back(OldCI->getArgOperand(i));
  }

  Function *NewFunc = Intrinsic::getOrInsertDeclaration(
      &M, Intrinsic::spv_resource_handlefrombinding, OldCI->getType());
  CallInst *NewCI = Builder.CreateCall(NewFunc, Args);
  NewCI->setCallingConv(OldCI->getCallingConv());

  OldCI->replaceAllUsesWith(NewCI);
  OldCI->eraseFromParent();
}

void SPIRVLegalizeImplicitBinding::replaceCounterHandleCall(
    Module &M, CallInst *OldCI, uint32_t NewBinding) {
  IRBuilder<> Builder(OldCI);
  const uint32_t DescSet =
      cast<ConstantInt>(OldCI->getArgOperand(2))->getZExtValue();

  SmallVector<Value *, 8> Args;
  Args.push_back(OldCI->getArgOperand(0));
  Args.push_back(Builder.getInt32(NewBinding));
  Args.push_back(Builder.getInt32(DescSet));

  Type *Tys[] = {OldCI->getType(), OldCI->getArgOperand(0)->getType()};
  Function *NewFunc = Intrinsic::getOrInsertDeclaration(
      &M, Intrinsic::spv_resource_counterhandlefrombinding, Tys);
  CallInst *NewCI = Builder.CreateCall(NewFunc, Args);
  NewCI->setCallingConv(OldCI->getCallingConv());

  OldCI->replaceAllUsesWith(NewCI);
  OldCI->eraseFromParent();
}
