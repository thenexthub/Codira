//===-- HexagonMCExpr.cpp - Hexagon specific MC expression classes
//----------===//
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

#include "HexagonMCExpr.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "hexagon-mcexpr"

HexagonMCExpr *HexagonMCExpr::create(MCExpr const *Expr, MCContext &Ctx) {
  return new (Ctx) HexagonMCExpr(Expr);
}

bool HexagonMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                              const MCAssembler *Asm) const {
  return Expr->evaluateAsRelocatable(Res, Asm);
}

void HexagonMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*Expr);
}

MCFragment *toolchain::HexagonMCExpr::findAssociatedFragment() const {
  return Expr->findAssociatedFragment();
}

MCExpr const *HexagonMCExpr::getExpr() const { return Expr; }

void HexagonMCExpr::setMustExtend(bool Val) {
  assert((!Val || !MustNotExtend) && "Extension contradiction");
  MustExtend = Val;
}

bool HexagonMCExpr::mustExtend() const { return MustExtend; }
void HexagonMCExpr::setMustNotExtend(bool Val) {
  assert((!Val || !MustExtend) && "Extension contradiction");
  MustNotExtend = Val;
}
bool HexagonMCExpr::mustNotExtend() const { return MustNotExtend; }

bool HexagonMCExpr::s27_2_reloc() const { return S27_2_reloc; }
void HexagonMCExpr::setS27_2_reloc(bool Val) {
  S27_2_reloc = Val;
}

HexagonMCExpr::HexagonMCExpr(MCExpr const *Expr)
    : Expr(Expr), MustNotExtend(false), MustExtend(false), S27_2_reloc(false),
      SignMismatch(false) {}

void HexagonMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  MAI->printExpr(OS, *Expr);
}

void HexagonMCExpr::setSignMismatch(bool Val) {
  SignMismatch = Val;
}

bool HexagonMCExpr::signMismatch() const {
  return SignMismatch;
}
