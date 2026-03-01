//===-- RISCVMCAsmInfo.h - RISC-V Asm Info ---------------------*- C++ -*--===//
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
// This file contains the declaration of the RISCVMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVMCASMINFO_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVMCASMINFO_H

#include "vm/core/MC/MCAsmInfoDarwin.h"
#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCFixup.h"

namespace vm::core {
class Triple;

class RISCVMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit RISCVMCAsmInfo(const Triple &TargetTriple);

  const MCExpr *getExprForFDESymbol(const MCSymbol *Sym, unsigned Encoding,
                                    MCStreamer &Streamer) const override;
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace RISCV {
using Specifier = uint16_t;
// Specifiers mapping to relocation types below FirstTargetFixupKind are
// encoded literally, with these exceptions:
enum {
  S_None,
  // Specifiers mapping to distinct relocation types.
  S_LO = FirstTargetFixupKind,
  S_PCREL_LO,
  S_TPREL_LO,
  // Vendor-specific relocation types might conflict across vendors.
  // Refer to them using Specifier constants.
  S_QC_ABS20,
};

Specifier parseSpecifierName(StringRef name);
StringRef getSpecifierName(Specifier Kind);
} // namespace RISCV

class RISCVMCAsmInfoDarwin : public MCAsmInfoDarwin {
public:
  explicit RISCVMCAsmInfoDarwin();
};

} // namespace vm::core

#endif
