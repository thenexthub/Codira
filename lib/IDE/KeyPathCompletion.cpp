//===--- KeyPathCompletion.cpp --------------------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/IDE/KeyPathCompletion.h"
#include "language/IDE/CodeCompletion.h"
#include "language/IDE/CompletionLookup.h"
#include "language/Sema/ConstraintSystem.h"

using namespace language;
using namespace language::constraints;
using namespace language::ide;

void KeyPathTypeCheckCompletionCallback::sawSolutionImpl(
    const constraints::Solution &S) {
  // Determine the code completion.
  size_t ComponentIndex = 0;
  for (auto &Component : KeyPath->getComponents()) {
    if (Component.getKind() == KeyPathExpr::Component::Kind::CodeCompletion) {
      break;
    } else {
      ComponentIndex++;
    }
  }
  assert(ComponentIndex < KeyPath->getComponents().size() &&
         "Didn't find a code compleiton component?");

  Type BaseType;
  if (ComponentIndex == 0) {
    // We are completing on the root and need to extract the key path's root
    // type.
    if (auto *rootTy = KeyPath->getExplicitRootType()) {
      BaseType = S.getResolvedType(rootTy);
    } else {
      // The key path doesn't have a root TypeRepr set, so we can't look the key
      // path's root up through it. Build a constraint locator and look the
      // root type up through it.
      // FIXME: Improve the linear search over S.typeBindings when it's possible
      // to look up type variables by their locators.
      auto RootLocator =
          S.getConstraintLocator(KeyPath, {ConstraintLocator::KeyPathRoot});
      auto BaseVariableTypeBinding =
          toolchain::find_if(S.typeBindings, [&RootLocator](const auto &Entry) {
            return Entry.first->getImpl().getLocator() == RootLocator;
          });
      if (BaseVariableTypeBinding != S.typeBindings.end()) {
        BaseType = S.simplifyType(BaseVariableTypeBinding->second);
      }
    }
  } else {
    // We are completing after a component. Get the previous component's result
    // type.
    BaseType = S.simplifyType(S.getType(KeyPath, ComponentIndex - 1));
  }
  if (BaseType.isNull()) {
    return;
  }

  // If ExpectedTy is a duplicate of any other result, ignore this solution.
  if (toolchain::any_of(Results, [&](const Result &R) {
        return R.BaseType->isEqual(BaseType);
      })) {
    return;
  }
  Results.push_back({BaseType, /*OnRoot=*/(ComponentIndex == 0)});
}

void KeyPathTypeCheckCompletionCallback::collectResults(
    DeclContext *DC, SourceLoc DotLoc,
    ide::CodeCompletionContext &CompletionCtx) {
  ASTContext &Ctx = DC->getASTContext();
  CompletionLookup Lookup(CompletionCtx.getResultSink(), Ctx, DC,
                          &CompletionCtx);

  if (DotLoc.isValid()) {
    Lookup.setHaveDot(DotLoc);
  }
  Lookup.shouldCheckForDuplicates(Results.size() > 1);

  for (auto &Result : Results) {
    Lookup.setIsCodiraKeyPathExpr(Result.OnRoot);
    Lookup.getValueExprCompletions(Result.BaseType);
  }

  collectCompletionResults(CompletionCtx, Lookup, DC, ExpectedTypeContext(),
                           /*CanCurrDeclContextHandleAsync=*/false);
}
