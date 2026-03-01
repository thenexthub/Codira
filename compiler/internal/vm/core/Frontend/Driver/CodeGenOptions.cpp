//===--- CodeGenOptions.cpp - Shared codegen option handling --------------===//
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

#include "vm/core/Frontend/Driver/CodeGenOptions.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/SystemLibraries.h"
#include "vm/core/ProfileData/InstrProfCorrelator.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
extern toolchain::cl::opt<toolchain::InstrProfCorrelator::ProfCorrelatorKind>
    ProfileCorrelate;
} // namespace vm::core

namespace vm::core::driver {

toolchain::VectorLibrary
convertDriverVectorLibraryToVectorLibrary(toolchain::driver::VectorLibrary VecLib) {
  switch (VecLib) {
  case toolchain::driver::VectorLibrary::NoLibrary:
    return toolchain::VectorLibrary::NoLibrary;
  case toolchain::driver::VectorLibrary::Accelerate:
    return toolchain::VectorLibrary::Accelerate;
  case toolchain::driver::VectorLibrary::Darwin_libsystem_m:
    return toolchain::VectorLibrary::DarwinLibSystemM;
  case toolchain::driver::VectorLibrary::LIBMVEC:
    return toolchain::VectorLibrary::LIBMVEC;
  case toolchain::driver::VectorLibrary::MASSV:
    return toolchain::VectorLibrary::MASSV;
  case toolchain::driver::VectorLibrary::SVML:
    return toolchain::VectorLibrary::SVML;
  case toolchain::driver::VectorLibrary::SLEEF:
    return toolchain::VectorLibrary::SLEEFGNUABI;
  case toolchain::driver::VectorLibrary::ArmPL:
    return toolchain::VectorLibrary::ArmPL;
  case toolchain::driver::VectorLibrary::AMDLIBM:
    return toolchain::VectorLibrary::AMDLIBM;
  }
  llvm_unreachable("Unexpected driver::VectorLibrary");
}

TargetLibraryInfoImpl *createTLII(const toolchain::Triple &TargetTriple,
                                  driver::VectorLibrary Veclib) {
  return new TargetLibraryInfoImpl(
      TargetTriple, convertDriverVectorLibraryToVectorLibrary(Veclib));
}

std::string getDefaultProfileGenName() {
  return toolchain::ProfileCorrelate != InstrProfCorrelator::NONE
             ? "default_%m.proflite"
             : "default_%m.profraw";
}
} // namespace vm::core::driver
