//===-- SetgidSetuidOrderChecker.cpp - check privilege revocation calls ---===//
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
//  This file defines a checker to detect possible reversed order of privilege
//  revocations when 'setgid' and 'setuid' is used.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"

using namespace language::Core;
using namespace ento;

namespace {

enum SetPrivilegeFunctionKind { Irrelevant, Setuid, Setgid };

class SetgidSetuidOrderChecker : public Checker<check::PostCall, eval::Assume> {
  const BugType BT{this, "Possible wrong order of privilege revocation"};

  const CallDescription SetuidDesc{CDM::CLibrary, {"setuid"}, 1};
  const CallDescription SetgidDesc{CDM::CLibrary, {"setgid"}, 1};

  const CallDescription GetuidDesc{CDM::CLibrary, {"getuid"}, 0};
  const CallDescription GetgidDesc{CDM::CLibrary, {"getgid"}, 0};

  const CallDescriptionSet OtherSetPrivilegeDesc{
      {CDM::CLibrary, {"seteuid"}, 1},   {CDM::CLibrary, {"setegid"}, 1},
      {CDM::CLibrary, {"setreuid"}, 2},  {CDM::CLibrary, {"setregid"}, 2},
      {CDM::CLibrary, {"setresuid"}, 3}, {CDM::CLibrary, {"setresgid"}, 3}};

public:
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  ProgramStateRef evalAssume(ProgramStateRef State, SVal Cond,
                             bool Assumption) const;

private:
  void processSetuid(ProgramStateRef State, const CallEvent &Call,
                     CheckerContext &C) const;
  void processSetgid(ProgramStateRef State, const CallEvent &Call,
                     CheckerContext &C) const;
  void processOther(ProgramStateRef State, const CallEvent &Call,
                    CheckerContext &C) const;
  /// Check if a function like \c getuid or \c getgid is called directly from
  /// the first argument of function called from \a Call.
  bool isFunctionCalledInArg(const CallDescription &Desc,
                             const CallEvent &Call) const;
  void emitReport(ProgramStateRef State, CheckerContext &C) const;
};

} // end anonymous namespace

/// Store if there was a call to 'setuid(getuid())' or 'setgid(getgid())' not
/// followed by other different privilege-change functions.
/// If the value \c Setuid is stored and a 'setgid(getgid())' call is found we
/// have found the bug to be reported. Value \c Setgid is used too to prevent
/// warnings at a setgid-setuid-setgid sequence.
REGISTER_TRAIT_WITH_PROGRAMSTATE(LastSetPrivilegeCall, SetPrivilegeFunctionKind)
/// Store the symbol value of the last 'setuid(getuid())' call. This is used to
/// detect if the result is compared to -1 and avoid warnings on that branch
/// (which is the failure branch of the call), and for identification of note
/// tags.
REGISTER_TRAIT_WITH_PROGRAMSTATE(LastSetuidCallSVal, SymbolRef)

void SetgidSetuidOrderChecker::checkPostCall(const CallEvent &Call,
                                             CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (SetuidDesc.matches(Call)) {
    processSetuid(State, Call, C);
  } else if (SetgidDesc.matches(Call)) {
    processSetgid(State, Call, C);
  } else if (OtherSetPrivilegeDesc.contains(Call)) {
    processOther(State, Call, C);
  }
}

ProgramStateRef SetgidSetuidOrderChecker::evalAssume(ProgramStateRef State,
                                                     SVal Cond,
                                                     bool Assumption) const {
  SValBuilder &SVB = State->getStateManager().getSValBuilder();
  SymbolRef LastSetuidSym = State->get<LastSetuidCallSVal>();
  if (!LastSetuidSym)
    return State;

  // Check if the most recent call to 'setuid(getuid())' is assumed to be != 0.
  // It should be only -1 at failure, but we want to accept a "!= 0" check too.
  // (But now an invalid failure check like "!= 1" will be recognized as correct
  // too. The "invalid failure check" is a different bug that is not the scope
  // of this checker.)
  auto FailComparison =
      SVB.evalBinOpNN(State, BO_NE, nonloc::SymbolVal(LastSetuidSym),
                      SVB.makeIntVal(0, /*isUnsigned=*/false),
                      SVB.getConditionType())
          .getAs<DefinedOrUnknownSVal>();
  if (!FailComparison)
    return State;
  if (auto IsFailBranch = State->assume(*FailComparison);
      IsFailBranch.first && !IsFailBranch.second) {
    // This is the 'setuid(getuid())' != 0 case.
    // On this branch we do not want to emit warning.
    State = State->set<LastSetPrivilegeCall>(Irrelevant);
    State = State->set<LastSetuidCallSVal>(SymbolRef{});
  }
  return State;
}

