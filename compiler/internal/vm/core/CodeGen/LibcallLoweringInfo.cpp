//===- LibcallLoweringInfo.cpp - Interface for runtime libcalls -----------===//
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

#include "vm/core/CodeGen/LibcallLoweringInfo.h"
#include "vm/core/Analysis/RuntimeLibcallInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

LibcallLoweringInfo::LibcallLoweringInfo(
    const RTLIB::RuntimeLibcallsInfo &RTLCI,
    const TargetSubtargetInfo &Subtarget)
    : RTLCI(RTLCI) {
  // TODO: This should be generated with lowering predicates, and assert the
  // call is available.
  for (RTLIB::LibcallImpl Impl : RTLIB::libcall_impls()) {
    if (RTLCI.isAvailable(Impl)) {
      RTLIB::Libcall LC = RTLIB::RuntimeLibcallsInfo::getLibcallFromImpl(Impl);
      // FIXME: Hack, assume the first available libcall wins.
      if (LibcallImpls[LC] == RTLIB::Unsupported)
        LibcallImpls[LC] = Impl;
    }
  }

  Subtarget.initLibcallLoweringInfo(*this);
}

AnalysisKey LibcallLoweringModuleAnalysis::Key;

bool LibcallLoweringModuleAnalysisResult::invalidate(
    Module &, const PreservedAnalyses &PA,
    ModuleAnalysisManager::Invalidator &) {
  // Passes that change the runtime libcall set must explicitly invalidate this
  // pass.
  auto PAC = PA.getChecker<LibcallLoweringModuleAnalysis>();
  return !PAC.preservedWhenStateless();
}

LibcallLoweringModuleAnalysisResult
LibcallLoweringModuleAnalysis::run(Module &M, ModuleAnalysisManager &MAM) {
  LibcallLoweringMap.init(&MAM.getResult<RuntimeLibraryAnalysis>(M));
  return LibcallLoweringMap;
}

INITIALIZE_PASS_BEGIN(LibcallLoweringInfoWrapper, "libcall-lowering-info",
                      "Library Function Lowering Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(RuntimeLibraryInfoWrapper)
INITIALIZE_PASS_END(LibcallLoweringInfoWrapper, "libcall-lowering-info",
                    "Library Function Lowering Analysis", false, true)

char LibcallLoweringInfoWrapper::ID = 0;

LibcallLoweringInfoWrapper::LibcallLoweringInfoWrapper() : ImmutablePass(ID) {}

bool LibcallLoweringInfoWrapper::doInitialization(Module &M) {
  Result.init(&getAnalysis<RuntimeLibraryInfoWrapper>().getRTLCI(M));
  return false;
}

void LibcallLoweringInfoWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<RuntimeLibraryInfoWrapper>();
  AU.setPreservesAll();
}

void LibcallLoweringInfoWrapper::releaseMemory() { Result.clear(); }
