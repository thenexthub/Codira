//=== CastToStructChecker.cpp ----------------------------------*- C++ -*--===//
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
// This files defines CastToStructChecker, a builtin checker that checks for
// cast from non-struct pointer to struct pointer and widening struct data cast.
// This check corresponds to CWE-588.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/DynamicRecursiveASTVisitor.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class CastToStructVisitor : public DynamicRecursiveASTVisitor {
  BugReporter &BR;
  const CheckerBase *Checker;
  AnalysisDeclContext *AC;

public:
  explicit CastToStructVisitor(BugReporter &B, const CheckerBase *Checker,
                               AnalysisDeclContext *A)
      : BR(B), Checker(Checker), AC(A) {}
  bool VisitCastExpr(CastExpr *CE) override;
};
}

bool CastToStructVisitor::VisitCastExpr(CastExpr *CE) {
  const Expr *E = CE->getSubExpr();
  ASTContext &Ctx = AC->getASTContext();
  QualType OrigTy = Ctx.getCanonicalType(E->getType());
  QualType ToTy = Ctx.getCanonicalType(CE->getType());

  const PointerType *OrigPTy = dyn_cast<PointerType>(OrigTy.getTypePtr());
  const PointerType *ToPTy = dyn_cast<PointerType>(ToTy.getTypePtr());

  if (!ToPTy || !OrigPTy)
    return true;

  QualType OrigPointeeTy = OrigPTy->getPointeeType();
  QualType ToPointeeTy = ToPTy->getPointeeType();

  if (!ToPointeeTy->isStructureOrClassType())
    return true;

  // We allow cast from void*.
  if (OrigPointeeTy->isVoidType())
    return true;

  // Now the cast-to-type is struct pointer, the original type is not void*.
  if (!OrigPointeeTy->isRecordType()) {
    SourceRange Sr[1] = {CE->getSourceRange()};
    PathDiagnosticLocation Loc(CE, BR.getSourceManager(), AC);
    BR.EmitBasicReport(
        AC->getDecl(), Checker, "Cast from non-struct type to struct type",
        categories::LogicError, "Casting a non-structure type to a structure "
                                "type and accessing a field can lead to memory "
                                "access errors or data corruption.",
        Loc, Sr);
  } else {
    // Don't warn when size of data is unknown.
    const auto *U = dyn_cast<UnaryOperator>(E);
    if (!U || U->getOpcode() != UO_AddrOf)
      return true;

    // Don't warn for references
    const ValueDecl *VD = nullptr;
    if (const auto *SE = dyn_cast<DeclRefExpr>(U->getSubExpr()))
      VD = SE->getDecl();
    else if (const auto *SE = dyn_cast<MemberExpr>(U->getSubExpr()))
      VD = SE->getMemberDecl();
    if (!VD || VD->getType()->isReferenceType())
      return true;

    if (ToPointeeTy->isIncompleteType() ||
        OrigPointeeTy->isIncompleteType())
      return true;

    // Warn when there is widening cast.
    unsigned ToWidth = Ctx.getTypeInfo(ToPointeeTy).Width;
    unsigned OrigWidth = Ctx.getTypeInfo(OrigPointeeTy).Width;
    if (ToWidth <= OrigWidth)
      return true;

    PathDiagnosticLocation Loc(CE, BR.getSourceManager(), AC);
    BR.EmitBasicReport(AC->getDecl(), Checker, "Widening cast to struct type",
                       categories::LogicError,
                       "Casting data to a larger structure type and accessing "
                       "a field can lead to memory access errors or data "
                       "corruption.",
                       Loc, CE->getSourceRange());
  }

  return true;
}

namespace {
class CastToStructChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager &Mgr,
                        BugReporter &BR) const {
    CastToStructVisitor Visitor(BR, this, Mgr.getAnalysisDeclContext(D));
    Visitor.TraverseDecl(const_cast<Decl *>(D));
  }
};
} // end anonymous namespace

void ento::registerCastToStructChecker(CheckerManager &mgr) {
  mgr.registerChecker<CastToStructChecker>();
}

bool ento::shouldRegisterCastToStructChecker(const CheckerManager &mgr) {
  return true;
}
