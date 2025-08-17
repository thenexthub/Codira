//== BoolAssignmentChecker.cpp - Boolean assignment checker -----*- C++ -*--==//
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
// This defines BoolAssignmentChecker, a builtin check in ExprEngine that
// performs checks for assignment of non-Boolean values to Boolean variables.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Checkers/Taint.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <optional>

using namespace language::Core;
using namespace ento;

namespace {
class BoolAssignmentChecker : public Checker<check::Bind> {
  const BugType BT{this, "Assignment of a non-Boolean value"};
  void emitReport(ProgramStateRef State, CheckerContext &C,
                  bool IsTainted = false) const;

public:
  void checkBind(SVal Loc, SVal Val, const Stmt *S, bool AtDeclInit,
                 CheckerContext &C) const;
};
} // end anonymous namespace

void BoolAssignmentChecker::emitReport(ProgramStateRef State, CheckerContext &C,
                                       bool IsTainted) const {
  if (ExplodedNode *N = C.generateNonFatalErrorNode(State)) {
    StringRef Msg = IsTainted ? "Might assign a tainted non-Boolean value"
                              : "Assignment of a non-Boolean value";
    C.emitReport(std::make_unique<PathSensitiveBugReport>(BT, Msg, N));
  }
}

static bool isBooleanType(QualType Ty) {
  if (Ty->isBooleanType()) // C++ or C99
    return true;

  if (const TypedefType *TT = Ty->getAs<TypedefType>())
    return TT->getDecl()->getName() == "BOOL" ||  // Objective-C
           TT->getDecl()->getName() == "_Bool" || // stdbool.h < C99
           TT->getDecl()->getName() == "Boolean"; // MacTypes.h

  return false;
}

void BoolAssignmentChecker::checkBind(SVal Loc, SVal Val, const Stmt *S,
                                      bool AtDeclInit,
                                      CheckerContext &C) const {

  // We are only interested in stores into Booleans.
  const TypedValueRegion *TR =
      dyn_cast_or_null<TypedValueRegion>(Loc.getAsRegion());

  if (!TR)
    return;

  QualType RegTy = TR->getValueType();

  if (!isBooleanType(RegTy))
    return;

  // Get the value of the right-hand side.  We only care about values
  // that are defined (UnknownVals and UndefinedVals are handled by other
  // checkers).
  std::optional<NonLoc> NV = Val.getAs<NonLoc>();
  if (!NV)
    return;

  // Check if the assigned value meets our criteria for correctness.  It must
  // be a value that is either 0 or 1.  One way to check this is to see if
  // the value is possibly < 0 (for a negative value) or greater than 1.
  ProgramStateRef State = C.getState();
  BasicValueFactory &BVF = C.getSValBuilder().getBasicValueFactory();
  ConstraintManager &CM = C.getConstraintManager();

  toolchain::APSInt Zero = BVF.getValue(0, RegTy);
  toolchain::APSInt One = BVF.getValue(1, RegTy);

  ProgramStateRef StIn, StOut;
  std::tie(StIn, StOut) = CM.assumeInclusiveRangeDual(State, *NV, Zero, One);

  if (!StIn)
    emitReport(StOut, C);
  if (StIn && StOut && taint::isTainted(State, *NV))
    emitReport(StOut, C, /*IsTainted=*/true);
}

void ento::registerBoolAssignmentChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<BoolAssignmentChecker>();
}

bool ento::shouldRegisterBoolAssignmentChecker(const CheckerManager &Mgr) {
  return true;
}
