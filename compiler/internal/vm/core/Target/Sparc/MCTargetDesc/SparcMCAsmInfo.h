//===- SparcMCAsmInfo.h - Sparc asm properties -----------------*- C++ -*--===//
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
// This file contains the declaration of the SparcMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"

namespace vm::core {

class Triple;

class SparcELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit SparcELFMCAsmInfo(const Triple &TheTriple);

  const MCExpr*
  getExprForPersonalitySymbol(const MCSymbol *Sym, unsigned Encoding,
                              MCStreamer &Streamer) const override;
  const MCExpr* getExprForFDESymbol(const MCSymbol *Sym,
                                    unsigned Encoding,
                                    MCStreamer &Streamer) const override;

  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace Sparc {
uint16_t parseSpecifier(StringRef name);
StringRef getSpecifierName(uint16_t S);
} // namespace Sparc

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H
