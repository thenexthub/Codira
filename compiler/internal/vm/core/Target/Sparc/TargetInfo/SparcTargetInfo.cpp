//===-- SparcTargetInfo.cpp - Sparc Target Implementation -----------------===//
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

#include "TargetInfo/SparcTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getTheSparcTarget() {
  static Target TheSparcTarget;
  return TheSparcTarget;
}
Target &toolchain::getTheSparcV9Target() {
  static Target TheSparcV9Target;
  return TheSparcV9Target;
}
Target &toolchain::getTheSparcelTarget() {
  static Target TheSparcelTarget;
  return TheSparcelTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSparcTargetInfo() {
  RegisterTarget<Triple::sparc, /*HasJIT=*/false> X(getTheSparcTarget(),
                                                    "sparc", "Sparc", "Sparc");
  RegisterTarget<Triple::sparcv9, /*HasJIT=*/false> Y(
      getTheSparcV9Target(), "sparcv9", "Sparc V9", "Sparc");
  RegisterTarget<Triple::sparcel, /*HasJIT=*/false> Z(
      getTheSparcelTarget(), "sparcel", "Sparc LE", "Sparc");
}
