//===- DirectXTargetInfo.cpp - DirectX Target Implementation ----*- C++ -*-===//
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
/// This file contains DirectX target initializer.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {
Target &getTheDirectXTarget() {
  static Target TheDirectXTarget;
  return TheDirectXTarget;
}
} // namespace vm::core

using namespace vm::core;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDirectXTargetInfo() {
  RegisterTarget<Triple::dxil, /*HasJIT=*/false> X(
      getTheDirectXTarget(), "dxil", "DirectX Intermediate Language", "DXIL");
}
