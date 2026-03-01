//=--- X86MCExpr.h - X86 specific MC expression classes ---*- C++ -*-=//
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
// This file describes X86-specific MCExprs, i.e, registers used for
// extended variable assignments.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MCEXPR_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MCEXPR_H

#include "X86ATTInstPrinter.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

class X86MCExpr : public MCTargetExpr {

private:
  const MCRegister Reg; // All

  explicit X86MCExpr(MCRegister R) : Reg(R) {}

public:
  static const X86MCExpr *create(MCRegister Reg, MCContext &Ctx) {
    return new (Ctx) X86MCExpr(Reg);
  }

  /// getSubExpr - Get the child of this expression.
  MCRegister getReg() const { return Reg; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override {
    if (!MAI || MAI->getAssemblerDialect() == 0)
      OS << '%';
    OS << X86ATTInstPrinter::getRegisterName(Reg);
  }

  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override {
    return false;
  }
  // Register values should be inlined as they are not valid .set expressions.
  bool inlineAssignedExpr() const override { return true; }
  bool isEqualTo(const MCExpr *X) const override {
    if (auto *E = dyn_cast<X86MCExpr>(X))
      return getReg() == E->getReg();
    return false;
  }
  void visitUsedExpr(MCStreamer &Streamer) const override {}
  MCFragment *findAssociatedFragment() const override { return nullptr; }

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
};
} // end namespace vm::core

#endif
