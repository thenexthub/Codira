//=- NSAutoreleasePoolChecker.cpp --------------------------------*- C++ -*-==//
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
//  This file defines a NSAutoreleasePoolChecker, a small checker that warns
//  about subpar uses of NSAutoreleasePool.  Note that while the check itself
//  (in its current form) could be written as a flow-insensitive check, in
//  can be potentially enhanced in the future with flow-sensitive information.
//  It is also a good example of the CheckerVisitor interface.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace language::Core;
using namespace ento;

namespace {
class NSAutoreleasePoolChecker
  : public Checker<check::PreObjCMessage> {
  const BugType BT{this, "Use -drain instead of -release",
                   "API Upgrade (Apple)"};
  mutable Selector releaseS;

public:
  void checkPreObjCMessage(const ObjCMethodCall &msg, CheckerContext &C) const;
};

} // end anonymous namespace

void NSAutoreleasePoolChecker::checkPreObjCMessage(const ObjCMethodCall &msg,
                                                   CheckerContext &C) const {
  if (!msg.isInstanceMessage())
    return;

  const ObjCInterfaceDecl *OD = msg.getReceiverInterface();
  if (!OD)
    return;
  if (!OD->getIdentifier()->isStr("NSAutoreleasePool"))
    return;

  if (releaseS.isNull())
    releaseS = GetNullarySelector("release", C.getASTContext());
  // Sending 'release' message?
  if (msg.getSelector() != releaseS)
    return;

  ExplodedNode *N = C.generateNonFatalErrorNode();
  if (!N) {
    assert(0);
    return;
  }

  auto Report = std::make_unique<PathSensitiveBugReport>(
      BT,
      "Use -drain instead of -release when using NSAutoreleasePool and "
      "garbage collection",
      N);
  Report->addRange(msg.getSourceRange());
  C.emitReport(std::move(Report));
}

void ento::registerNSAutoreleasePoolChecker(CheckerManager &mgr) {
  mgr.registerChecker<NSAutoreleasePoolChecker>();
}

bool ento::shouldRegisterNSAutoreleasePoolChecker(const CheckerManager &mgr) {
  const LangOptions &LO = mgr.getLangOpts();
  return LO.getGC() != LangOptions::NonGC;
}
