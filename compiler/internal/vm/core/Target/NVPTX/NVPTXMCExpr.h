//===-- NVPTXMCExpr.h - NVPTX specific MC expression classes ----*- C++ -*-===//
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

// Modeled after ARMMCExpr

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXMCEXPR_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXMCEXPR_H

#include "vm/core/ADT/APFloat.h"
#include "vm/core/MC/MCExpr.h"
#include <utility>

namespace vm::core {

class NVPTXFloatMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_NVPTX_None,
    VK_NVPTX_BFLOAT_PREC_FLOAT, // FP constant in bfloat-precision
    VK_NVPTX_HALF_PREC_FLOAT,   // FP constant in half-precision
    VK_NVPTX_SINGLE_PREC_FLOAT, // FP constant in single-precision
    VK_NVPTX_DOUBLE_PREC_FLOAT  // FP constant in double-precision
  };

private:
  const VariantKind Kind;
  const APFloat Flt;

  explicit NVPTXFloatMCExpr(VariantKind Kind, APFloat Flt)
      : Kind(Kind), Flt(std::move(Flt)) {}

public:
  /// @name Construction
  /// @{

  static const NVPTXFloatMCExpr *create(VariantKind Kind, const APFloat &Flt,
                                        MCContext &Ctx);

  static const NVPTXFloatMCExpr *createConstantBFPHalf(const APFloat &Flt,
                                                       MCContext &Ctx) {
    return create(VK_NVPTX_BFLOAT_PREC_FLOAT, Flt, Ctx);
  }

  static const NVPTXFloatMCExpr *createConstantFPHalf(const APFloat &Flt,
                                                        MCContext &Ctx) {
    return create(VK_NVPTX_HALF_PREC_FLOAT, Flt, Ctx);
  }

  static const NVPTXFloatMCExpr *createConstantFPSingle(const APFloat &Flt,
                                                        MCContext &Ctx) {
    return create(VK_NVPTX_SINGLE_PREC_FLOAT, Flt, Ctx);
  }

  static const NVPTXFloatMCExpr *createConstantFPDouble(const APFloat &Flt,
                                                        MCContext &Ctx) {
    return create(VK_NVPTX_DOUBLE_PREC_FLOAT, Flt, Ctx);
  }

  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  APFloat getAPFloat() const { return Flt; }

/// @}

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override {
    return false;
  }
  void visitUsedExpr(MCStreamer &Streamer) const override {};
  MCFragment *findAssociatedFragment() const override { return nullptr; }

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
};

/// A wrapper for MCSymbolRefExpr that tells the assembly printer that the
/// symbol should be enclosed by generic().
class NVPTXGenericMCSymbolRefExpr : public MCTargetExpr {
private:
  const MCSymbolRefExpr *SymExpr;

  explicit NVPTXGenericMCSymbolRefExpr(const MCSymbolRefExpr *_SymExpr)
      : SymExpr(_SymExpr) {}

public:
  /// @name Construction
  /// @{

  static const NVPTXGenericMCSymbolRefExpr
  *create(const MCSymbolRefExpr *SymExpr, MCContext &Ctx);

  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  const MCSymbolRefExpr *getSymbolExpr() const { return SymExpr; }

  /// @}

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override {
    return false;
  }
  void visitUsedExpr(MCStreamer &Streamer) const override {};
  MCFragment *findAssociatedFragment() const override { return nullptr; }

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
  };
} // end namespace vm::core

#endif
