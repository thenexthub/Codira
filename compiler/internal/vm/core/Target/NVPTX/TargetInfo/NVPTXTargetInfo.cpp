//===-- NVPTXTargetInfo.cpp - NVPTX Target Implementation -----------------===//
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

#include "TargetInfo/NVPTXTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

Target &toolchain::getTheNVPTXTarget32() {
  static Target TheNVPTXTarget32;
  return TheNVPTXTarget32;
}
Target &toolchain::getTheNVPTXTarget64() {
  static Target TheNVPTXTarget64;
  return TheNVPTXTarget64;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeNVPTXTargetInfo() {
  RegisterTarget<Triple::nvptx> X(getTheNVPTXTarget32(), "nvptx",
                                  "NVIDIA PTX 32-bit", "NVPTX");
  RegisterTarget<Triple::nvptx64> Y(getTheNVPTXTarget64(), "nvptx64",
                                    "NVIDIA PTX 64-bit", "NVPTX");
}
