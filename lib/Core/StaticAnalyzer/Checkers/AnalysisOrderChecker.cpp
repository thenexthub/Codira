//===- AnalysisOrderChecker - Print callbacks called ------------*- C++ -*-===//
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
// This checker prints callbacks that are called during analysis.
// This is required to ensure that callbacks are fired in order
// and do not duplicate or get lost.
// Feel free to extend this checker with any callback you need to check.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ExprCXX.h"
#include "language/Core/Analysis/CFGStmtMap.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {

class AnalysisOrderChecker
    : public Checker<
          check::PreStmt<CastExpr>, check::PostStmt<CastExpr>,
          check::PreStmt<ArraySubscriptExpr>,
          check::PostStmt<ArraySubscriptExpr>, check::PreStmt<CXXNewExpr>,
          check::PostStmt<CXXNewExpr>, check::PreStmt<CXXDeleteExpr>,
          check::PostStmt<CXXDeleteExpr>, check::PreStmt<CXXConstructExpr>,
          check::PostStmt<CXXConstructExpr>, check::PreStmt<OffsetOfExpr>,
          check::PostStmt<OffsetOfExpr>, check::PreCall, check::PostCall,
          check::EndFunction, check::EndAnalysis, check::NewAllocator,
          check::Bind, check::PointerEscape, check::RegionChanges,
          check::LiveSymbols, eval::Call> {

  bool isCallbackEnabled(const AnalyzerOptions &Opts,
                         StringRef CallbackName) const {
    return Opts.getCheckerBooleanOption(this, "*") ||
           Opts.getCheckerBooleanOption(this, CallbackName);
  }

  bool isCallbackEnabled(CheckerContext &C, StringRef CallbackName) const {
    AnalyzerOptions &Opts = C.getAnalysisManager().getAnalyzerOptions();
    return isCallbackEnabled(Opts, CallbackName);
  }

  bool isCallbackEnabled(ProgramStateRef State, StringRef CallbackName) const {
    AnalyzerOptions &Opts = State->getStateManager().getOwningEngine()
                                 .getAnalysisManager().getAnalyzerOptions();
    return isCallbackEnabled(Opts, CallbackName);
  }

public:
  void checkPreStmt(const CastExpr *CE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCastExpr"))
      toolchain::errs() << "PreStmt<CastExpr> (Kind : " << CE->getCastKindName()
                   << ")\n";
  }

  void checkPostStmt(const CastExpr *CE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCastExpr"))
      toolchain::errs() << "PostStmt<CastExpr> (Kind : " << CE->getCastKindName()
                   << ")\n";
  }

  void checkPreStmt(const ArraySubscriptExpr *SubExpr,
                    CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtArraySubscriptExpr"))
      toolchain::errs() << "PreStmt<ArraySubscriptExpr>\n";
  }

  void checkPostStmt(const ArraySubscriptExpr *SubExpr,
                     CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtArraySubscriptExpr"))
      toolchain::errs() << "PostStmt<ArraySubscriptExpr>\n";
  }

  void checkPreStmt(const CXXNewExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCXXNewExpr"))
      toolchain::errs() << "PreStmt<CXXNewExpr>\n";
  }

  void checkPostStmt(const CXXNewExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCXXNewExpr"))
      toolchain::errs() << "PostStmt<CXXNewExpr>\n";
  }

  void checkPreStmt(const CXXDeleteExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCXXDeleteExpr"))
      toolchain::errs() << "PreStmt<CXXDeleteExpr>\n";
  }

  void checkPostStmt(const CXXDeleteExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCXXDeleteExpr"))
      toolchain::errs() << "PostStmt<CXXDeleteExpr>\n";
  }

  void checkPreStmt(const CXXConstructExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtCXXConstructExpr"))
      toolchain::errs() << "PreStmt<CXXConstructExpr>\n";
  }

  void checkPostStmt(const CXXConstructExpr *NE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtCXXConstructExpr"))
      toolchain::errs() << "PostStmt<CXXConstructExpr>\n";
  }

  void checkPreStmt(const OffsetOfExpr *OOE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreStmtOffsetOfExpr"))
      toolchain::errs() << "PreStmt<OffsetOfExpr>\n";
  }

  void checkPostStmt(const OffsetOfExpr *OOE, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostStmtOffsetOfExpr"))
      toolchain::errs() << "PostStmt<OffsetOfExpr>\n";
  }

  bool evalCall(const CallEvent &Call, CheckerContext &C) const {
    if (isCallbackEnabled(C, "EvalCall")) {
      toolchain::errs() << "EvalCall";
      if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Call.getDecl()))
        toolchain::errs() << " (" << ND->getQualifiedNameAsString() << ')';
      toolchain::errs() << " {argno: " << Call.getNumArgs() << '}';
      toolchain::errs() << " [" << Call.getKindAsString() << ']';
      toolchain::errs() << '\n';
      return true;
    }
    return false;
  }

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PreCall")) {
      toolchain::errs() << "PreCall";
      if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Call.getDecl()))
        toolchain::errs() << " (" << ND->getQualifiedNameAsString() << ')';
      toolchain::errs() << " [" << Call.getKindAsString() << ']';
      toolchain::errs() << '\n';
    }
  }

  void checkPostCall(const CallEvent &Call, CheckerContext &C) const {
    if (isCallbackEnabled(C, "PostCall")) {
      toolchain::errs() << "PostCall";
      if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(Call.getDecl()))
        toolchain::errs() << " (" << ND->getQualifiedNameAsString() << ')';
      toolchain::errs() << " [" << Call.getKindAsString() << ']';
      toolchain::errs() << '\n';
    }
  }

  void checkEndFunction(const ReturnStmt *S, CheckerContext &C) const {
    if (isCallbackEnabled(C, "EndFunction")) {
      toolchain::errs() << "EndFunction\nReturnStmt: " << (S ? "yes" : "no") << "\n";
      if (!S)
        return;

      toolchain::errs() << "CFGElement: ";
      CFGStmtMap *Map = C.getCurrentAnalysisDeclContext()->getCFGStmtMap();
      CFGElement LastElement = Map->getBlock(S)->back();

      if (LastElement.getAs<CFGStmt>())
        toolchain::errs() << "CFGStmt\n";
      else if (LastElement.getAs<CFGAutomaticObjDtor>())
        toolchain::errs() << "CFGAutomaticObjDtor\n";
    }
  }

  void checkEndAnalysis(ExplodedGraph &G, BugReporter &BR,
                        ExprEngine &Eng) const {
    if (isCallbackEnabled(BR.getAnalyzerOptions(), "EndAnalysis"))
      toolchain::errs() << "EndAnalysis\n";
  }

  void checkNewAllocator(const CXXAllocatorCall &Call,
                         CheckerContext &C) const {
    if (isCallbackEnabled(C, "NewAllocator"))
      toolchain::errs() << "NewAllocator\n";
  }

  void checkBind(SVal Loc, SVal Val, const Stmt *S, bool AtDeclInit,
                 CheckerContext &C) const {
    if (isCallbackEnabled(C, "Bind"))
      toolchain::errs() << "Bind\n";
  }

  void checkLiveSymbols(ProgramStateRef State, SymbolReaper &SymReaper) const {
    if (isCallbackEnabled(State, "LiveSymbols"))
      toolchain::errs() << "LiveSymbols\n";
  }

  ProgramStateRef
  checkRegionChanges(ProgramStateRef State,
                     const InvalidatedSymbols *Invalidated,
                     ArrayRef<const MemRegion *> ExplicitRegions,
                     ArrayRef<const MemRegion *> Regions,
                     const LocationContext *LCtx, const CallEvent *Call) const {
    if (isCallbackEnabled(State, "RegionChanges"))
      toolchain::errs() << "RegionChanges\n";
    return State;
  }

  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols &Escaped,
                                     const CallEvent *Call,
                                     PointerEscapeKind Kind) const {
    if (isCallbackEnabled(State, "PointerEscape"))
      toolchain::errs() << "PointerEscape\n";
    return State;
  }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Registration.
//===----------------------------------------------------------------------===//

void ento::registerAnalysisOrderChecker(CheckerManager &mgr) {
  mgr.registerChecker<AnalysisOrderChecker>();
}

bool ento::shouldRegisterAnalysisOrderChecker(const CheckerManager &mgr) {
  return true;
}
