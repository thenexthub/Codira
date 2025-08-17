//===-- DebugIteratorModeling.cpp ---------------------------------*- C++ -*--//
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
// Defines a checker for debugging iterator modeling.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

#include "Iterator.h"

using namespace language::Core;
using namespace ento;
using namespace iterator;

namespace {

class DebugIteratorModeling
  : public Checker<eval::Call> {

  const BugType DebugMsgBugType{this, "Checking analyzer assumptions", "debug",
                                /*SuppressOnSink=*/true};

  template <typename Getter>
  void analyzerIteratorDataField(const CallExpr *CE, CheckerContext &C,
                                 Getter get, SVal Default) const;
  void analyzerIteratorPosition(const CallExpr *CE, CheckerContext &C) const;
  void analyzerIteratorContainer(const CallExpr *CE, CheckerContext &C) const;
  void analyzerIteratorValidity(const CallExpr *CE, CheckerContext &C) const;
  ExplodedNode *reportDebugMsg(toolchain::StringRef Msg, CheckerContext &C) const;

  typedef void (DebugIteratorModeling::*FnCheck)(const CallExpr *,
                                                 CheckerContext &) const;

  CallDescriptionMap<FnCheck> Callbacks = {
      {{CDM::SimpleFunc, {"clang_analyzer_iterator_position"}, 1},
       &DebugIteratorModeling::analyzerIteratorPosition},
      {{CDM::SimpleFunc, {"clang_analyzer_iterator_container"}, 1},
       &DebugIteratorModeling::analyzerIteratorContainer},
      {{CDM::SimpleFunc, {"clang_analyzer_iterator_validity"}, 1},
       &DebugIteratorModeling::analyzerIteratorValidity},
  };

public:
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};

} // namespace

bool DebugIteratorModeling::evalCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  const auto *CE = dyn_cast_or_null<CallExpr>(Call.getOriginExpr());
  if (!CE)
    return false;

  const FnCheck *Handler = Callbacks.lookup(Call);
  if (!Handler)
    return false;

  (this->**Handler)(CE, C);
  return true;
}

template <typename Getter>
void DebugIteratorModeling::analyzerIteratorDataField(const CallExpr *CE,
                                                      CheckerContext &C,
                                                      Getter get,
                                                      SVal Default) const {
  if (CE->getNumArgs() == 0) {
    reportDebugMsg("Missing iterator argument", C);
    return;
  }

  auto State = C.getState();
  SVal V = C.getSVal(CE->getArg(0));
  const auto *Pos = getIteratorPosition(State, V);
  if (Pos) {
    State = State->BindExpr(CE, C.getLocationContext(), get(Pos));
  } else {
    State = State->BindExpr(CE, C.getLocationContext(), Default);
  }
  C.addTransition(State);
}

void DebugIteratorModeling::analyzerIteratorPosition(const CallExpr *CE,
                                                     CheckerContext &C) const {
  auto &BVF = C.getSValBuilder().getBasicValueFactory();
  analyzerIteratorDataField(CE, C, [](const IteratorPosition *P) {
      return nonloc::SymbolVal(P->getOffset());
    }, nonloc::ConcreteInt(BVF.getValue(toolchain::APSInt::get(0))));
}

void DebugIteratorModeling::analyzerIteratorContainer(const CallExpr *CE,
                                                      CheckerContext &C) const {
  auto &BVF = C.getSValBuilder().getBasicValueFactory();
  analyzerIteratorDataField(CE, C, [](const IteratorPosition *P) {
      return loc::MemRegionVal(P->getContainer());
    }, loc::ConcreteInt(BVF.getValue(toolchain::APSInt::get(0))));
}

void DebugIteratorModeling::analyzerIteratorValidity(const CallExpr *CE,
                                                     CheckerContext &C) const {
  auto &BVF = C.getSValBuilder().getBasicValueFactory();
  analyzerIteratorDataField(CE, C, [&BVF](const IteratorPosition *P) {
      return
        nonloc::ConcreteInt(BVF.getValue(toolchain::APSInt::get((P->isValid()))));
    }, nonloc::ConcreteInt(BVF.getValue(toolchain::APSInt::get(0))));
}

ExplodedNode *DebugIteratorModeling::reportDebugMsg(toolchain::StringRef Msg,
                                                    CheckerContext &C) const {
  ExplodedNode *N = C.generateNonFatalErrorNode();
  if (!N)
    return nullptr;

  auto &BR = C.getBugReporter();
  BR.emitReport(
      std::make_unique<PathSensitiveBugReport>(DebugMsgBugType, Msg, N));
  return N;
}

void ento::registerDebugIteratorModeling(CheckerManager &mgr) {
  mgr.registerChecker<DebugIteratorModeling>();
}

bool ento::shouldRegisterDebugIteratorModeling(const CheckerManager &mgr) {
  return true;
}
