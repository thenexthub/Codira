//=- RunLoopAutoreleaseLeakChecker.cpp --------------------------*- C++ -*-==//
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
//
//===----------------------------------------------------------------------===//
//
// A checker for detecting leaks resulting from allocating temporary
// autoreleased objects before starting the main run loop.
//
// Checks for two antipatterns:
// 1. ObjCMessageExpr followed by [[NSRunLoop mainRunLoop] run] in the same
// autorelease pool.
// 2. ObjCMessageExpr followed by [[NSRunLoop mainRunLoop] run] in no
// autorelease pool.
//
// Any temporary objects autoreleased in code called in those expressions
// will not be deallocated until the program exits, and are effectively leaks.
//
//===----------------------------------------------------------------------===//
//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace language::Core;
using namespace ento;
using namespace ast_matchers;

namespace {

const char * RunLoopBind = "NSRunLoopM";
const char * RunLoopRunBind = "RunLoopRunM";
const char * OtherMsgBind = "OtherMessageSentM";
const char * AutoreleasePoolBind = "AutoreleasePoolM";
const char * OtherStmtAutoreleasePoolBind = "OtherAutoreleasePoolM";

class RunLoopAutoreleaseLeakChecker : public Checker<check::ASTCodeBody> {

public:
  void checkASTCodeBody(const Decl *D,
                        AnalysisManager &AM,
                        BugReporter &BR) const;

};

} // end anonymous namespace

/// \return Whether @c A occurs before @c B in traversal of
/// @c Parent.
/// Conceptually a very incomplete/unsound approximation of happens-before
/// relationship (A is likely to be evaluated before B),
/// but useful enough in this case.
static bool seenBefore(const Stmt *Parent, const Stmt *A, const Stmt *B) {
  for (const Stmt *C : Parent->children()) {
    if (!C) continue;

    if (C == A)
      return true;

    if (C == B)
      return false;

    return seenBefore(C, A, B);
  }
  return false;
}

static void emitDiagnostics(BoundNodes &Match,
                            const Decl *D,
                            BugReporter &BR,
                            AnalysisManager &AM,
                            const RunLoopAutoreleaseLeakChecker *Checker) {

  assert(D->hasBody());
  const Stmt *DeclBody = D->getBody();

  AnalysisDeclContext *ADC = AM.getAnalysisDeclContext(D);

  const auto *ME = Match.getNodeAs<ObjCMessageExpr>(OtherMsgBind);
  assert(ME);

  const auto *AP =
      Match.getNodeAs<ObjCAutoreleasePoolStmt>(AutoreleasePoolBind);
  const auto *OAP =
      Match.getNodeAs<ObjCAutoreleasePoolStmt>(OtherStmtAutoreleasePoolBind);
  bool HasAutoreleasePool = (AP != nullptr);

  const auto *RL = Match.getNodeAs<ObjCMessageExpr>(RunLoopBind);
  const auto *RLR = Match.getNodeAs<Stmt>(RunLoopRunBind);
  assert(RLR && "Run loop launch not found");
  assert(ME != RLR);

  // Launch of run loop occurs before the message-sent expression is seen.
  if (seenBefore(DeclBody, RLR, ME))
    return;

  if (HasAutoreleasePool && (OAP != AP))
    return;

  PathDiagnosticLocation Location = PathDiagnosticLocation::createBegin(
    ME, BR.getSourceManager(), ADC);
  SourceRange Range = ME->getSourceRange();

  BR.EmitBasicReport(ADC->getDecl(), Checker,
                     /*Name=*/"Memory leak inside autorelease pool",
                     /*BugCategory=*/"Memory",
                     /*Name=*/
                     (Twine("Temporary objects allocated in the") +
                      " autorelease pool " +
                      (HasAutoreleasePool ? "" : "of last resort ") +
                      "followed by the launch of " +
                      (RL ? "main run loop " : "xpc_main ") +
                      "may never get released; consider moving them to a "
                      "separate autorelease pool")
                         .str(),
                     Location, Range);
}

static StatementMatcher getRunLoopRunM(StatementMatcher Extra = anything()) {
  StatementMatcher MainRunLoopM =
      objcMessageExpr(hasSelector("mainRunLoop"),
                      hasReceiverType(asString("NSRunLoop")),
                      Extra)
          .bind(RunLoopBind);

  StatementMatcher MainRunLoopRunM = objcMessageExpr(hasSelector("run"),
                         hasReceiver(MainRunLoopM),
                         Extra).bind(RunLoopRunBind);

  StatementMatcher XPCRunM =
      callExpr(callee(functionDecl(hasName("xpc_main")))).bind(RunLoopRunBind);
  return anyOf(MainRunLoopRunM, XPCRunM);
}

static StatementMatcher getOtherMessageSentM(StatementMatcher Extra = anything()) {
  return objcMessageExpr(unless(anyOf(equalsBoundNode(RunLoopBind),
                                      equalsBoundNode(RunLoopRunBind))),
                         Extra)
      .bind(OtherMsgBind);
}

static void
checkTempObjectsInSamePool(const Decl *D, AnalysisManager &AM, BugReporter &BR,
                           const RunLoopAutoreleaseLeakChecker *Chkr) {
  StatementMatcher RunLoopRunM = getRunLoopRunM();
  StatementMatcher OtherMessageSentM = getOtherMessageSentM(
    hasAncestor(autoreleasePoolStmt().bind(OtherStmtAutoreleasePoolBind)));

  StatementMatcher RunLoopInAutorelease =
      autoreleasePoolStmt(
        hasDescendant(RunLoopRunM),
        hasDescendant(OtherMessageSentM)).bind(AutoreleasePoolBind);

  DeclarationMatcher GroupM = decl(hasDescendant(RunLoopInAutorelease));

  auto Matches = match(GroupM, *D, AM.getASTContext());
  for (BoundNodes Match : Matches)
    emitDiagnostics(Match, D, BR, AM, Chkr);
}

static void
checkTempObjectsInNoPool(const Decl *D, AnalysisManager &AM, BugReporter &BR,
                         const RunLoopAutoreleaseLeakChecker *Chkr) {

  auto NoPoolM = unless(hasAncestor(autoreleasePoolStmt()));

  StatementMatcher RunLoopRunM = getRunLoopRunM(NoPoolM);
  StatementMatcher OtherMessageSentM = getOtherMessageSentM(NoPoolM);

  DeclarationMatcher GroupM = functionDecl(
    isMain(),
    hasDescendant(RunLoopRunM),
    hasDescendant(OtherMessageSentM)
  );

  auto Matches = match(GroupM, *D, AM.getASTContext());

  for (BoundNodes Match : Matches)
    emitDiagnostics(Match, D, BR, AM, Chkr);

}

void RunLoopAutoreleaseLeakChecker::checkASTCodeBody(const Decl *D,
                        AnalysisManager &AM,
                        BugReporter &BR) const {
  checkTempObjectsInSamePool(D, AM, BR, this);
  checkTempObjectsInNoPool(D, AM, BR, this);
}

void ento::registerRunLoopAutoreleaseLeakChecker(CheckerManager &mgr) {
  mgr.registerChecker<RunLoopAutoreleaseLeakChecker>();
}

bool ento::shouldRegisterRunLoopAutoreleaseLeakChecker(const CheckerManager &mgr) {
  return true;
}
