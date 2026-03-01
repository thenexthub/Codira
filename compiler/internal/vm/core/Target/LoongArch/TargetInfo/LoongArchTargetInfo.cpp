//===-- LoongArchTargetInfo.cpp - LoongArch Target Implementation ---------===//
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

#include "TargetInfo/LoongArchTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getTheLoongArch32Target() {
  static Target TheLoongArch32Target;
  return TheLoongArch32Target;
}

Target &toolchain::getTheLoongArch64Target() {
  static Target TheLoongArch64Target;
  return TheLoongArch64Target;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeLoongArchTargetInfo() {
  RegisterTarget<Triple::loongarch32, /*HasJIT=*/false> X(
      getTheLoongArch32Target(), "loongarch32", "32-bit LoongArch",
      "LoongArch");
  RegisterTarget<Triple::loongarch64, /*HasJIT=*/true> Y(
      getTheLoongArch64Target(), "loongarch64", "64-bit LoongArch",
      "LoongArch");
}
