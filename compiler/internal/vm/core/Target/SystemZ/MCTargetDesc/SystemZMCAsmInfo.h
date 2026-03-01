//====-- SystemZMCAsmInfo.h - SystemZ asm properties -----------*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZMCASMINFO_H
#define LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCAsmInfoGOFF.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {
class Triple;
enum SystemZAsmDialect { AD_GNU = 0, AD_HLASM = 1 };

class SystemZMCAsmInfoELF : public MCAsmInfoELF {
public:
  explicit SystemZMCAsmInfoELF(const Triple &TT);
};

class SystemZMCAsmInfoGOFF : public MCAsmInfoGOFF {
public:
  explicit SystemZMCAsmInfoGOFF(const Triple &TT);
  bool isAcceptableChar(char C) const override;
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

namespace SystemZ {
using Specifier = uint16_t;
enum {
  S_None,

  S_DTPOFF,
  S_GOT,
  S_GOTENT,
  S_INDNTPOFF,
  S_NTPOFF,
  S_PLT,
  S_TLSGD,
  S_TLSLD,
  S_TLSLDM,

  // HLASM docs for address constants:
  // https://www.ibm.com/docs/en/hla-and-tf/1.6?topic=value-address-constants
  S_RCon, // Address of ADA of symbol.
  S_VCon, // Address of external function symbol.
  S_QCon, // Class-based offset.
};
} // namespace SystemZ

} // end namespace vm::core

#endif
