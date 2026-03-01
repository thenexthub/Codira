//===-- PowerPCTargetInfo.cpp - PowerPC Target Implementation -------------===//
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

#include "TargetInfo/PowerPCTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getThePPC32Target() {
  static Target ThePPC32Target;
  return ThePPC32Target;
}
Target &toolchain::getThePPC32LETarget() {
  static Target ThePPC32LETarget;
  return ThePPC32LETarget;
}
Target &toolchain::getThePPC64Target() {
  static Target ThePPC64Target;
  return ThePPC64Target;
}
Target &toolchain::getThePPC64LETarget() {
  static Target ThePPC64LETarget;
  return ThePPC64LETarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializePowerPCTargetInfo() {
  RegisterTarget<Triple::ppc, /*HasJIT=*/true> W(getThePPC32Target(), "ppc32",
                                                 "PowerPC 32", "PPC");

  RegisterTarget<Triple::ppcle, /*HasJIT=*/true> X(
      getThePPC32LETarget(), "ppc32le", "PowerPC 32 LE", "PPC");

  RegisterTarget<Triple::ppc64, /*HasJIT=*/true> Y(getThePPC64Target(), "ppc64",
                                                   "PowerPC 64", "PPC");

  RegisterTarget<Triple::ppc64le, /*HasJIT=*/true> Z(
      getThePPC64LETarget(), "ppc64le", "PowerPC 64 LE", "PPC");
}
