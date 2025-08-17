//===- OSObjectCStyleCast.cpp ------------------------------------*- C++ -*-==//
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
// This file defines OSObjectCStyleCast checker, which checks for C-style casts
// of OSObjects. Such casts almost always indicate a code smell,
// as an explicit static or dynamic cast should be used instead.
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "toolchain/Support/Debug.h"

using namespace language::Core;
using namespace ento;
using namespace ast_matchers;

namespace {
static constexpr const char *const WarnAtNode = "WarnAtNode";
static constexpr const char *const WarnRecordDecl = "WarnRecordDecl";

class OSObjectCStyleCastChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager &AM,
                        BugReporter &BR) const;
};
} // namespace

namespace language::Core {
namespace ast_matchers {
AST_MATCHER_P(StringLiteral, mentionsBoundType, std::string, BindingID) {
  return Builder->removeBindings([this, &Node](const BoundNodesMap &Nodes) {
    const DynTypedNode &BN = Nodes.getNode(this->BindingID);
    if (const auto *ND = BN.get<NamedDecl>()) {
      return ND->getName() != Node.getString();
    }
    return true;
  });
}
} // end namespace ast_matchers
} // end namespace language::Core

static void emitDiagnostics(const BoundNodes &Nodes,
                            BugReporter &BR,
                            AnalysisDeclContext *ADC,
                            const OSObjectCStyleCastChecker *Checker) {
  const auto *CE = Nodes.getNodeAs<CastExpr>(WarnAtNode);
  const CXXRecordDecl *RD = Nodes.getNodeAs<CXXRecordDecl>(WarnRecordDecl);
  assert(CE && RD);

  std::string Diagnostics;
  toolchain::raw_string_ostream OS(Diagnostics);
  OS << "C-style cast of an OSObject is prone to type confusion attacks; "
     << "use 'OSRequiredCast' if the object is definitely of type '"
     << RD->getNameAsString() << "', or 'OSDynamicCast' followed by "
     << "a null check if unsure",

  BR.EmitBasicReport(
    ADC->getDecl(),
    Checker,
    /*Name=*/"OSObject C-Style Cast",
    categories::SecurityError,
    Diagnostics,
    PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(), ADC),
    CE->getSourceRange());
}

static decltype(auto) hasTypePointingTo(DeclarationMatcher DeclM) {
  return hasType(pointerType(pointee(hasDeclaration(DeclM))));
}

void OSObjectCStyleCastChecker::checkASTCodeBody(const Decl *D,
                                                 AnalysisManager &AM,
                                                 BugReporter &BR) const {

  AnalysisDeclContext *ADC = AM.getAnalysisDeclContext(D);

  auto DynamicCastM = callExpr(callee(functionDecl(hasName("safeMetaCast"))));
  // 'allocClassWithName' allocates an object with the given type.
  // The type is actually provided as a string argument (type's name).
  // This makes the following pattern possible:
  //
  // Foo *object = (Foo *)allocClassWithName("Foo");
  //
  // While OSRequiredCast can be used here, it is still not a useful warning.
  auto AllocClassWithNameM = callExpr(
      callee(functionDecl(hasName("allocClassWithName"))),
      // Here we want to make sure that the string argument matches the
      // type in the cast expression.
      hasArgument(0, stringLiteral(mentionsBoundType(WarnRecordDecl))));

  auto OSObjTypeM =
      hasTypePointingTo(cxxRecordDecl(isDerivedFrom("OSMetaClassBase")));
  auto OSObjSubclassM = hasTypePointingTo(
      cxxRecordDecl(isDerivedFrom("OSObject")).bind(WarnRecordDecl));

  auto CastM =
      cStyleCastExpr(
          allOf(OSObjSubclassM,
                hasSourceExpression(
                    allOf(OSObjTypeM,
                          unless(anyOf(DynamicCastM, AllocClassWithNameM))))))
          .bind(WarnAtNode);

  auto Matches =
      match(stmt(forEachDescendant(CastM)), *D->getBody(), AM.getASTContext());
  for (BoundNodes Match : Matches)
    emitDiagnostics(Match, BR, ADC, this);
}

void ento::registerOSObjectCStyleCast(CheckerManager &Mgr) {
  Mgr.registerChecker<OSObjectCStyleCastChecker>();
}

bool ento::shouldRegisterOSObjectCStyleCast(const CheckerManager &mgr) {
  return true;
}
