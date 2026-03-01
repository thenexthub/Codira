//===-- TargetInfo/AMDGPUTargetInfo.cpp - TargetInfo for AMDGPU -----------===//
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
//
/// \file
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/AMDGPUTargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

/// The target for R600 GPUs.
Target &toolchain::getTheR600Target() {
  static Target TheAMDGPUTarget;
  return TheAMDGPUTarget;
}

/// The target for GCN GPUs.
Target &toolchain::getTheGCNTarget() {
  static Target TheGCNTarget;
  return TheGCNTarget;
}

/// Extern function to initialize the targets for the AMDGPU backend
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeAMDGPUTargetInfo() {
  RegisterTarget<Triple::r600, false> R600(getTheR600Target(), "r600",
                                           "AMD GPUs HD2XXX-HD6XXX", "AMDGPU");
  RegisterTarget<Triple::amdgcn, false> GCN(getTheGCNTarget(), "amdgcn",
                                            "AMD GCN GPUs", "AMDGPU");
}
