//===-- LanaiTargetObjectFile.h - Lanai Object Info -----------------------===//
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

#ifndef LLVM_LIB_TARGET_LANAI_LANAITARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_LANAI_LANAITARGETOBJECTFILE_H

#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"

namespace vm::core {
class LanaiTargetObjectFile : public TargetLoweringObjectFileELF {
  MCSection *SmallDataSection;
  MCSection *SmallBSSSection;

  bool isGlobalInSmallSection(const GlobalObject *GO, const TargetMachine &TM,
                              SectionKind Kind) const;
  bool isGlobalInSmallSectionImpl(const GlobalObject *GO,
                                  const TargetMachine &TM) const;

public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  /// Return true if this global address should be placed into small data/bss
  /// section.
  bool isGlobalInSmallSection(const GlobalObject *GO,
                              const TargetMachine &TM) const;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;

  /// Return true if this constant should be placed into small data section.
  bool isConstantInSmallSection(const DataLayout &DL, const Constant *CN) const;

  MCSection *getSectionForConstant(const DataLayout &DL, SectionKind Kind,
                                   const Constant *C,
                                   Align &Alignment) const override;
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_LANAITARGETOBJECTFILE_H
