//===-- ARMTargetInfo.cpp - ARM Target Implementation ---------------------===//
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

#include "TargetInfo/ARMTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

Target &toolchain::getTheARMLETarget() {
  static Target TheARMLETarget;
  return TheARMLETarget;
}
Target &toolchain::getTheARMBETarget() {
  static Target TheARMBETarget;
  return TheARMBETarget;
}
Target &toolchain::getTheThumbLETarget() {
  static Target TheThumbLETarget;
  return TheThumbLETarget;
}
Target &toolchain::getTheThumbBETarget() {
  static Target TheThumbBETarget;
  return TheThumbBETarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeARMTargetInfo() {
  RegisterTarget<Triple::arm, /*HasJIT=*/true> X(getTheARMLETarget(), "arm",
                                                 "ARM", "ARM");
  RegisterTarget<Triple::armeb, /*HasJIT=*/true> Y(getTheARMBETarget(), "armeb",
                                                   "ARM (big endian)", "ARM");

  RegisterTarget<Triple::thumb, /*HasJIT=*/true> A(getTheThumbLETarget(),
                                                   "thumb", "Thumb", "ARM");
  RegisterTarget<Triple::thumbeb, /*HasJIT=*/true> B(
      getTheThumbBETarget(), "thumbeb", "Thumb (big endian)", "ARM");
}
