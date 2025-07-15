//===--- ConformingMethodList.cpp -----------------------------------------===//
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

#include "language/IDE/ConformingMethodList.h"
#include "ExprContextAnalysis.h"
#include "language/AST/ASTDemangler.h"
#include "language/AST/ConformanceLookup.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/NameLookup.h"
#include "language/AST/USRGeneration.h"
#include "language/Basic/Assertions.h"
#include "language/IDE/TypeCheckCompletionCallback.h"
#include "language/Parse/IDEInspectionCallbacks.h"
#include "language/Sema/IDETypeChecking.h"
#include "language/Sema/ConstraintSystem.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"

using namespace language;
using namespace ide;

namespace {
class ConformingMethodListCallbacks : public CodeCompletionCallbacks,
                                      public DoneParsingCallback {
  ArrayRef<const char *> ExpectedTypeNames;
  ConformingMethodListConsumer &Consumer;
  SourceLoc Loc;
  CodeCompletionExpr *CCExpr = nullptr;
  DeclContext *CurDeclContext = nullptr;

  void getMatchingMethods(Type T,
                          toolchain::SmallPtrSetImpl<ProtocolDecl*> &expectedTypes,
                          SmallVectorImpl<ValueDecl *> &result);

public:
  ConformingMethodListCallbacks(Parser &P,
                                ArrayRef<const char *> ExpectedTypeNames,
                                ConformingMethodListConsumer &Consumer)
      : CodeCompletionCallbacks(P), DoneParsingCallback(),
        ExpectedTypeNames(ExpectedTypeNames), Consumer(Consumer) {}

  // Only handle callbacks for suffix completions.
  // {
  void completeDotExpr(CodeCompletionExpr *E, SourceLoc DotLoc) override;
  void completePostfixExpr(CodeCompletionExpr *E, bool hasSpace) override;
  // }

  void doneParsing(SourceFile *SrcFile) override;
};

void ConformingMethodListCallbacks::completeDotExpr(CodeCompletionExpr *E,
                                                    SourceLoc DotLoc) {
  CurDeclContext = P.CurDeclContext;
  CCExpr = E;
}

void ConformingMethodListCallbacks::completePostfixExpr(CodeCompletionExpr *E,
                                                        bool hasSpace) {
  CurDeclContext = P.CurDeclContext;
  CCExpr = E;
}

class ConformingMethodListCallback : public TypeCheckCompletionCallback {
public:
  struct Result {
    Type Ty;

    /// Types of variables that were determined in the solution that produced
    /// this result. This in particular includes parameters of closures that
    /// were type-checked with the code completion expression.
    toolchain::SmallDenseMap<const VarDecl *, Type> SolutionSpecificVarTypes;
  };
private:
  CodeCompletionExpr *CCExpr;

  SmallVector<Result> Results;

  void sawSolutionImpl(const constraints::Solution &S) override {
    if (!S.hasType(CCExpr->getBase())) {
      return;
    }
    if (Type T = getTypeForCompletion(S, CCExpr->getBase())) {
      toolchain::SmallDenseMap<const VarDecl *, Type> SolutionSpecificVarTypes;
      getSolutionSpecificVarTypes(S, SolutionSpecificVarTypes);
      Results.push_back({T, SolutionSpecificVarTypes});
    }
  }

public:
  ConformingMethodListCallback(CodeCompletionExpr *CCExpr) : CCExpr(CCExpr) {}

