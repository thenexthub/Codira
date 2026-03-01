//===-- LoongArchMCAsmInfo.h - LoongArch Asm Info --------------*- C++ -*--===//
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
// This file contains the declaration of the LoongArchMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHMCASMINFO_H
#define LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"

namespace vm::core {
class Triple;
class StringRef;

class LoongArchMCExpr : public MCSpecifierExpr {
public:
  using Specifier = uint16_t;
  enum { VK_None };

private:
  const bool RelaxHint;

  explicit LoongArchMCExpr(const MCExpr *Expr, Specifier S, bool Hint)
      : MCSpecifierExpr(Expr, S), RelaxHint(Hint) {}

public:
  static const LoongArchMCExpr *create(const MCExpr *Expr, uint16_t S,
                                       MCContext &Ctx, bool Hint = false);

  bool getRelaxHint() const { return RelaxHint; }
};

class LoongArchMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit LoongArchMCAsmInfo(const Triple &TargetTriple);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace LoongArch {
uint16_t parseSpecifier(StringRef name);
} // namespace LoongArch

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHMCASMINFO_H
