//===-- MipsMCAsmInfo.h - Mips Asm Info ------------------------*- C++ -*--===//
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
// This file contains the declaration of the MipsMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCASMINFO_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSMCASMINFO_H

#include "vm/core/MC/MCAsmInfoCOFF.h"
#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCFixup.h"

namespace vm::core {
class Triple;

class MipsELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit MipsELFMCAsmInfo(const Triple &TheTriple,
                            const MCTargetOptions &Options);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

class MipsCOFFMCAsmInfo : public MCAsmInfoGNUCOFF {
  void anchor() override;

public:
  explicit MipsCOFFMCAsmInfo();
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

namespace Mips {
using Specifier = uint16_t;
enum {
  S_None,
  S_CALL_HI16 = FirstTargetFixupKind,
  S_CALL_LO16,
  S_DTPREL,
  S_DTPREL_HI,
  S_DTPREL_LO,
  S_GOT,
  S_GOTTPREL,
  S_GOT_CALL,
  S_GOT_DISP,
  S_GOT_HI16,
  S_GOT_LO16,
  S_GOT_OFST,
  S_GOT_PAGE,
  S_GPREL,
  S_HI,
  S_HIGHER,
  S_HIGHEST,
  S_LO,
  S_NEG,
  S_PCREL_HI16,
  S_PCREL_LO16,
  S_TLSGD,
  S_TLSLDM,
  S_TPREL_HI,
  S_TPREL_LO,
  S_Special,
};

bool isGpOff(const MCSpecifierExpr &E);
const MCSpecifierExpr *createGpOff(const MCExpr *Expr, Specifier S,
                                   MCContext &Ctx);
}

} // namespace vm::core

#endif
