//== TrustReturnsNonnullChecker.cpp -- API nullability modeling -*- C++ -*--==//
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
// This checker adds nullability-related assumptions to methods annotated with
// returns_nonnull attribute.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/Attr.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {

class TrustReturnsNonnullChecker : public Checker<check::PostCall> {

public:
  TrustReturnsNonnullChecker(ASTContext &Ctx) {}

  void checkPostCall(const CallEvent &Call, CheckerContext &C) const {
    ProgramStateRef State = C.getState();

    if (isNonNullPtr(Call))
      if (auto L = Call.getReturnValue().getAs<Loc>())
        State = State->assume(*L, /*assumption=*/true);

    C.addTransition(State);
  }

private:
  /// \returns Whether the method declaration has the attribute returns_nonnull.
  bool isNonNullPtr(const CallEvent &Call) const {
    QualType ExprRetType = Call.getResultType();
    const Decl *CallDeclaration =  Call.getDecl();
    if (!ExprRetType->isAnyPointerType() || !CallDeclaration)
      return false;

    return CallDeclaration->hasAttr<ReturnsNonNullAttr>();
  }
};

} // namespace

void ento::registerTrustReturnsNonnullChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<TrustReturnsNonnullChecker>(Mgr.getASTContext());
}

bool ento::shouldRegisterTrustReturnsNonnullChecker(const CheckerManager &mgr) {
  return true;
}
