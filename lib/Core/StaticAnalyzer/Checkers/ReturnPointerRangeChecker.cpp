//== ReturnPointerRangeChecker.cpp ------------------------------*- C++ -*--==//
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
// This file defines ReturnPointerRangeChecker, which is a path-sensitive check
// which looks for an out-of-bound pointer being returned to callers.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporterVisitors.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace language::Core;
using namespace ento;

namespace {
class ReturnPointerRangeChecker :
    public Checker< check::PreStmt<ReturnStmt> > {
  // FIXME: This bug correspond to CWE-466.  Eventually we should have bug
  // types explicitly reference such exploit categories (when applicable).
  const BugType BT{this, "Buffer overflow"};

public:
    void checkPreStmt(const ReturnStmt *RS, CheckerContext &C) const;
};
}

void ReturnPointerRangeChecker::checkPreStmt(const ReturnStmt *RS,
                                             CheckerContext &C) const {
  ProgramStateRef state = C.getState();

  const Expr *RetE = RS->getRetValue();
  if (!RetE)
    return;

  // Skip "body farmed" functions.
  if (RetE->getSourceRange().isInvalid())
    return;

  SVal V = C.getSVal(RetE);
  const MemRegion *R = V.getAsRegion();

  const ElementRegion *ER = dyn_cast_or_null<ElementRegion>(R);
  if (!ER)
    return;

  DefinedOrUnknownSVal Idx = ER->getIndex().castAs<DefinedOrUnknownSVal>();
  // Zero index is always in bound, this also passes ElementRegions created for
  // pointer casts.
  if (Idx.isZeroConstant())
    return;

  // FIXME: All of this out-of-bounds checking should eventually be refactored
  // into a common place.
  DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      state, ER->getSuperRegion(), C.getSValBuilder(), ER->getValueType());

  // We assume that the location after the last element in the array is used as
  // end() iterator. Reporting on these would return too many false positives.
  if (Idx == ElementCount)
    return;

  ProgramStateRef StInBound, StOutBound;
  std::tie(StInBound, StOutBound) = state->assumeInBoundDual(Idx, ElementCount);
  if (StOutBound && !StInBound) {
    ExplodedNode *N = C.generateErrorNode(StOutBound);

    if (!N)
      return;

    constexpr toolchain::StringLiteral Msg =
        "Returned pointer value points outside the original object "
        "(potential buffer overflow)";

    // Generate a report for this bug.
    auto Report = std::make_unique<PathSensitiveBugReport>(BT, Msg, N);
    Report->addRange(RetE->getSourceRange());

    const auto ConcreteElementCount = ElementCount.getAs<nonloc::ConcreteInt>();
    const auto ConcreteIdx = Idx.getAs<nonloc::ConcreteInt>();

    const auto *DeclR = ER->getSuperRegion()->getAs<DeclRegion>();

    if (DeclR)
      Report->addNote("Original object declared here",
                      {DeclR->getDecl(), C.getSourceManager()});

    if (ConcreteElementCount) {
      SmallString<128> SBuf;
      toolchain::raw_svector_ostream OS(SBuf);
      OS << "Original object ";
      if (DeclR) {
        OS << "'";
        DeclR->getDecl()->printName(OS);
        OS << "' ";
      }
      OS << "is an array of " << ConcreteElementCount->getValue() << " '";
      ER->getValueType().print(OS,
                               PrintingPolicy(C.getASTContext().getLangOpts()));
      OS << "' objects";
      if (ConcreteIdx) {
        OS << ", returned pointer points at index " << ConcreteIdx->getValue();
      }

      Report->addNote(SBuf,
                      {RetE, C.getSourceManager(), C.getLocationContext()});
    }

    bugreporter::trackExpressionValue(N, RetE, *Report);

    C.emitReport(std::move(Report));
  }
}

void ento::registerReturnPointerRangeChecker(CheckerManager &mgr) {
  mgr.registerChecker<ReturnPointerRangeChecker>();
}

bool ento::shouldRegisterReturnPointerRangeChecker(const CheckerManager &mgr) {
  return true;
}
