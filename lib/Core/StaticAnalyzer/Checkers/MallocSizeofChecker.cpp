// MallocSizeofChecker.cpp - Check for dubious malloc arguments ---*- C++ -*-=//
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
// Reports inconsistencies between the casted type of the return value of a
// malloc/calloc/realloc call and the operand of any sizeof expressions
// contained within its argument(s).
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtVisitor.h"
#include "language/Core/AST/TypeLoc.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace ento;

namespace {

typedef std::pair<const TypeSourceInfo *, const CallExpr *> TypeCallPair;
typedef toolchain::PointerUnion<const Stmt *, const VarDecl *> ExprParent;

class CastedAllocFinder
  : public ConstStmtVisitor<CastedAllocFinder, TypeCallPair> {
  IdentifierInfo *II_malloc, *II_calloc, *II_realloc;

public:
  struct CallRecord {
    ExprParent CastedExprParent;
    const Expr *CastedExpr;
    const TypeSourceInfo *ExplicitCastType;
    const CallExpr *AllocCall;

    CallRecord(ExprParent CastedExprParent, const Expr *CastedExpr,
               const TypeSourceInfo *ExplicitCastType,
               const CallExpr *AllocCall)
      : CastedExprParent(CastedExprParent), CastedExpr(CastedExpr),
        ExplicitCastType(ExplicitCastType), AllocCall(AllocCall) {}
  };

  typedef std::vector<CallRecord> CallVec;
  CallVec Calls;

  CastedAllocFinder(ASTContext *Ctx) :
    II_malloc(&Ctx->Idents.get("malloc")),
    II_calloc(&Ctx->Idents.get("calloc")),
    II_realloc(&Ctx->Idents.get("realloc")) {}

  void VisitChild(ExprParent Parent, const Stmt *S) {
    TypeCallPair AllocCall = Visit(S);
    if (AllocCall.second && AllocCall.second != S)
      Calls.push_back(CallRecord(Parent, cast<Expr>(S), AllocCall.first,
                                 AllocCall.second));
  }

  void VisitChildren(const Stmt *S) {
    for (const Stmt *Child : S->children())
      if (Child)
        VisitChild(S, Child);
  }

  TypeCallPair VisitCastExpr(const CastExpr *E) {
    return Visit(E->getSubExpr());
  }

  TypeCallPair VisitExplicitCastExpr(const ExplicitCastExpr *E) {
    return TypeCallPair(E->getTypeInfoAsWritten(),
                        Visit(E->getSubExpr()).second);
  }

  TypeCallPair VisitParenExpr(const ParenExpr *E) {
    return Visit(E->getSubExpr());
  }

  TypeCallPair VisitStmt(const Stmt *S) {
    VisitChildren(S);
    return TypeCallPair();
  }

  TypeCallPair VisitCallExpr(const CallExpr *E) {
    VisitChildren(E);
    const FunctionDecl *FD = E->getDirectCallee();
    if (FD) {
      IdentifierInfo *II = FD->getIdentifier();
      if (II == II_malloc || II == II_calloc || II == II_realloc)
        return TypeCallPair((const TypeSourceInfo *)nullptr, E);
    }
    return TypeCallPair();
  }

  TypeCallPair VisitDeclStmt(const DeclStmt *S) {
    for (const auto *I : S->decls())
      if (const VarDecl *VD = dyn_cast<VarDecl>(I))
        if (const Expr *Init = VD->getInit())
          VisitChild(VD, Init);
    return TypeCallPair();
  }
};

class SizeofFinder : public ConstStmtVisitor<SizeofFinder> {
public:
  std::vector<const UnaryExprOrTypeTraitExpr *> Sizeofs;

  void VisitBinMul(const BinaryOperator *E) {
    Visit(E->getLHS());
    Visit(E->getRHS());
  }

  void VisitImplicitCastExpr(const ImplicitCastExpr *E) {
    return Visit(E->getSubExpr());
  }

  void VisitParenExpr(const ParenExpr *E) {
    return Visit(E->getSubExpr());
  }

  void VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *E) {
    if (E->getKind() != UETT_SizeOf)
      return;

    Sizeofs.push_back(E);
  }
};

