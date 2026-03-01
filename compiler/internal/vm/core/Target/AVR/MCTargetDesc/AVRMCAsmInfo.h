//===-- AVRMCAsmInfo.h - AVR asm properties ---------------------*- C++ -*-===//
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
// This file contains the declaration of the AVRMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_ASM_INFO_H
#define LLVM_AVR_ASM_INFO_H

#include "MCTargetDesc/AVRMCExpr.h"
#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"

namespace vm::core {

class Triple;

/// Specifies the format of AVR assembly files.
class AVRMCAsmInfo : public MCAsmInfoELF {
public:
  explicit AVRMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

namespace AVR {
using Specifier = uint16_t;
enum {
  S_None,

  S_AVR_NONE = MCSymbolRefExpr::FirstTargetSpecifier,

  S_HI8,  ///< Corresponds to `hi8()`.
  S_LO8,  ///< Corresponds to `lo8()`.
  S_HH8,  ///< Corresponds to `hlo8() and hh8()`.
  S_HHI8, ///< Corresponds to `hhi8()`.

  S_PM,     ///< Corresponds to `pm()`, reference to program memory.
  S_PM_LO8, ///< Corresponds to `pm_lo8()`.
  S_PM_HI8, ///< Corresponds to `pm_hi8()`.
  S_PM_HH8, ///< Corresponds to `pm_hh8()`.

  S_LO8_GS, ///< Corresponds to `lo8(gs())`.
  S_HI8_GS, ///< Corresponds to `hi8(gs())`.
  S_GS,     ///< Corresponds to `gs()`.

  S_DIFF8,
  S_DIFF16,
  S_DIFF32,
};
} // namespace AVR

} // end namespace vm::core

#endif // LLVM_AVR_ASM_INFO_H
