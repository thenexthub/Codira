//===- SPIRVPushConstantAccess.cpp - Translate CBuffer Loads ----*- C++ -*-===//
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
// This pass changes the types of all the globals in the PushConstant
// address space into a target extension type, and makes all references
// to this global go though a custom SPIR-V intrinsic.
//
// This allows the backend to properly lower the push constant struct type
// to a fully laid out type, and generate the proper OpAccessChain.
//
//===----------------------------------------------------------------------===//

#include "SPIRVPushConstantAccess.h"
#include "SPIRV.h"
#include "SPIRVSubtarget.h"
#include "SPIRVTargetMachine.h"
#include "SPIRVUtils.h"
#include "vm/core/Frontend/HLSL/CBuffer.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsSPIRV.h"
#include "vm/core/IR/Module.h"

#define DEBUG_TYPE "spirv-pushconstant-access"
using namespace vm::core;

static bool replacePushConstantAccesses(Module &M, SPIRVGlobalRegistry *GR) {
  bool Changed = false;
  for (GlobalVariable &GV : make_early_inc_range(M.globals())) {
    if (GV.getAddressSpace() !=
        storageClassToAddressSpace(SPIRV::StorageClass::PushConstant))
      continue;

    GV.removeDeadConstantUsers();

    Type *PCType = toolchain::TargetExtType::get(
        M.getContext(), "spirv.PushConstant", {GV.getValueType()});
    GlobalVariable *NewGV =
        new GlobalVariable(M, PCType, GV.isConstant(), GV.getLinkage(),
                           /* initializer= */ nullptr, GV.getName(),
                           /* InsertBefore= */ &GV, GV.getThreadLocalMode(),
                           GV.getAddressSpace(), GV.isExternallyInitialized());
    NewGV->setVisibility(GV.getVisibility());

    for (User *U : make_early_inc_range(GV.users())) {
      Instruction *I = cast<Instruction>(U);
      IRBuilder<> Builder(I);
      Value *GetPointerCall = Builder.CreateIntrinsic(
          NewGV->getType(), Intrinsic::spv_pushconstant_getpointer, {NewGV});
      GR->buildAssignPtr(Builder, GV.getValueType(), GetPointerCall);

      I->replaceUsesOfWith(&GV, GetPointerCall);
    }

    GV.eraseFromParent();
    Changed = true;
  }

  return Changed;
}

PreservedAnalyses SPIRVPushConstantAccess::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  const SPIRVSubtarget *ST = TM.getSubtargetImpl();
  SPIRVGlobalRegistry *GR = ST->getSPIRVGlobalRegistry();
  return replacePushConstantAccesses(M, GR) ? PreservedAnalyses::none()
                                            : PreservedAnalyses::all();
}

namespace {
class SPIRVPushConstantAccessLegacy : public ModulePass {
  SPIRVTargetMachine *TM = nullptr;

public:
  bool runOnModule(Module &M) override {
    const SPIRVSubtarget *ST = TM->getSubtargetImpl();
    SPIRVGlobalRegistry *GR = ST->getSPIRVGlobalRegistry();
    return replacePushConstantAccesses(M, GR);
  }
  StringRef getPassName() const override {
    return "SPIRV push constant Access";
  }
  SPIRVPushConstantAccessLegacy(SPIRVTargetMachine *TM)
      : ModulePass(ID), TM(TM) {}

  static char ID; // Pass identification.
};
char SPIRVPushConstantAccessLegacy::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(SPIRVPushConstantAccessLegacy, DEBUG_TYPE,
                "SPIRV push constant Access", false, false)

ModulePass *
toolchain::createSPIRVPushConstantAccessLegacyPass(SPIRVTargetMachine *TM) {
  return new SPIRVPushConstantAccessLegacy(TM);
}
