//=== FixedAddressChecker.cpp - Fixed address usage checker ----*- C++ -*--===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This files defines FixedAddressChecker, a builtin checker that checks for
// assignment of a fixed address to a pointer.
// This check corresponds to CWE-587.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class FixedAddressChecker
  : public Checker< check::PreStmt<BinaryOperator> > {
  const BugType BT{this, "Use fixed address"};

public:
  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;
};
}

void FixedAddressChecker::checkPreStmt(const BinaryOperator *B,
                                       CheckerContext &C) const {
  // Using a fixed address is not portable because that address will probably
  // not be valid in all environments or platforms.

  if (B->getOpcode() != BO_Assign)
    return;

  QualType T = B->getType();
  if (!T->isPointerType())
    return;

  // Omit warning if the RHS has already pointer type. Without this passing
  // around one fixed value in several pointer variables would produce several
  // redundant warnings.
  if (B->getRHS()->IgnoreParenCasts()->getType()->isPointerType())
    return;

  SVal RV = C.getSVal(B->getRHS());

  if (!RV.isConstant() || RV.isZeroConstant())
    return;

  if (C.getSourceManager().isInSystemMacro(B->getRHS()->getBeginLoc()))
    return;

  if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
    // FIXME: improve grammar in the following strings:
    constexpr toolchain::StringLiteral Msg =
        "Using a fixed address is not portable because that address will "
        "probably not be valid in all environments or platforms.";
    auto R = std::make_unique<PathSensitiveBugReport>(BT, Msg, N);
    R->addRange(B->getRHS()->getSourceRange());
    C.emitReport(std::move(R));
  }
}

void ento::registerFixedAddressChecker(CheckerManager &mgr) {
  mgr.registerChecker<FixedAddressChecker>();
}

bool ento::shouldRegisterFixedAddressChecker(const CheckerManager &mgr) {
  return true;
}
