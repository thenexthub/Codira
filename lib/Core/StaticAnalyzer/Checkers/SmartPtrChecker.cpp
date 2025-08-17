// SmartPtrChecker.cpp - Check for smart pointer dereference - C++ --------===//
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
// This file defines a checker that check for null dereference of C++ smart
// pointer.
//
//===----------------------------------------------------------------------===//
#include "SmartPtr.h"

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"

using namespace language::Core;
using namespace ento;

namespace {

static const BugType *NullDereferenceBugTypePtr;

class SmartPtrChecker : public Checker<check::PreCall> {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  BugType NullDereferenceBugType{this, "Null SmartPtr dereference",
                                 "C++ Smart Pointer"};

private:
  void reportBug(CheckerContext &C, const MemRegion *DerefRegion,
                 const CallEvent &Call) const;
  void explainDereference(toolchain::raw_ostream &OS, const MemRegion *DerefRegion,
                          const CallEvent &Call) const;
};
} // end of anonymous namespace

// Define the inter-checker API.
namespace language::Core {
namespace ento {
namespace smartptr {

const BugType *getNullDereferenceBugType() { return NullDereferenceBugTypePtr; }

} // namespace smartptr
} // namespace ento
} // namespace language::Core

void SmartPtrChecker::checkPreCall(const CallEvent &Call,
                                   CheckerContext &C) const {
  if (!smartptr::isStdSmartPtrCall(Call))
    return;
  ProgramStateRef State = C.getState();
  const auto *OC = dyn_cast<CXXMemberOperatorCall>(&Call);
  if (!OC)
    return;
  const MemRegion *ThisRegion = OC->getCXXThisVal().getAsRegion();
  if (!ThisRegion)
    return;

  OverloadedOperatorKind OOK = OC->getOverloadedOperator();
  if (OOK == OO_Star || OOK == OO_Arrow) {
    if (smartptr::isNullSmartPtr(State, ThisRegion))
      reportBug(C, ThisRegion, Call);
  }
}

void SmartPtrChecker::reportBug(CheckerContext &C, const MemRegion *DerefRegion,
                                const CallEvent &Call) const {
  ExplodedNode *ErrNode = C.generateErrorNode();
  if (!ErrNode)
    return;
  toolchain::SmallString<128> Str;
  toolchain::raw_svector_ostream OS(Str);
  explainDereference(OS, DerefRegion, Call);
  auto R = std::make_unique<PathSensitiveBugReport>(NullDereferenceBugType,
                                                    OS.str(), ErrNode);
  R->markInteresting(DerefRegion);
  C.emitReport(std::move(R));
}

void SmartPtrChecker::explainDereference(toolchain::raw_ostream &OS,
                                         const MemRegion *DerefRegion,
                                         const CallEvent &Call) const {
  OS << "Dereference of null smart pointer ";
  DerefRegion->printPretty(OS);
}

void ento::registerSmartPtrChecker(CheckerManager &Mgr) {
  SmartPtrChecker *Checker = Mgr.registerChecker<SmartPtrChecker>();
  NullDereferenceBugTypePtr = &Checker->NullDereferenceBugType;
}

bool ento::shouldRegisterSmartPtrChecker(const CheckerManager &mgr) {
  const LangOptions &LO = mgr.getLangOpts();
  return LO.CPlusPlus;
}
