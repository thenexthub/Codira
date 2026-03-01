//===-- MCTargetDesc/AMDGPUMCAsmInfo.h - AMDGPU MCAsm Interface -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCASMINFO_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
namespace vm::core {

class Triple;

// If you need to create another MCAsmInfo class, which inherits from MCAsmInfo,
// you will need to make sure your new class sets PrivateGlobalPrefix to
// a prefix that won't appear in a function name.  The default value
// for PrivateGlobalPrefix is 'L', so it will consider any function starting
// with 'L' as a local symbol.
class AMDGPUMCAsmInfo : public MCAsmInfoELF {
public:
  explicit AMDGPUMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  bool shouldOmitSectionDirective(StringRef SectionName) const override;
  unsigned getMaxInstLength(const MCSubtargetInfo *STI) const override;
};
} // namespace vm::core
#endif
