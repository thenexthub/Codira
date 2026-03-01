//===- AMDGPUExportKernelRuntimeHandles.cpp - Lower enqueued block --------===//
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
// \file
//
// Give any globals used for OpenCL block enqueue runtime handles external
// linkage so the runtime may access them. These should behave like internal
// functions for purposes of linking, but need to have an external symbol in the
// final object for the runtime to access them.
//
// TODO: This could be replaced with a new linkage type or global object
// metadata that produces an external symbol in the final object, but allows
// rename on IR linking. Alternatively if we can rely on
// GlobalValue::getGlobalIdentifier we can just make these external symbols to
// begin with.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUExportKernelRuntimeHandles.h"
#include "AMDGPU.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Pass.h"

#define DEBUG_TYPE "amdgpu-export-kernel-runtime-handles"

using namespace vm::core;

namespace {

/// Lower enqueued blocks.
class AMDGPUExportKernelRuntimeHandlesLegacy : public ModulePass {
public:
  static char ID;

  explicit AMDGPUExportKernelRuntimeHandlesLegacy() : ModulePass(ID) {}

private:
  bool runOnModule(Module &M) override;
};

} // end anonymous namespace

char AMDGPUExportKernelRuntimeHandlesLegacy::ID = 0;

char &toolchain::AMDGPUExportKernelRuntimeHandlesLegacyID =
    AMDGPUExportKernelRuntimeHandlesLegacy::ID;

INITIALIZE_PASS(AMDGPUExportKernelRuntimeHandlesLegacy, DEBUG_TYPE,
                "Externalize enqueued block runtime handles", false, false)

ModulePass *toolchain::createAMDGPUExportKernelRuntimeHandlesLegacyPass() {
  return new AMDGPUExportKernelRuntimeHandlesLegacy();
}

static bool exportKernelRuntimeHandles(Module &M) {
  bool Changed = false;

  const StringLiteral HandleSectionName(".amdgpu.kernel.runtime.handle");

  for (GlobalVariable &GV : M.globals()) {
    if (GV.getSection() == HandleSectionName) {
      GV.setLinkage(GlobalValue::ExternalLinkage);
      GV.setDSOLocal(false);
      Changed = true;
    }
  }

  if (!Changed)
    return false;

  // FIXME: We shouldn't really need to export the kernel address. We can
  // initialize the runtime handle with the kernel descriptor.
  for (Function &F : M) {
    if (F.getCallingConv() != CallingConv::AMDGPU_KERNEL)
      continue;

    const MDNode *Associated = F.getMetadata(LLVMContext::MD_associated);
    if (!Associated)
      continue;

    auto *VM = cast<ValueAsMetadata>(Associated->getOperand(0));
    auto *Handle = dyn_cast<GlobalObject>(VM->getValue());
    if (Handle && Handle->getSection() == HandleSectionName) {
      F.setLinkage(GlobalValue::ExternalLinkage);
      F.setVisibility(GlobalValue::ProtectedVisibility);
    }
  }

  return Changed;
}

bool AMDGPUExportKernelRuntimeHandlesLegacy::runOnModule(Module &M) {
  return exportKernelRuntimeHandles(M);
}

PreservedAnalyses
AMDGPUExportKernelRuntimeHandlesPass::run(Module &M,
                                          ModuleAnalysisManager &MAM) {
  if (!exportKernelRuntimeHandles(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<AllAnalysesOn<Function>>();
  return PA;
}
