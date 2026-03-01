//===-- AVRAsmBackend.h - AVR Asm Backend  --------------------------------===//
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
// \file The AVR assembly backend implementation.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_AVR_ASM_BACKEND_H
#define LLVM_AVR_ASM_BACKEND_H

#include "MCTargetDesc/AVRFixupKinds.h"

#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/TargetParser/Triple.h"

namespace vm::core {

class MCAssembler;
class MCContext;
struct MCFixupKindInfo;

/// Utilities for manipulating generated AVR machine code.
class AVRAsmBackend : public MCAsmBackend {
public:
  AVRAsmBackend(Triple::OSType OSType)
      : MCAsmBackend(toolchain::endianness::little), OSType(OSType) {}

  void adjustFixupValue(const MCFixup &Fixup, const MCValue &Target,
                        uint64_t &Value, MCContext *Ctx = nullptr) const;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  bool forceRelocation(const MCFragment &F, const MCFixup &Fixup,
                       const MCValue &Target);

private:
  Triple::OSType OSType;
};

} // end namespace vm::core

#endif // LLVM_AVR_ASM_BACKEND_H
