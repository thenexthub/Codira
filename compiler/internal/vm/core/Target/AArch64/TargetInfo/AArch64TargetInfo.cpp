//===-- AArch64TargetInfo.cpp - AArch64 Target Implementation -----------------===//
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

#include "TargetInfo/AArch64TargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;
Target &toolchain::getTheAArch64leTarget() {
  static Target TheAArch64leTarget;
  return TheAArch64leTarget;
}
Target &toolchain::getTheAArch64beTarget() {
  static Target TheAArch64beTarget;
  return TheAArch64beTarget;
}
Target &toolchain::getTheAArch64_32Target() {
  static Target TheAArch64leTarget;
  return TheAArch64leTarget;
}
Target &toolchain::getTheARM64Target() {
  static Target TheARM64Target;
  return TheARM64Target;
}
Target &toolchain::getTheARM64_32Target() {
  static Target TheARM64_32Target;
  return TheARM64_32Target;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeAArch64TargetInfo() {
  // Now register the "arm64" name for use with "-march". We don't want it to
  // take possession of the Triple::aarch64 tags though.
  TargetRegistry::RegisterTarget(getTheARM64Target(), "arm64",
                                 "ARM64 (little endian)", "AArch64",
                                 [](Triple::ArchType) { return false; }, true);
  TargetRegistry::RegisterTarget(getTheARM64_32Target(), "arm64_32",
                                 "ARM64 (little endian ILP32)", "AArch64",
                                 [](Triple::ArchType) { return false; }, true);

  RegisterTarget<Triple::aarch64, /*HasJIT=*/true> Z(
      getTheAArch64leTarget(), "aarch64", "AArch64 (little endian)", "AArch64");
  RegisterTarget<Triple::aarch64_be, /*HasJIT=*/true> W(
      getTheAArch64beTarget(), "aarch64_be", "AArch64 (big endian)", "AArch64");
  RegisterTarget<Triple::aarch64_32, /*HasJIT=*/true> X(
      getTheAArch64_32Target(), "aarch64_32", "AArch64 (little endian ILP32)", "AArch64");
}