// Determine if the pointee and sizeof types are compatible.  Here
// we ignore constness of pointer types.
static bool typesCompatible(ASTContext &C, QualType A, QualType B) {
  // sizeof(void*) is compatible with any other pointer.
  if (B->isVoidPointerType() && A->getAs<PointerType>())
    return true;

  // sizeof(pointer type) is compatible with void*
  if (A->isVoidPointerType() && B->getAs<PointerType>())
    return true;

  while (true) {
    A = A.getCanonicalType();
    B = B.getCanonicalType();

    if (A.getTypePtr() == B.getTypePtr())
      return true;

    if (const PointerType *ptrA = A->getAs<PointerType>())
      if (const PointerType *ptrB = B->getAs<PointerType>()) {
        A = ptrA->getPointeeType();
        B = ptrB->getPointeeType();
        continue;
      }

    break;
  }

  return false;
}

static bool compatibleWithArrayType(ASTContext &C, QualType PT, QualType T) {
  // Ex: 'int a[10][2]' is compatible with 'int', 'int[2]', 'int[10][2]'.
  while (const ArrayType *AT = T->getAsArrayTypeUnsafe()) {
    QualType ElemType = AT->getElementType();
    if (typesCompatible(C, PT, AT->getElementType()))
      return true;
    T = ElemType;
  }

  return false;
}

class MallocSizeofChecker : public Checker<check::ASTCodeBody> {
public:
  void checkASTCodeBody(const Decl *D, AnalysisManager& mgr,
                        BugReporter &BR) const {
    AnalysisDeclContext *ADC = mgr.getAnalysisDeclContext(D);
    CastedAllocFinder Finder(&BR.getContext());
    Finder.Visit(D->getBody());
    for (const auto &CallRec : Finder.Calls) {
      QualType CastedType = CallRec.CastedExpr->getType();
      if (!CastedType->isPointerType())
        continue;
      QualType PointeeType = CastedType->getPointeeType();
      if (PointeeType->isVoidType())
        continue;

      for (const Expr *Arg : CallRec.AllocCall->arguments()) {
        if (!Arg->getType()->isIntegralOrUnscopedEnumerationType())
          continue;

        SizeofFinder SFinder;
        SFinder.Visit(Arg);
        if (SFinder.Sizeofs.size() != 1)
          continue;

        QualType SizeofType = SFinder.Sizeofs[0]->getTypeOfArgument();

        if (typesCompatible(BR.getContext(), PointeeType, SizeofType))
          continue;

        // If the argument to sizeof is an array, the result could be a
        // pointer to any array element.
        if (compatibleWithArrayType(BR.getContext(), PointeeType, SizeofType))
          continue;

        const TypeSourceInfo *TSI = nullptr;
        if (const auto *VD =
                dyn_cast<const VarDecl *>(CallRec.CastedExprParent)) {
          TSI = VD->getTypeSourceInfo();
        } else {
          TSI = CallRec.ExplicitCastType;
        }

        SmallString<64> buf;
        toolchain::raw_svector_ostream OS(buf);

        OS << "Result of ";
        const FunctionDecl *Callee = CallRec.AllocCall->getDirectCallee();
        if (Callee && Callee->getIdentifier())
          OS << '\'' << Callee->getIdentifier()->getName() << '\'';
        else
          OS << "call";
        OS << " is converted to a pointer of type '" << PointeeType
           << "', which is incompatible with "
           << "sizeof operand type '" << SizeofType << "'";
        SmallVector<SourceRange, 4> Ranges;
        Ranges.push_back(CallRec.AllocCall->getCallee()->getSourceRange());
        Ranges.push_back(SFinder.Sizeofs[0]->getSourceRange());
        if (TSI)
          Ranges.push_back(TSI->getTypeLoc().getSourceRange());

        PathDiagnosticLocation L = PathDiagnosticLocation::createBegin(
            CallRec.AllocCall->getCallee(), BR.getSourceManager(), ADC);

        BR.EmitBasicReport(D, this, "Allocator sizeof operand mismatch",
                           categories::UnixAPI, OS.str(), L, Ranges);
      }
    }
  }
};

}

void ento::registerMallocSizeofChecker(CheckerManager &mgr) {
  mgr.registerChecker<MallocSizeofChecker>();
}

bool ento::shouldRegisterMallocSizeofChecker(const CheckerManager &mgr) {
  return true;
}
