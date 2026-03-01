//===-- XtensaMCAsmInfo.h - Xtensa Asm Info --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
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
// This file contains the declaration of the XtensaMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSATARGETASMINFO_H
#define LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSATARGETASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"

namespace vm::core {
class Triple;
class StringRef;

class XtensaMCAsmInfo : public MCAsmInfoELF {
public:
  explicit XtensaMCAsmInfo(const Triple &TT);

  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace Xtensa {
enum Specifier { S_None, S_TPOFF };

uint8_t parseSpecifier(StringRef name);
StringRef getSpecifierName(uint8_t S);
} // namespace Xtensa

} // namespace vm::core

#endif // LLVM_LIB_TARGET_XTENSA_MCTARGETDESC_XTENSATARGETASMINFO_H
