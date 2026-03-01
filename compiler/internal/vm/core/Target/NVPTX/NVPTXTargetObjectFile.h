//===-- NVPTXTargetObjectFile.h - NVPTX Object Info -------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXTARGETOBJECTFILE_H

#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/SectionKind.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"

namespace vm::core {

class NVPTXTargetObjectFile : public TargetLoweringObjectFile {
public:
  NVPTXTargetObjectFile() = default;

  ~NVPTXTargetObjectFile() override;

  MCSection *getSectionForConstant(const DataLayout &DL, SectionKind Kind,
                                   const Constant *C,
                                   Align &Alignment) const override {
    return ReadOnlySection;
  }

  MCSection *getExplicitSectionGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override {
    return DataSection;
  }

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_NVPTX_NVPTXTARGETOBJECTFILE_H
