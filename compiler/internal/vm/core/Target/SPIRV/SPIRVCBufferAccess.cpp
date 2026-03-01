//===- SPIRVCBufferAccess.cpp - Translate CBuffer Loads ---------*- C++ -*-===//
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
// This pass replaces all accesses to constant buffer global variables with
// accesses to the proper SPIR-V resource.
//
// The pass operates as follows:
// 1. It finds all constant buffers by looking for the `!hlsl.cbs` metadata.
// 2. For each cbuffer, it finds the global variable holding the resource handle
//    and the global variables for each of the cbuffer's members.
// 3. For each member variable, it creates a call to the
//    `toolchain.spv.resource.getpointer` intrinsic. This intrinsic takes the
//    resource handle and the member's index within the cbuffer as arguments.
//    The result is a pointer to that member within the SPIR-V resource.
// 4. It then replaces all uses of the original member global variable with the
//    pointer returned by the `getpointer` intrinsic. This effectively retargets
//    all loads and GEPs to the new resource pointer.
// 5. Finally, it cleans up by deleting the original global variables and the
//    `!hlsl.cbs` metadata.
//
// This approach allows subsequent passes, like SPIRVEmitIntrinsics, to
// correctly handle GEPs that operate on the result of the `getpointer` call,
// folding them into a single OpAccessChain instruction.
//
//===----------------------------------------------------------------------===//

#include "SPIRVCBufferAccess.h"
#include "SPIRV.h"
#include "vm/core/Frontend/HLSL/CBuffer.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsSPIRV.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/ReplaceConstant.h"

#define DEBUG_TYPE "spirv-cbuffer-access"
using namespace vm::core;

// Finds the single instruction that defines the resource handle. This is
// typically a call to `toolchain.spv.resource.handlefrombinding`.
static Instruction *findHandleDef(GlobalVariable *HandleVar) {
  for (User *U : HandleVar->users()) {
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      if (auto *I = dyn_cast<Instruction>(SI->getValueOperand())) {
        return I;
      }
    }
  }
  return nullptr;
}

static bool replaceCBufferAccesses(Module &M) {
  std::optional<hlsl::CBufferMetadata> CBufMD =
      hlsl::CBufferMetadata::get(M, [](Type *Ty) {
        if (auto *TET = dyn_cast<TargetExtType>(Ty))
          return TET->getName() == "spirv.Padding";
        return false;
      });
  if (!CBufMD)
    return false;

  SmallVector<Constant *> CBufferGlobals;
  for (const hlsl::CBufferMapping &Mapping : *CBufMD)
    for (const hlsl::CBufferMember &Member : Mapping.Members)
      CBufferGlobals.push_back(Member.GV);
  convertUsersOfConstantsToInstructions(CBufferGlobals);

  for (const hlsl::CBufferMapping &Mapping : *CBufMD) {
    Instruction *HandleDef = findHandleDef(Mapping.Handle);
    if (!HandleDef) {
      report_fatal_error("Could not find handle definition for cbuffer: " +
                         Mapping.Handle->getName());
    }

    // The handle definition should dominate all uses of the cbuffer members.
    // We'll insert our getpointer calls right after it.
    IRBuilder<> Builder(HandleDef->getNextNode());
    auto *HandleTy = cast<TargetExtType>(Mapping.Handle->getValueType());
    auto *LayoutTy = cast<StructType>(HandleTy->getTypeParameter(0));
    const StructLayout *SL = M.getDataLayout().getStructLayout(LayoutTy);

    for (const hlsl::CBufferMember &Member : Mapping.Members) {
      GlobalVariable *MemberGV = Member.GV;
      if (MemberGV->use_empty()) {
        continue;
      }

      uint32_t IndexInStruct = SL->getElementContainingOffset(Member.Offset);

      // Create the getpointer intrinsic call.
      Value *IndexVal = Builder.getInt32(IndexInStruct);
      Type *PtrType = MemberGV->getType();
      Value *GetPointerCall = Builder.CreateIntrinsic(
          PtrType, Intrinsic::spv_resource_getpointer, {HandleDef, IndexVal});

      MemberGV->replaceAllUsesWith(GetPointerCall);
    }
  }

  // Now that all uses are replaced, clean up the globals and metadata.
  for (const hlsl::CBufferMapping &Mapping : *CBufMD) {
    for (const auto &Member : Mapping.Members) {
      Member.GV->eraseFromParent();
    }
    // Erase the stores to the handle variable before erasing the handle itself.
    SmallVector<Instruction *, 4> HandleStores;
    for (User *U : Mapping.Handle->users()) {
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        HandleStores.push_back(SI);
      }
    }
    for (Instruction *I : HandleStores) {
      I->eraseFromParent();
    }
    Mapping.Handle->eraseFromParent();
  }

  CBufMD->eraseFromModule();
  return true;
}

PreservedAnalyses SPIRVCBufferAccess::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  if (replaceCBufferAccesses(M)) {
    return PreservedAnalyses::none();
  }
  return PreservedAnalyses::all();
}

namespace {
class SPIRVCBufferAccessLegacy : public ModulePass {
public:
  bool runOnModule(Module &M) override { return replaceCBufferAccesses(M); }
  StringRef getPassName() const override { return "SPIRV CBuffer Access"; }
  SPIRVCBufferAccessLegacy() : ModulePass(ID) {}

  static char ID; // Pass identification.
};
char SPIRVCBufferAccessLegacy::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(SPIRVCBufferAccessLegacy, DEBUG_TYPE, "SPIRV CBuffer Access",
                false, false)

ModulePass *toolchain::createSPIRVCBufferAccessLegacyPass() {
  return new SPIRVCBufferAccessLegacy();
}
