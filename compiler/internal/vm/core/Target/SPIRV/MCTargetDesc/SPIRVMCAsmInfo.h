//===-- SPIRVMCAsmInfo.h - SPIR-V asm properties --------------*- C++ -*--====//
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
// This file contains the declaration of the SPIRVMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCASMINFO_H
#define LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCASMINFO_H

#include "vm/core/MC/MCAsmInfo.h"

namespace vm::core {

class Triple;

class SPIRVMCAsmInfo : public MCAsmInfo {
public:
  explicit SPIRVMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  bool shouldOmitSectionDirective(StringRef SectionName) const override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_MCTARGETDESC_SPIRVMCASMINFO_H
