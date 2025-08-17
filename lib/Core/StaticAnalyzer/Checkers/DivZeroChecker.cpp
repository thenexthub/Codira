//== DivZeroChecker.cpp - Division by zero checker --------------*- C++ -*--==//
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
// This defines DivZeroChecker, a builtin check in ExprEngine that performs
// checks for division by zeros.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Checkers/Taint.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/CommonBugCategories.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <optional>

using namespace language::Core;
using namespace ento;
using namespace taint;

namespace {
class DivZeroChecker : public CheckerFamily<check::PreStmt<BinaryOperator>> {
  void reportBug(StringRef Msg, ProgramStateRef StateZero,
                 CheckerContext &C) const;
  void reportTaintBug(StringRef Msg, ProgramStateRef StateZero,
                      CheckerContext &C,
                      toolchain::ArrayRef<SymbolRef> TaintedSyms) const;

public:
  /// This checker family implements two user-facing checker parts.
  CheckerFrontendWithBugType DivideZeroChecker{"Division by zero"};
  CheckerFrontendWithBugType TaintedDivChecker{"Division by zero",
                                               categories::TaintedData};

  void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const;

  /// Identifies this checker family for debugging purposes.
  StringRef getDebugTag() const override { return "DivZeroChecker"; }
};
} // end anonymous namespace

static const Expr *getDenomExpr(const ExplodedNode *N) {
  const Stmt *S = N->getLocationAs<PreStmt>()->getStmt();
  if (const auto *BE = dyn_cast<BinaryOperator>(S))
    return BE->getRHS();
  return nullptr;
}

void DivZeroChecker::reportBug(StringRef Msg, ProgramStateRef StateZero,
                               CheckerContext &C) const {
  if (!DivideZeroChecker.isEnabled())
    return;
  if (ExplodedNode *N = C.generateErrorNode(StateZero)) {
    auto R =
        std::make_unique<PathSensitiveBugReport>(DivideZeroChecker, Msg, N);
    bugreporter::trackExpressionValue(N, getDenomExpr(N), *R);
    C.emitReport(std::move(R));
  }
}

void DivZeroChecker::reportTaintBug(
    StringRef Msg, ProgramStateRef StateZero, CheckerContext &C,
    toolchain::ArrayRef<SymbolRef> TaintedSyms) const {
  if (!TaintedDivChecker.isEnabled())
    return;
  if (ExplodedNode *N = C.generateErrorNode(StateZero)) {
    auto R =
        std::make_unique<PathSensitiveBugReport>(TaintedDivChecker, Msg, N);
    bugreporter::trackExpressionValue(N, getDenomExpr(N), *R);
    for (auto Sym : TaintedSyms)
      R->markInteresting(Sym);
    C.emitReport(std::move(R));
  }
}

void DivZeroChecker::checkPreStmt(const BinaryOperator *B,
                                  CheckerContext &C) const {
  BinaryOperator::Opcode Op = B->getOpcode();
  if (Op != BO_Div &&
      Op != BO_Rem &&
      Op != BO_DivAssign &&
      Op != BO_RemAssign)
    return;

  if (!B->getRHS()->getType()->isScalarType())
    return;

  SVal Denom = C.getSVal(B->getRHS());
  std::optional<DefinedSVal> DV = Denom.getAs<DefinedSVal>();

  // Divide-by-undefined handled in the generic checking for uses of
  // undefined values.
  if (!DV)
    return;

  // Check for divide by zero.
  ConstraintManager &CM = C.getConstraintManager();
  ProgramStateRef stateNotZero, stateZero;
  std::tie(stateNotZero, stateZero) = CM.assumeDual(C.getState(), *DV);

  if (!stateNotZero) {
    assert(stateZero);
    reportBug("Division by zero", stateZero, C);
    return;
  }

  if ((stateNotZero && stateZero)) {
    std::vector<SymbolRef> taintedSyms = getTaintedSymbols(C.getState(), *DV);
    if (!taintedSyms.empty()) {
      reportTaintBug("Division by a tainted value, possibly zero", stateZero, C,
                     taintedSyms);
      // Fallthrough to continue analysis in case of non-zero denominator.
    }
  }

  // If we get here, then the denom should not be zero. We abandon the implicit
  // zero denom case for now.
  C.addTransition(stateNotZero);
}

void ento::registerDivZeroChecker(CheckerManager &Mgr) {
  Mgr.getChecker<DivZeroChecker>()->DivideZeroChecker.enable(Mgr);
}

bool ento::shouldRegisterDivZeroChecker(const CheckerManager &) { return true; }

void ento::registerTaintedDivChecker(CheckerManager &Mgr) {
  Mgr.getChecker<DivZeroChecker>()->TaintedDivChecker.enable(Mgr);
}

bool ento::shouldRegisterTaintedDivChecker(const CheckerManager &) {
  return true;
}
