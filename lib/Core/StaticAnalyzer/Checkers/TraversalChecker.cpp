//== TraversalChecker.cpp -------------------------------------- -*- C++ -*--=//
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
// These checkers print various aspects of the ExprEngine's traversal of the CFG
// as it builds the ExplodedGraph.
//
//===----------------------------------------------------------------------===//
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/ParentMap.h"
#include "language/Core/AST/StmtObjC.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace ento;

namespace {
// TODO: This checker is only referenced from two small test files and it
// doesn't seem to be useful for manual debugging, so consider reimplementing
// those tests with more modern tools and removing this checker.
class TraversalDumper
    : public Checker<check::BeginFunction, check::EndFunction> {
public:
  void checkBeginFunction(CheckerContext &C) const;
  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
};
}

void TraversalDumper::checkBeginFunction(CheckerContext &C) const {
  toolchain::outs() << "--BEGIN FUNCTION--\n";
}

void TraversalDumper::checkEndFunction(const ReturnStmt *RS,
                                       CheckerContext &C) const {
  toolchain::outs() << "--END FUNCTION--\n";
}

void ento::registerTraversalDumper(CheckerManager &mgr) {
  mgr.registerChecker<TraversalDumper>();
}

bool ento::shouldRegisterTraversalDumper(const CheckerManager &mgr) {
  return true;
}

//------------------------------------------------------------------------------

namespace {
// TODO: This checker appears to be a utility for creating `FileCheck` tests
// verifying its stdout output, but there are no tests that rely on it, so
// perhaps it should be removed.
class CallDumper : public Checker< check::PreCall,
                                   check::PostCall > {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
};
}

void CallDumper::checkPreCall(const CallEvent &Call, CheckerContext &C) const {
  unsigned Indentation = 0;
  for (const LocationContext *LC = C.getLocationContext()->getParent();
       LC != nullptr; LC = LC->getParent())
    ++Indentation;

  // It is mildly evil to print directly to toolchain::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  toolchain::outs().indent(Indentation);
  Call.dump(toolchain::outs());
}

void CallDumper::checkPostCall(const CallEvent &Call, CheckerContext &C) const {
  const Expr *CallE = Call.getOriginExpr();
  if (!CallE)
    return;

  unsigned Indentation = 0;
  for (const LocationContext *LC = C.getLocationContext()->getParent();
       LC != nullptr; LC = LC->getParent())
    ++Indentation;

  // It is mildly evil to print directly to toolchain::outs() rather than emitting
  // warnings, but this ensures things do not get filtered out by the rest of
  // the static analyzer machinery.
  toolchain::outs().indent(Indentation);
  if (Call.getResultType()->isVoidType())
    toolchain::outs() << "Returning void\n";
  else
    toolchain::outs() << "Returning " << C.getSVal(CallE) << "\n";
}

void ento::registerCallDumper(CheckerManager &mgr) {
  mgr.registerChecker<CallDumper>();
}

bool ento::shouldRegisterCallDumper(const CheckerManager &mgr) {
  return true;
}
