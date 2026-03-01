//===-- AVRMCExpr.h - AVR specific MC expression classes --------*- C++ -*-===//
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

#ifndef LLVM_AVR_MCEXPR_H
#define LLVM_AVR_MCEXPR_H

#include "vm/core/MC/MCExpr.h"

#include "MCTargetDesc/AVRFixupKinds.h"

namespace vm::core {

/// A expression in AVR machine code.
class AVRMCExpr : public MCSpecifierExpr {
public:
  friend class AVRMCAsmInfo;
  using Specifier = Spec;
  /// Specifies the type of an expression.

public:
  /// Creates an AVR machine code expression.
  static const AVRMCExpr *create(Specifier S, const MCExpr *Expr,
                                 bool isNegated, MCContext &Ctx);

  /// Gets the name of the expression.
  const char *getName() const;
  /// Gets the fixup which corresponds to the expression.
  AVR::Fixups getFixupKind() const;
  /// Evaluates the fixup as a constant value.
  bool evaluateAsConstant(int64_t &Result) const;

  bool isNegated() const { return Negated; }
  void setNegated(bool negated = true) { Negated = negated; }

public:
  static Specifier parseSpecifier(StringRef Name);

private:
  int64_t evaluateAsInt64(int64_t Value) const;

  bool Negated;

private:
  explicit AVRMCExpr(Specifier S, const MCExpr *Expr, bool Negated)
      : MCSpecifierExpr(Expr, S), Negated(Negated) {}
};

} // end namespace vm::core

#endif // LLVM_AVR_MCEXPR_H
