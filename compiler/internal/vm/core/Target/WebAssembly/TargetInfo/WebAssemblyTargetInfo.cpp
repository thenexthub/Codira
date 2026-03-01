//===-- WebAssemblyTargetInfo.cpp - WebAssembly Target Implementation -----===//
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
///
/// \file
/// This file registers the WebAssembly target.
///
//===----------------------------------------------------------------------===//

#include "TargetInfo/WebAssemblyTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-target-info"

Target &toolchain::getTheWebAssemblyTarget32() {
  static Target TheWebAssemblyTarget32;
  return TheWebAssemblyTarget32;
}
Target &toolchain::getTheWebAssemblyTarget64() {
  static Target TheWebAssemblyTarget64;
  return TheWebAssemblyTarget64;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeWebAssemblyTargetInfo() {
  RegisterTarget<Triple::wasm32> X(getTheWebAssemblyTarget32(), "wasm32",
                                   "WebAssembly 32-bit", "WebAssembly");
  RegisterTarget<Triple::wasm64> Y(getTheWebAssemblyTarget64(), "wasm64",
                                   "WebAssembly 64-bit", "WebAssembly");
}

// Defines toolchain::WebAssembly::getWasm64Opcode toolchain::WebAssembly::getStackOpcode
// which have to be in a shared location between CodeGen and MC.
#define GET_INSTRMAP_INFO 1
#define GET_INSTRINFO_ENUM 1
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "WebAssemblyGenInstrInfo.inc"
