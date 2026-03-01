//===-- MipsTargetInfo.cpp - Mips Target Implementation -------------------===//
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

#include "TargetInfo/MipsTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getTheMipsTarget() {
  static Target TheMipsTarget;
  return TheMipsTarget;
}
Target &toolchain::getTheMipselTarget() {
  static Target TheMipselTarget;
  return TheMipselTarget;
}
Target &toolchain::getTheMips64Target() {
  static Target TheMips64Target;
  return TheMips64Target;
}
Target &toolchain::getTheMips64elTarget() {
  static Target TheMips64elTarget;
  return TheMips64elTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeMipsTargetInfo() {
  RegisterTarget<Triple::mips,
                 /*HasJIT=*/true>
      X(getTheMipsTarget(), "mips", "MIPS (32-bit big endian)", "Mips");

  RegisterTarget<Triple::mipsel,
                 /*HasJIT=*/true>
      Y(getTheMipselTarget(), "mipsel", "MIPS (32-bit little endian)", "Mips");

  RegisterTarget<Triple::mips64,
                 /*HasJIT=*/true>
      A(getTheMips64Target(), "mips64", "MIPS (64-bit big endian)", "Mips");

  RegisterTarget<Triple::mips64el,
                 /*HasJIT=*/true>
      B(getTheMips64elTarget(), "mips64el", "MIPS (64-bit little endian)",
        "Mips");
}
