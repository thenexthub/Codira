//==- HexagonMCExpr.h - Hexagon specific MC expression classes --*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONMCEXPR_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONMCEXPR_H

#include "vm/core/MC/MCExpr.h"

namespace vm::core {
class HexagonMCExpr : public MCTargetExpr {
public:
  enum VariantKind : uint8_t {
    VK_None,

    VK_DTPREL = MCSymbolRefExpr::FirstTargetSpecifier,
    VK_GD_GOT,
    VK_GD_PLT,
    VK_GOT,
    VK_GOTREL,
    VK_IE,
    VK_IE_GOT,
    VK_LD_GOT,
    VK_LD_PLT,
    VK_PCREL,
    VK_PLT,
    VK_TPREL,

    VK_LO16,
    VK_HI16,
    VK_GPREL,
  };

  static HexagonMCExpr *create(MCExpr const *Expr, MCContext &Ctx);
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override;
  MCExpr const *getExpr() const;
  void setMustExtend(bool Val = true);
  bool mustExtend() const;
  void setMustNotExtend(bool Val = true);
  bool mustNotExtend() const;
  void setS27_2_reloc(bool Val = true);
  bool s27_2_reloc() const;
  void setSignMismatch(bool Val = true);
  bool signMismatch() const;

private:
  HexagonMCExpr(MCExpr const *Expr);
  MCExpr const *Expr;
  bool MustNotExtend;
  bool MustExtend;
  bool S27_2_reloc;
  bool SignMismatch;
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONMCEXPR_H
