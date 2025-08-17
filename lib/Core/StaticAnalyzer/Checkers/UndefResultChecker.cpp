//=== UndefResultChecker.cpp ------------------------------------*- C++ -*-===//
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
// This defines UndefResultChecker, a builtin check in ExprEngine that
// performs checks for undefined results of non-assignment binary operators.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace ento;

namespace {
class UndefResultChecker
  : public Checker< check::PostStmt<BinaryOperator> > {

  const BugType BT{this, "Result of operation is garbage or undefined"};

public:
  void checkPostStmt(const BinaryOperator *B, CheckerContext &C) const;
};
} // end anonymous namespace

static bool isArrayIndexOutOfBounds(CheckerContext &C, const Expr *Ex) {
  ProgramStateRef state = C.getState();

  if (!isa<ArraySubscriptExpr>(Ex))
    return false;

  SVal Loc = C.getSVal(Ex);
  if (!Loc.isValid())
    return false;

  const MemRegion *MR = Loc.castAs<loc::MemRegionVal>().getRegion();
  const ElementRegion *ER = dyn_cast<ElementRegion>(MR);
  if (!ER)
    return false;

  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();
  DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      state, ER->getSuperRegion(), C.getSValBuilder(), ER->getValueType());
  ProgramStateRef StInBound, StOutBound;
  std::tie(StInBound, StOutBound) = state->assumeInBoundDual(Idx, ElementCount);
  return StOutBound && !StInBound;
}

void UndefResultChecker::checkPostStmt(const BinaryOperator *B,
                                       CheckerContext &C) const {
  if (C.getSVal(B).isUndef()) {

    // Do not report assignments of uninitialized values inside swap functions.
    // This should allow to swap partially uninitialized structs
    if (const FunctionDecl *EnclosingFunctionDecl =
        dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
      if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
        return;

    // Generate an error node.
    ExplodedNode *N = C.generateErrorNode();
    if (!N)
      return;

    SmallString<256> sbuf;
    toolchain::raw_svector_ostream OS(sbuf);
    const Expr *Ex = nullptr;
    bool isLeft = true;

    if (C.getSVal(B->getLHS()).isUndef()) {
      Ex = B->getLHS()->IgnoreParenCasts();
      isLeft = true;
    }
    else if (C.getSVal(B->getRHS()).isUndef()) {
      Ex = B->getRHS()->IgnoreParenCasts();
      isLeft = false;
    }

    if (Ex) {
      OS << "The " << (isLeft ? "left" : "right") << " operand of '"
         << BinaryOperator::getOpcodeStr(B->getOpcode())
         << "' is a garbage value";
      if (isArrayIndexOutOfBounds(C, Ex))
        OS << " due to array index out of bounds";
    } else {
      // Neither operand was undefined, but the result is undefined.
      OS << "The result of the '"
         << BinaryOperator::getOpcodeStr(B->getOpcode())
         << "' expression is undefined";
    }
    auto report = std::make_unique<PathSensitiveBugReport>(BT, OS.str(), N);
    if (Ex) {
      report->addRange(Ex->getSourceRange());
      bugreporter::trackExpressionValue(N, Ex, *report);
    }
    else
      bugreporter::trackExpressionValue(N, B, *report);

    C.emitReport(std::move(report));
  }
}

void ento::registerUndefResultChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefResultChecker>();
}

bool ento::shouldRegisterUndefResultChecker(const CheckerManager &mgr) {
  return true;
}
