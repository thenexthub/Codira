//===-- ChrootChecker.cpp - chroot usage checks ---------------------------===//
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
//  This file defines chroot checker, which checks improper use of chroot.
//  This is described by the SEI Cert C rule POS05-C.
//  The checker is a warning not a hard failure since it only checks for a
//  recommended rule.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTContext.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"

using namespace language::Core;
using namespace ento;

namespace {
enum ChrootKind { NO_CHROOT, ROOT_CHANGED, ROOT_CHANGE_FAILED, JAIL_ENTERED };
} // namespace

// Track chroot state changes for success, failure, state change
// and "jail"
REGISTER_TRAIT_WITH_PROGRAMSTATE(ChrootState, ChrootKind)
namespace {

// This checker checks improper use of chroot.
// The state transitions
//
//                          -> ROOT_CHANGE_FAILED
//                          |
// NO_CHROOT ---chroot(path)--> ROOT_CHANGED ---chdir(/) --> JAIL_ENTERED
//                                  |                               |
//         ROOT_CHANGED<--chdir(..)--      JAIL_ENTERED<--chdir(..)--
//                                  |                               |
//                      bug<--foo()--          JAIL_ENTERED<--foo()--
//
class ChrootChecker final : public Checker<eval::Call, check::PreCall> {
public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

private:
  bool evalChroot(const CallEvent &Call, CheckerContext &C) const;
  bool evalChdir(const CallEvent &Call, CheckerContext &C) const;

  const BugType BreakJailBug{this, "Break out of jail"};
  const CallDescription Chroot{CDM::CLibrary, {"chroot"}, 1};
  const CallDescription Chdir{CDM::CLibrary, {"chdir"}, 1};
};

bool ChrootChecker::evalCall(const CallEvent &Call, CheckerContext &C) const {
  if (Chroot.matches(Call))
    return evalChroot(Call, C);

  if (Chdir.matches(Call))
    return evalChdir(Call, C);

  return false;
}

bool ChrootChecker::evalChroot(const CallEvent &Call, CheckerContext &C) const {
  BasicValueFactory &BVF = C.getSValBuilder().getBasicValueFactory();
  const LocationContext *LCtx = C.getLocationContext();
  ProgramStateRef State = C.getState();
  const auto *CE = cast<CallExpr>(Call.getOriginExpr());

  const QualType IntTy = C.getASTContext().IntTy;
  SVal Zero = nonloc::ConcreteInt{BVF.getValue(0, IntTy)};
  SVal Minus1 = nonloc::ConcreteInt{BVF.getValue(-1, IntTy)};

  ProgramStateRef ChrootFailed = State->BindExpr(CE, LCtx, Minus1);
  C.addTransition(ChrootFailed->set<ChrootState>(ROOT_CHANGE_FAILED));

  ProgramStateRef ChrootSucceeded = State->BindExpr(CE, LCtx, Zero);
  C.addTransition(ChrootSucceeded->set<ChrootState>(ROOT_CHANGED));
  return true;
}

bool ChrootChecker::evalChdir(const CallEvent &Call, CheckerContext &C) const {
  ProgramStateRef State = C.getState();

  // If there are no jail state, just return.
  if (State->get<ChrootState>() == NO_CHROOT)
    return false;

  // After chdir("/"), enter the jail, set the enum value JAIL_ENTERED.
  SVal ArgVal = Call.getArgSVal(0);

  if (const MemRegion *R = ArgVal.getAsRegion()) {
    R = R->StripCasts();
    if (const auto *StrRegion = dyn_cast<StringRegion>(R)) {
      if (StrRegion->getStringLiteral()->getString() == "/") {
        C.addTransition(State->set<ChrootState>(JAIL_ENTERED));
        return true;
      }
    }
  }
  return false;
}

class ChrootInvocationVisitor final : public BugReporterVisitor {
public:
  explicit ChrootInvocationVisitor(const CallDescription &Chroot)
      : Chroot{Chroot} {}

  PathDiagnosticPieceRef VisitNode(const ExplodedNode *N,
                                   BugReporterContext &BRC,
                                   PathSensitiveBugReport &BR) override {
    if (Satisfied)
      return nullptr;

    auto StmtP = N->getLocation().getAs<StmtPoint>();
    if (!StmtP)
      return nullptr;

    const CallExpr *Call = StmtP->getStmtAs<CallExpr>();
    if (!Call)
      return nullptr;

    if (!Chroot.matchesAsWritten(*Call))
      return nullptr;

    Satisfied = true;
    PathDiagnosticLocation Pos(Call, BRC.getSourceManager(),
                               N->getLocationContext());
    return std::make_shared<PathDiagnosticEventPiece>(Pos, "chroot called here",
                                                      /*addPosRange=*/true);
  }

  void Profile(toolchain::FoldingSetNodeID &ID) const override {
    static bool Tag;
    ID.AddPointer(&Tag);
  }

private:
  const CallDescription &Chroot;
  bool Satisfied = false;
};

// Check the jail state before any function call except chroot and chdir().
void ChrootChecker::checkPreCall(const CallEvent &Call,
                                 CheckerContext &C) const {
  // Ignore chroot and chdir.
  if (matchesAny(Call, Chroot, Chdir))
    return;

  // If jail state is not ROOT_CHANGED just return.
  if (C.getState()->get<ChrootState>() != ROOT_CHANGED)
    return;

  // Generate bug report.
  ExplodedNode *Err =
      C.generateNonFatalErrorNode(C.getState(), C.getPredecessor());
  if (!Err)
    return;

  auto R = std::make_unique<PathSensitiveBugReport>(
      BreakJailBug, R"(No call of chdir("/") immediately after chroot)", Err);
  R->addVisitor<ChrootInvocationVisitor>(Chroot);
  C.emitReport(std::move(R));
}

} // namespace

void ento::registerChrootChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<ChrootChecker>();
}

bool ento::shouldRegisterChrootChecker(const CheckerManager &) { return true; }
