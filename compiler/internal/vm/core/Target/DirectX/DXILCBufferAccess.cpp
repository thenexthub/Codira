//===- DXILCBufferAccess.cpp - Translate CBuffer Loads --------------------===//
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

#include "DXILCBufferAccess.h"
#include "DirectX.h"
#include "vm/core/Analysis/DXILResource.h"
#include "vm/core/Frontend/HLSL/CBuffer.h"
#include "vm/core/Frontend/HLSL/HLSLResource.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/IntrinsicsDirectX.h"
#include "vm/core/IR/ReplaceConstant.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Transforms/Utils/Local.h"

#define DEBUG_TYPE "dxil-cbuffer-access"
using namespace vm::core;

static void replaceUsersOfGlobal(GlobalVariable *Global,
                                 GlobalVariable *HandleGV, size_t Offset) {
  for (Use &U : make_early_inc_range(Global->uses())) {
    auto UseInst = dyn_cast<Instruction>(U.getUser());
    // TODO: Constants? Metadata?
    assert(UseInst && "Non-instruction use of cbuffer");

    IRBuilder<> Builder(UseInst);
    LoadInst *Handle = Builder.CreateLoad(HandleGV->getValueType(), HandleGV,
                                          HandleGV->getName());
    Value *Ptr = Builder.CreateIntrinsic(
        Global->getType(), Intrinsic::dx_resource_getpointer,
        ArrayRef<Value *>{Handle,
                          ConstantInt::get(Builder.getInt32Ty(), Offset)});
    U.set(Ptr);
  }

  Global->removeFromParent();
}

static bool replaceCBufferAccesses(Module &M) {
  std::optional<hlsl::CBufferMetadata> CBufMD = hlsl::CBufferMetadata::get(
      M, [](Type *Ty) { return isa<toolchain::dxil::PaddingExtType>(Ty); });
  if (!CBufMD)
    return false;

  SmallVector<Constant *> CBufferGlobals;
  for (const hlsl::CBufferMapping &Mapping : *CBufMD)
    for (const hlsl::CBufferMember &Member : Mapping.Members)
      CBufferGlobals.push_back(Member.GV);
  convertUsersOfConstantsToInstructions(CBufferGlobals);

  for (const hlsl::CBufferMapping &Mapping : *CBufMD)
    for (const hlsl::CBufferMember &Member : Mapping.Members)
      replaceUsersOfGlobal(Member.GV, Mapping.Handle, Member.Offset);

  CBufMD->eraseFromModule();
  return true;
}

PreservedAnalyses DXILCBufferAccess::run(Module &M, ModuleAnalysisManager &AM) {
  PreservedAnalyses PA;
  bool Changed = replaceCBufferAccesses(M);

  if (!Changed)
    return PreservedAnalyses::all();
  return PA;
}

namespace {
class DXILCBufferAccessLegacy : public ModulePass {
public:
  bool runOnModule(Module &M) override { return replaceCBufferAccesses(M); }
  StringRef getPassName() const override { return "DXIL CBuffer Access"; }
  DXILCBufferAccessLegacy() : ModulePass(ID) {}

  static char ID; // Pass identification.
};
char DXILCBufferAccessLegacy::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS(DXILCBufferAccessLegacy, DEBUG_TYPE, "DXIL CBuffer Access",
                false, false)

ModulePass *toolchain::createDXILCBufferAccessLegacyPass() {
  return new DXILCBufferAccessLegacy();
}
