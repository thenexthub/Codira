//===--- TypeContextInfo.cpp ----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/IDE/TypeContextInfo.h"
#include "ExprContextAnalysis.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/NameLookup.h"
#include "language/AST/USRGeneration.h"
#include "language/IDE/TypeCheckCompletionCallback.h"
#include "language/Parse/IDEInspectionCallbacks.h"
#include "language/Sema/ConstraintSystem.h"
#include "language/Sema/IDETypeChecking.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "toolchain/ADT/SmallSet.h"

using namespace language;
using namespace ide;

class ContextInfoCallbacks : public CodeCompletionCallbacks,
                             public DoneParsingCallback {
  TypeContextInfoConsumer &Consumer;
  SourceLoc Loc;
  Expr *ParsedExpr = nullptr;
  DeclContext *CurDeclContext = nullptr;

  void getImplicitMembers(Type T, SmallVectorImpl<ValueDecl *> &Result);

public:
  ContextInfoCallbacks(Parser &P, TypeContextInfoConsumer &Consumer)
      : CodeCompletionCallbacks(P), DoneParsingCallback(), Consumer(Consumer) {}

  void completePostfixExprBeginning(CodeCompletionExpr *E) override;
  void completeForEachSequenceBeginning(CodeCompletionExpr *E) override;
  void completeCaseStmtBeginning(CodeCompletionExpr *E) override;

  void completeCallArg(CodeCompletionExpr *E) override;
  void completeReturnStmt(CodeCompletionExpr *E) override;
  void completeThenStmt(CodeCompletionExpr *E) override;
  void completeYieldStmt(CodeCompletionExpr *E,
                         std::optional<unsigned> yieldIndex) override;

  void completeUnresolvedMember(CodeCompletionExpr *E,
                                SourceLoc DotLoc) override;

  void doneParsing(SourceFile *SrcFile) override;
};

void ContextInfoCallbacks::completePostfixExprBeginning(CodeCompletionExpr *E) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}

void ContextInfoCallbacks::completeForEachSequenceBeginning(
    CodeCompletionExpr *E) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}
void ContextInfoCallbacks::completeCallArg(CodeCompletionExpr *E) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}
void ContextInfoCallbacks::completeReturnStmt(CodeCompletionExpr *E) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}
void ContextInfoCallbacks::completeThenStmt(CodeCompletionExpr *E) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}
void ContextInfoCallbacks::completeYieldStmt(
    CodeCompletionExpr *E, std::optional<unsigned> yieldIndex) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}
void ContextInfoCallbacks::completeUnresolvedMember(CodeCompletionExpr *E,
                                                    SourceLoc DotLoc) {
  CurDeclContext = P.CurDeclContext;
  ParsedExpr = E;
}

void ContextInfoCallbacks::completeCaseStmtBeginning(CodeCompletionExpr *E) {
  // TODO: Implement?
}

class TypeContextInfoCallback : public TypeCheckCompletionCallback {
  Expr *ParsedExpr;
  SmallVector<Type, 2> Types;

  void sawSolutionImpl(const constraints::Solution &S) override {
    if (!S.hasType(ParsedExpr)) {
      return;
    }
    if (Type T = getTypeForCompletion(S, ParsedExpr)) {
      Types.push_back(T);
    }
  }

public:
  TypeContextInfoCallback(Expr *ParsedExpr) : ParsedExpr(ParsedExpr) {}

  ArrayRef<Type> getTypes() const { return Types; }
};

void ContextInfoCallbacks::doneParsing(SourceFile *SrcFile) {
  if (!ParsedExpr)
    return;

  TypeContextInfoCallback TypeCheckCallback(ParsedExpr);
  {
    toolchain::SaveAndRestore<TypeCheckCompletionCallback *> CompletionCollector(
        Context.CompletionCallback, &TypeCheckCallback);
    typeCheckContextAt(
        TypeCheckASTNodeAtLocContext::declContext(CurDeclContext),
        ParsedExpr->getLoc());
  }

  toolchain::SmallSet<CanType, 2> seenTypes;
  SmallVector<TypeContextInfoItem, 2> results;

  for (auto T : TypeCheckCallback.getTypes()) {
    if (T->is<ErrorType>() || T->is<UnresolvedType>())
      continue;

    T = T->getRValueType();

    auto interfaceTy = T;
    if (interfaceTy->hasArchetype())
      interfaceTy = interfaceTy->mapTypeOutOfContext();

    // TODO: Do we need '.none' for Optionals?
    auto objTy = T->lookThroughAllOptionalTypes();
    if (!seenTypes.insert(objTy->getCanonicalType()).second)
      continue;

    results.emplace_back(interfaceTy);
    auto &item = results.back();
    getImplicitMembers(objTy, item.ImplicitMembers);
  }

  Consumer.handleResults(results);
}

void ContextInfoCallbacks::getImplicitMembers(
    Type T, SmallVectorImpl<ValueDecl *> &Result) {

  if (!T->mayHaveMembers())
    return;

  class LocalConsumer : public VisibleDeclConsumer {
    DeclContext *DC;
    Type T;
    SmallVectorImpl<ValueDecl *> &Result;

    bool canBeImplictMember(ValueDecl *VD) {
      if (VD->isOperator())
        return false;

      // Enum element decls can always be referenced by implicit member
      // expression.
      if (isa<EnumElementDecl>(VD))
        return true;

      // Static properties which is convertible to 'Self'.
      if (auto *Var = dyn_cast<VarDecl>(VD)) {
        if (Var->isStatic()) {
          auto declTy = T->getTypeOfMember(Var);
          if (declTy->isEqual(T) ||
              language::isConvertibleTo(declTy, T, /*openArchetypes=*/true, *DC))
            return true;
        }
      }

      return false;
    }

  public:
    LocalConsumer(DeclContext *DC, Type T, SmallVectorImpl<ValueDecl *> &Result)
        : DC(DC), T(T), Result(Result) {}

    void foundDecl(ValueDecl *VD, DeclVisibilityKind Reason,
                   DynamicLookupInfo) override {
      if (canBeImplictMember(VD) && !VD->shouldHideFromEditor())
        Result.push_back(VD);
    }

  } LocalConsumer(CurDeclContext, T, Result);

  lookupVisibleMemberDecls(LocalConsumer, MetatypeType::get(T),
                           Loc, CurDeclContext,
                           /*includeInstanceMembers=*/false,
                           /*includeDerivedRequirements*/false,
                           /*includeProtocolExtensionMembers*/true);
}

IDEInspectionCallbacksFactory *language::ide::makeTypeContextInfoCallbacksFactory(
    TypeContextInfoConsumer &Consumer) {

  // CC callback factory which produces 'ContextInfoCallbacks'.
  class ContextInfoCallbacksFactoryImpl
      : public IDEInspectionCallbacksFactory {
    TypeContextInfoConsumer &Consumer;

  public:
    ContextInfoCallbacksFactoryImpl(TypeContextInfoConsumer &Consumer)
        : Consumer(Consumer) {}

    Callbacks createCallbacks(Parser &P) override {
      auto Callbacks = std::make_shared<ContextInfoCallbacks>(P, Consumer);
      return {Callbacks, Callbacks};
    }
  };

  return new ContextInfoCallbacksFactoryImpl(Consumer);
}
