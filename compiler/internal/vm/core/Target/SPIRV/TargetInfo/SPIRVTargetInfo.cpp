//===-- SPIRVTargetInfo.cpp - SPIR-V Target Implementation ----*- C++ -*---===//
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

#include "TargetInfo/SPIRVTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

Target &toolchain::getTheSPIRV32Target() {
  static Target TheSPIRV32Target;
  return TheSPIRV32Target;
}
Target &toolchain::getTheSPIRV64Target() {
  static Target TheSPIRV64Target;
  return TheSPIRV64Target;
}
Target &toolchain::getTheSPIRVLogicalTarget() {
  static Target TheSPIRVLogicalTarget;
  return TheSPIRVLogicalTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSPIRVTargetInfo() {
  RegisterTarget<Triple::spirv32> X(getTheSPIRV32Target(), "spirv32",
                                    "SPIR-V 32-bit", "SPIRV");
  RegisterTarget<Triple::spirv64> Y(getTheSPIRV64Target(), "spirv64",
                                    "SPIR-V 64-bit", "SPIRV");
  RegisterTarget<Triple::spirv> Z(getTheSPIRVLogicalTarget(), "spirv",
                                  "SPIR-V Logical", "SPIRV");
}
