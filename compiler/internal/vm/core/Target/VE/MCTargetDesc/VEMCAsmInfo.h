//===- VEMCAsmInfo.h - VE asm properties -----------------------*- C++ -*--===//
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
// This file contains the declaration of the VEMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_VE_MCTARGETDESC_VEMCASMINFO_H
#define LLVM_LIB_TARGET_VE_MCTARGETDESC_VEMCASMINFO_H

#include "VEFixupKinds.h"
#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"

namespace vm::core {

class Triple;

class VEELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit VEELFMCAsmInfo(const Triple &TheTriple);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

namespace VE {
enum Specifier {
  S_None,

  S_REFLONG = MCSymbolRefExpr::FirstTargetSpecifier,
  S_HI32,        // @hi
  S_LO32,        // @lo
  S_PC_HI32,     // @pc_hi
  S_PC_LO32,     // @pc_lo
  S_GOT_HI32,    // @got_hi
  S_GOT_LO32,    // @got_lo
  S_GOTOFF_HI32, // @gotoff_hi
  S_GOTOFF_LO32, // @gotoff_lo
  S_PLT_HI32,    // @plt_hi
  S_PLT_LO32,    // @plt_lo
  S_TLS_GD_HI32, // @tls_gd_hi
  S_TLS_GD_LO32, // @tls_gd_lo
  S_TPOFF_HI32,  // @tpoff_hi
  S_TPOFF_LO32,  // @tpoff_lo
};

VE::Fixups getFixupKind(uint8_t S);
} // namespace VE
} // namespace vm::core

#endif // LLVM_LIB_TARGET_VE_MCTARGETDESC_VEMCASMINFO_H
