//===- RuntimeLibcallInfo.cpp ---------------------------------------------===//
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

#include "vm/core/Analysis/RuntimeLibcallInfo.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

AnalysisKey RuntimeLibraryAnalysis::Key;

RuntimeLibraryAnalysis::RuntimeLibraryAnalysis(const Triple &TT,
                                               ExceptionHandling ExceptionModel,
                                               FloatABI::ABIType FloatABI,
                                               EABI EABIVersion,
                                               StringRef ABIName,
                                               VectorLibrary VecLib)
    : LibcallsInfo(std::in_place, TT, ExceptionModel, FloatABI, EABIVersion,
                   ABIName, VecLib) {}

RTLIB::RuntimeLibcallsInfo
RuntimeLibraryAnalysis::run(const Module &M, ModuleAnalysisManager &) {
  if (!LibcallsInfo)
    LibcallsInfo = RTLIB::RuntimeLibcallsInfo(M);
  return *LibcallsInfo;
}

INITIALIZE_PASS(RuntimeLibraryInfoWrapper, "runtime-library-info",
                "Runtime Library Function Analysis", false, true)

RuntimeLibraryInfoWrapper::RuntimeLibraryInfoWrapper()
    : ImmutablePass(ID), RTLA(RTLIB::RuntimeLibcallsInfo(Triple())) {}

RuntimeLibraryInfoWrapper::RuntimeLibraryInfoWrapper(
    const Triple &TT, ExceptionHandling ExceptionModel,
    FloatABI::ABIType FloatABI, EABI EABIVersion, StringRef ABIName,
    VectorLibrary VecLib)
    : ImmutablePass(ID), RTLCI(std::in_place, TT, ExceptionModel, FloatABI,
                               EABIVersion, ABIName, VecLib) {}

char RuntimeLibraryInfoWrapper::ID = 0;

ModulePass *toolchain::createRuntimeLibraryInfoWrapperPass() {
  return new RuntimeLibraryInfoWrapper();
}

void RuntimeLibraryInfoWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

// Assume this is stable unless explicitly invalidated.
bool RTLIB::RuntimeLibcallsInfo::invalidate(
    Module &M, const PreservedAnalyses &PA,
    ModuleAnalysisManager::Invalidator &) {
  auto PAC = PA.getChecker<RuntimeLibraryAnalysis>();
  return !PAC.preservedWhenStateless();
}