void SetgidSetuidOrderChecker::processSetuid(ProgramStateRef State,
                                             const CallEvent &Call,
                                             CheckerContext &C) const {
  bool IsSetuidWithGetuid = isFunctionCalledInArg(GetuidDesc, Call);
  if (State->get<LastSetPrivilegeCall>() != Setgid && IsSetuidWithGetuid) {
    SymbolRef RetSym = Call.getReturnValue().getAsSymbol();
    State = State->set<LastSetPrivilegeCall>(Setuid);
    State = State->set<LastSetuidCallSVal>(RetSym);
    const NoteTag *Note = C.getNoteTag([this,
                                        RetSym](PathSensitiveBugReport &BR) {
      if (!BR.isInteresting(RetSym) || &BR.getBugType() != &this->BT)
        return "";
      return "Call to 'setuid' found here that removes superuser privileges";
    });
    C.addTransition(State, Note);
    return;
  }
  State = State->set<LastSetPrivilegeCall>(Irrelevant);
  State = State->set<LastSetuidCallSVal>(SymbolRef{});
  C.addTransition(State);
}

void SetgidSetuidOrderChecker::processSetgid(ProgramStateRef State,
                                             const CallEvent &Call,
                                             CheckerContext &C) const {
  bool IsSetgidWithGetgid = isFunctionCalledInArg(GetgidDesc, Call);
  if (State->get<LastSetPrivilegeCall>() == Setuid) {
    if (IsSetgidWithGetgid) {
      State = State->set<LastSetPrivilegeCall>(Irrelevant);
      emitReport(State, C);
      return;
    }
    State = State->set<LastSetPrivilegeCall>(Irrelevant);
  } else {
    State = State->set<LastSetPrivilegeCall>(IsSetgidWithGetgid ? Setgid
                                                                : Irrelevant);
  }
  State = State->set<LastSetuidCallSVal>(SymbolRef{});
  C.addTransition(State);
}

void SetgidSetuidOrderChecker::processOther(ProgramStateRef State,
                                            const CallEvent &Call,
                                            CheckerContext &C) const {
  State = State->set<LastSetuidCallSVal>(SymbolRef{});
  State = State->set<LastSetPrivilegeCall>(Irrelevant);
  C.addTransition(State);
}

bool SetgidSetuidOrderChecker::isFunctionCalledInArg(
    const CallDescription &Desc, const CallEvent &Call) const {
  if (const auto *CallInArg0 =
          dyn_cast<CallExpr>(Call.getArgExpr(0)->IgnoreParenImpCasts()))
    return Desc.matchesAsWritten(*CallInArg0);
  return false;
}

void SetgidSetuidOrderChecker::emitReport(ProgramStateRef State,
                                          CheckerContext &C) const {
  if (ExplodedNode *N = C.generateNonFatalErrorNode(State)) {
    toolchain::StringLiteral Msg =
        "A 'setgid(getgid())' call following a 'setuid(getuid())' "
        "call is likely to fail; probably the order of these "
        "statements is wrong";
    auto Report = std::make_unique<PathSensitiveBugReport>(BT, Msg, N);
    Report->markInteresting(State->get<LastSetuidCallSVal>());
    C.emitReport(std::move(Report));
  }
}

void ento::registerSetgidSetuidOrderChecker(CheckerManager &mgr) {
  mgr.registerChecker<SetgidSetuidOrderChecker>();
}

bool ento::shouldRegisterSetgidSetuidOrderChecker(const CheckerManager &mgr) {
  return true;
}
