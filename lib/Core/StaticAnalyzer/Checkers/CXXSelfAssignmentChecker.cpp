//=== CXXSelfAssignmentChecker.cpp -----------------------------*- C++ -*--===//
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
// This file defines CXXSelfAssignmentChecker, which tests all custom defined
// copy and move assignment operators for the case of self assignment, thus
// where the parameter refers to the same location where the this pointer
// points to. The checker itself does not do any checks at all, but it
// causes the analyzer to check every copy and move assignment operator twice:
// once for when 'this' aliases with the parameter and once for when it may not.
// It is the task of the other enabled checkers to find the bugs in these two
// different cases.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {

class CXXSelfAssignmentChecker : public Checker<check::BeginFunction> {
public:
  CXXSelfAssignmentChecker();
  void checkBeginFunction(CheckerContext &C) const;
};
}

CXXSelfAssignmentChecker::CXXSelfAssignmentChecker() {}

void CXXSelfAssignmentChecker::checkBeginFunction(CheckerContext &C) const {
  if (!C.inTopFrame())
    return;
  const auto *LCtx = C.getLocationContext();
  const auto *MD = dyn_cast<CXXMethodDecl>(LCtx->getDecl());
  if (!MD)
    return;
  if (!MD->isCopyAssignmentOperator() && !MD->isMoveAssignmentOperator())
    return;
  auto &State = C.getState();
  auto &SVB = C.getSValBuilder();
  auto ThisVal =
      State->getSVal(SVB.getCXXThis(MD, LCtx->getStackFrame()));
  auto Param = SVB.makeLoc(State->getRegion(MD->getParamDecl(0), LCtx));
  auto ParamVal = State->getSVal(Param);

  ProgramStateRef SelfAssignState = State->bindLoc(Param, ThisVal, LCtx);
  const NoteTag *SelfAssignTag =
    C.getNoteTag([MD](PathSensitiveBugReport &BR) -> std::string {
        SmallString<256> Msg;
        toolchain::raw_svector_ostream Out(Msg);
        Out << "Assuming " << MD->getParamDecl(0)->getName() << " == *this";
        return std::string(Out.str());
      });
  C.addTransition(SelfAssignState, SelfAssignTag);

  ProgramStateRef NonSelfAssignState = State->bindLoc(Param, ParamVal, LCtx);
  const NoteTag *NonSelfAssignTag =
    C.getNoteTag([MD](PathSensitiveBugReport &BR) -> std::string {
        SmallString<256> Msg;
        toolchain::raw_svector_ostream Out(Msg);
        Out << "Assuming " << MD->getParamDecl(0)->getName() << " != *this";
        return std::string(Out.str());
      });
  C.addTransition(NonSelfAssignState, NonSelfAssignTag);
}

void ento::registerCXXSelfAssignmentChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<CXXSelfAssignmentChecker>();
}

bool ento::shouldRegisterCXXSelfAssignmentChecker(const CheckerManager &mgr) {
  return true;
}
