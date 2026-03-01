//===-- NVPTXMCAsmInfo.h - NVPTX asm properties ----------------*- C++ -*--===//
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
// This file contains the declaration of the NVPTXMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXMCASMINFO_H
#define LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXMCASMINFO_H

#include "vm/core/MC/MCAsmInfo.h"

namespace vm::core {
class Triple;

class NVPTXMCAsmInfo : public MCAsmInfo {
public:
  explicit NVPTXMCAsmInfo(const Triple &TheTriple,
                          const MCTargetOptions &Options);

  /// Return true if the .section directive should be omitted when
  /// emitting \p SectionName.  For example:
  ///
  /// shouldOmitSectionDirective(".text")
  ///
  /// returns false => .section .text,#alloc,#execinstr
  /// returns true  => .text
  bool shouldOmitSectionDirective(StringRef SectionName) const override {
    return true;
  }
};
} // namespace vm::core

#endif