  ArrayRef<Result> getResults() const { return Results; }
};

void ConformingMethodListCallbacks::doneParsing(SourceFile *SrcFile) {
  if (!CCExpr || !CCExpr->getBase())
    return;

  ConformingMethodListCallback TypeCheckCallback(CCExpr);
  {
    toolchain::SaveAndRestore<TypeCheckCompletionCallback *> CompletionCollector(
        Context.CompletionCallback, &TypeCheckCallback);
    typeCheckContextAt(
        TypeCheckASTNodeAtLocContext::declContext(CurDeclContext),
        CCExpr->getLoc());
  }

  if (TypeCheckCallback.getResults().size() != 1) {
    // Either no results or results were ambiguous, which we cannot handle.
    return;
  }
  auto Res = TypeCheckCallback.getResults()[0];
  Type T = Res.Ty;
  WithSolutionSpecificVarTypesRAII VarType(Res.SolutionSpecificVarTypes);

  if (!T || T->is<ErrorType>() || T->is<UnresolvedType>())
    return;

  T = T->getRValueType();

  // If there are no (instance) members for this type, bail.
  if (!T->mayHaveMembers() || T->is<ModuleType>()) {
    return;
  }

  auto interfaceTy = T;
  if (T->hasArchetype())
    interfaceTy = interfaceTy->mapTypeOutOfContext();

  toolchain::SmallPtrSet<ProtocolDecl*, 8> expectedProtocols;
  for (auto Name: ExpectedTypeNames) {
    if (auto *PD = resolveProtocolName(CurDeclContext, Name)) {
      expectedProtocols.insert(PD);
    }
  }

  // Collect the matching methods.
  ConformingMethodListResult result(CurDeclContext, interfaceTy);
  getMatchingMethods(T, expectedProtocols, result.Members);

  Consumer.handleResult(result);
}

void ConformingMethodListCallbacks::getMatchingMethods(
    Type T, toolchain::SmallPtrSetImpl<ProtocolDecl*> &expectedTypes,
    SmallVectorImpl<ValueDecl *> &result) {
  assert(T->mayHaveMembers() && !T->is<ModuleType>());

  class LocalConsumer : public VisibleDeclConsumer {
    /// The type of the parsed expression.
    Type T;

    /// The list of expected types.
    toolchain::SmallPtrSetImpl<ProtocolDecl*> &ExpectedTypes;

    /// Result sink to populate.
    SmallVectorImpl<ValueDecl *> &Result;

    /// Returns true if \p VD is a instance method whose return type conforms
    /// to the requested protocols.
    bool isMatchingMethod(ValueDecl *VD) {
      auto *FD = dyn_cast<FuncDecl>(VD);
      if (!FD)
        return false;
      if (FD->isStatic() || FD->isOperator())
        return false;

      assert(!T->hasTypeParameter());

      // T may contain primary archetypes from some fixed generic signature G.
      // This might be unrelated to the generic signature of FD. However if
      // FD has a generic parameter of its own and it returns a type containing
      // that parameter, we want to map it to the corresponding archetype
      // from the generic environment of FD, because all we do with the
      // resulting type is check conformance. If the conformance is conditional,
      // we might run into trouble with really complicated cases but the fake
      // archetype setup will mostly work.
      auto substitutions = T->getMemberSubstitutionMap(
          FD, FD->getGenericEnvironment());
      auto resultTy =  FD->getResultInterfaceType().subst(substitutions);
      if (resultTy->is<ErrorType>())
        return false;

      // The return type conforms to any of the requested protocols.
      for (auto Proto : ExpectedTypes) {
        if (checkConformance(resultTy, Proto))
          return true;
      }

      return false;
    }

  public:
    LocalConsumer(DeclContext *DC, Type T,
                  toolchain::SmallPtrSetImpl<ProtocolDecl*> &expectedTypes,
                  SmallVectorImpl<ValueDecl *> &result)
        : T(T), ExpectedTypes(expectedTypes), Result(result) {}

    void foundDecl(ValueDecl *VD, DeclVisibilityKind reason,
                   DynamicLookupInfo) override {
      if (isMatchingMethod(VD) && !VD->shouldHideFromEditor())
        Result.push_back(VD);
    }

  } LocalConsumer(CurDeclContext, T, expectedTypes, result);

  lookupVisibleMemberDecls(LocalConsumer, MetatypeType::get(T),
                           Loc, CurDeclContext,
                           /*includeInstanceMembers=*/false,
                           /*includeDerivedRequirements*/false,
                           /*includeProtocolExtensionMembers*/true);
}

} // anonymous namespace.

IDEInspectionCallbacksFactory *
language::ide::makeConformingMethodListCallbacksFactory(
    ArrayRef<const char *> expectedTypeNames,
    ConformingMethodListConsumer &Consumer) {

  // CC callback factory which produces 'ContextInfoCallbacks'.
  class ConformingMethodListCallbacksFactoryImpl
      : public IDEInspectionCallbacksFactory {
    ArrayRef<const char *> ExpectedTypeNames;
    ConformingMethodListConsumer &Consumer;

  public:
    ConformingMethodListCallbacksFactoryImpl(
        ArrayRef<const char *> ExpectedTypeNames,
        ConformingMethodListConsumer &Consumer)
        : ExpectedTypeNames(ExpectedTypeNames), Consumer(Consumer) {}

    Callbacks createCallbacks(Parser &P) override {
      auto Callback = std::make_shared<ConformingMethodListCallbacks>(
          P, ExpectedTypeNames, Consumer);
      return {Callback, Callback};
    }
  };

  return new ConformingMethodListCallbacksFactoryImpl(expectedTypeNames,
                                                      Consumer);
}
