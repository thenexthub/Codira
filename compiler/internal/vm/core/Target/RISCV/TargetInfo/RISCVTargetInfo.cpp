//===-- RISCVTargetInfo.cpp - RISC-V Target Implementation ----------------===//
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

#include "TargetInfo/RISCVTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getTheRISCV32Target() {
  static Target TheRISCV32Target;
  return TheRISCV32Target;
}

Target &toolchain::getTheRISCV64Target() {
  static Target TheRISCV64Target;
  return TheRISCV64Target;
}

Target &toolchain::getTheRISCV32beTarget() {
  static Target TheRISCV32beTarget;
  return TheRISCV32beTarget;
}

Target &toolchain::getTheRISCV64beTarget() {
  static Target TheRISCV64beTarget;
  return TheRISCV64beTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeRISCVTargetInfo() {
  RegisterTarget<Triple::riscv32, /*HasJIT=*/true> X(
      getTheRISCV32Target(), "riscv32", "32-bit RISC-V", "RISCV");
  RegisterTarget<Triple::riscv64, /*HasJIT=*/true> Y(
      getTheRISCV64Target(), "riscv64", "64-bit RISC-V", "RISCV");
  RegisterTarget<Triple::riscv32be> A(getTheRISCV32beTarget(), "riscv32be",
                                      "32-bit big endian RISC-V", "RISCV");
  RegisterTarget<Triple::riscv64be> B(getTheRISCV64beTarget(), "riscv64be",
                                      "64-bit big endian RISC-V", "RISCV");
}
