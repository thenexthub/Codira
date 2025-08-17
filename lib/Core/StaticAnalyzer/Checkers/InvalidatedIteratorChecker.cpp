//===-- InvalidatedIteratorChecker.cpp ----------------------------*- C++ -*--//
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
// Defines a checker for access of invalidated iterators.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"


#include "Iterator.h"

using namespace language::Core;
using namespace ento;
using namespace iterator;

namespace {

class InvalidatedIteratorChecker
  : public Checker<check::PreCall, check::PreStmt<UnaryOperator>,
                   check::PreStmt<BinaryOperator>,
                   check::PreStmt<ArraySubscriptExpr>,
                   check::PreStmt<MemberExpr>> {

  const BugType InvalidatedBugType{this, "Iterator invalidated",
                                   "Misuse of STL APIs"};

  void verifyAccess(CheckerContext &C, SVal Val) const;
  void reportBug(StringRef Message, SVal Val, CheckerContext &C,
                 ExplodedNode *ErrNode) const;

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreStmt(const UnaryOperator *UO, CheckerContext &C) const;
  void checkPreStmt(const BinaryOperator *BO, CheckerContext &C) const;
  void checkPreStmt(const ArraySubscriptExpr *ASE, CheckerContext &C) const;
  void checkPreStmt(const MemberExpr *ME, CheckerContext &C) const;

};

} // namespace

void InvalidatedIteratorChecker::checkPreCall(const CallEvent &Call,
                                              CheckerContext &C) const {
  // Check for access of invalidated position
  const auto *Func = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
  if (!Func)
    return;

  if (Func->isOverloadedOperator() &&
      isAccessOperator(Func->getOverloadedOperator())) {
    // Check for any kind of access of invalidated iterator positions
    if (const auto *InstCall = dyn_cast<CXXInstanceCall>(&Call)) {
      verifyAccess(C, InstCall->getCXXThisVal());
    } else {
      verifyAccess(C, Call.getArgSVal(0));
    }
  }
}

void InvalidatedIteratorChecker::checkPreStmt(const UnaryOperator *UO,
                                              CheckerContext &C) const {
  if (isa<CXXThisExpr>(UO->getSubExpr()))
    return;

  ProgramStateRef State = C.getState();
  UnaryOperatorKind OK = UO->getOpcode();
  SVal SubVal = State->getSVal(UO->getSubExpr(), C.getLocationContext());

  if (isAccessOperator(OK)) {
    verifyAccess(C, SubVal);
  }
}

void InvalidatedIteratorChecker::checkPreStmt(const BinaryOperator *BO,
                                              CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  BinaryOperatorKind OK = BO->getOpcode();
  SVal LVal = State->getSVal(BO->getLHS(), C.getLocationContext());

  if (isAccessOperator(OK)) {
    verifyAccess(C, LVal);
  }
}

void InvalidatedIteratorChecker::checkPreStmt(const ArraySubscriptExpr *ASE,
                                              CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SVal LVal = State->getSVal(ASE->getLHS(), C.getLocationContext());
  verifyAccess(C, LVal);
}

void InvalidatedIteratorChecker::checkPreStmt(const MemberExpr *ME,
                                              CheckerContext &C) const {
  if (!ME->isArrow() || ME->isImplicitAccess())
    return;

  ProgramStateRef State = C.getState();
  SVal BaseVal = State->getSVal(ME->getBase(), C.getLocationContext());
  verifyAccess(C, BaseVal);
}

void InvalidatedIteratorChecker::verifyAccess(CheckerContext &C,
                                              SVal Val) const {
  auto State = C.getState();
  const auto *Pos = getIteratorPosition(State, Val);
  if (Pos && !Pos->isValid()) {
    auto *N = C.generateErrorNode(State);
    if (!N) {
      return;
    }
    reportBug("Invalidated iterator accessed.", Val, C, N);
  }
}

void InvalidatedIteratorChecker::reportBug(StringRef Message, SVal Val,
                                           CheckerContext &C,
                                           ExplodedNode *ErrNode) const {
  auto R = std::make_unique<PathSensitiveBugReport>(InvalidatedBugType, Message,
                                                    ErrNode);
  R->markInteresting(Val);
  C.emitReport(std::move(R));
}

void ento::registerInvalidatedIteratorChecker(CheckerManager &mgr) {
  mgr.registerChecker<InvalidatedIteratorChecker>();
}

bool ento::shouldRegisterInvalidatedIteratorChecker(const CheckerManager &mgr) {
  return true;
}
