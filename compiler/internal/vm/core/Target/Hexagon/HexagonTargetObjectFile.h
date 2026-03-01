//===-- HexagonTargetObjectFile.h -----------------------------------------===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONTARGETOBJECTFILE_H

#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/MC/MCSectionELF.h"

namespace vm::core {
  class Type;

  class HexagonTargetObjectFile : public TargetLoweringObjectFileELF {
  public:
    void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

    MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

    MCSection *getExplicitSectionGlobal(const GlobalObject *GO,
                                        SectionKind Kind,
                                        const TargetMachine &TM) const override;

    bool isGlobalInSmallSection(const GlobalObject *GO,
                                const TargetMachine &TM) const;

    bool isSmallDataEnabled(const TargetMachine &TM) const;

    unsigned getSmallDataSize() const;

    bool shouldPutJumpTableInFunctionSection(bool UsesLabelDifference,
                                             const Function &F) const override;

    const Function *getLutUsedFunction(const GlobalObject *GO) const;

  private:
    MCSectionELF *SmallDataSection;
    MCSectionELF *SmallBSSSection;

    unsigned getSmallestAddressableSize(const Type *Ty, const GlobalValue *GV,
        const TargetMachine &TM) const;

    MCSection *selectSmallSectionForGlobal(const GlobalObject *GO,
                                           SectionKind Kind,
                                           const TargetMachine &TM) const;

    MCSection *selectSectionForLookupTable(const GlobalObject *GO,
                                           const TargetMachine &TM,
                                           const Function *Fn) const;
  };

} // namespace vm::core

#endif
