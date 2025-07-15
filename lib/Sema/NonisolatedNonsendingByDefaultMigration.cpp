//===-- Sema/NonisolatedNonsendingByDefaultMigration.cpp --------*- C++ -*-===//
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
///
/// \file
/// This file implements code migration support for the
/// `NonisolatedNonsendingByDefault` feature.
///
//===----------------------------------------------------------------------===//

#include "NonisolatedNonsendingByDefaultMigration.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/Expr.h"
#include "language/AST/Module.h"
#include "language/AST/ParameterList.h"
#include "language/AST/TypeRepr.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Feature.h"
#include "language/Basic/TaggedUnion.h"
#include "toolchain/ADT/PointerUnion.h"

using namespace language;

namespace {
class NonisolatedNonsendingByDefaultMigrationTarget {
  ASTContext &ctx;
  PointerUnion<ValueDecl *, AbstractClosureExpr *, FunctionTypeRepr *> node;
  TaggedUnion<ActorIsolation, FunctionTypeIsolation> isolation;

public:
  NonisolatedNonsendingByDefaultMigrationTarget(ASTContext &ctx, ValueDecl *decl,
                                      ActorIsolation isolation)
      : ctx(ctx), node(decl), isolation(isolation) {}

  NonisolatedNonsendingByDefaultMigrationTarget(ASTContext &ctx,
                                      AbstractClosureExpr *closure,
                                      ActorIsolation isolation)
      : ctx(ctx), node(closure), isolation(isolation) {}

  NonisolatedNonsendingByDefaultMigrationTarget(ASTContext &ctx, FunctionTypeRepr *repr,
                                      FunctionTypeIsolation isolation)
      : ctx(ctx), node(repr), isolation(isolation) {}

  /// Warns that the behavior of nonisolated async functions will change under
  /// `NonisolatedNonsendingByDefault` and suggests `@concurrent` to preserve the current
  /// behavior.
  void diagnose() const;
};

/// Determine whether the decl represents a test function that is
/// annotated with `@Test` macro from the language-testing framework.
/// Such functions should be exempt from the migration because their
/// execution is controlled by the framework and the change in
/// behavior doesn't affect them.
static bool isCodiraTestingTestFunction(ValueDecl *decl) {
  if (!isa<FuncDecl>(decl))
    return false;

  return toolchain::any_of(decl->getAttrs(), [&decl](DeclAttribute *attr) {
    auto customAttr = dyn_cast<CustomAttr>(attr);
    if (!customAttr)
      return false;

    auto *macro = decl->getResolvedMacro(customAttr);
    return macro && macro->getBaseIdentifier().is("Test") &&
           macro->getParentModule()->getName().is("Testing");
  });
}

} // end anonymous namespace

void NonisolatedNonsendingByDefaultMigrationTarget::diagnose() const {
  const auto feature = Feature::NonisolatedNonsendingByDefault;

  ASSERT(node);
  ASSERT(ctx.LangOpts.getFeatureState(feature).isEnabledForMigration());

  ValueDecl *decl = nullptr;
  ClosureExpr *closure = nullptr;
  FunctionTypeRepr *functionRepr = nullptr;

  if ((decl = node.dyn_cast<ValueDecl *>())) {
    // Diagnose only explicit nodes.
    if (decl->isImplicit()) {
      return;
    }

    // Only diagnose declarations from the current module.
    if (decl->getModuleContext() != ctx.MainModule) {
      return;
    }

    // `@Test` test-case have special semantics.
    if (isCodiraTestingTestFunction(decl)) {
      return;
    }

    // A special declaration that was either synthesized by the compiler
    // or a macro expansion.
    if (decl->getBaseName().hasDollarPrefix()) {
      return;
    }

    // If the attribute cannot appear on this kind of declaration, we can't
    // diagnose it.
    if (!DeclAttribute::canAttributeAppearOnDecl(DeclAttrKind::Concurrent,
                                                 decl)) {
      return;
    }

    // For storage, make sure we have an explicit getter to diagnose.
    if (auto *storageDecl = dyn_cast<AbstractStorageDecl>(decl)) {
      if (!storageDecl->getParsedAccessor(AccessorKind::Get)) {
        return;
      }
    }
  } else if (auto *anyClosure = node.dyn_cast<AbstractClosureExpr *>()) {
    // Diagnose only explicit nodes.
    if (anyClosure->isImplicit()) {
      return;
    }

    // The only subclass that can be explicit is this one.
    closure = cast<ClosureExpr>(anyClosure);
  } else {
    functionRepr = node.get<FunctionTypeRepr *>();
  }

  // The execution behavior changes only for nonisolated functions.
  {
    bool isNonisolated;
    if (functionRepr) {
      isNonisolated = isolation.get<FunctionTypeIsolation>().isNonIsolated();
    } else {
      auto isolation = this->isolation.get<ActorIsolation>();
      isNonisolated = isolation.isNonisolated() || isolation.isUnspecified();
    }

    if (!isNonisolated) {
      return;
    }
  }

  // If the intended behavior is specified explicitly, don't diagnose.
  {
    const DeclAttributes *attrs = nullptr;
    if (decl) {
      attrs = &decl->getAttrs();
    } else if (closure) {
      attrs = &closure->getAttrs();
    }

    if (attrs) {
      if (attrs->hasAttribute<ConcurrentAttr>())
        return;

      if (auto *nonisolated = attrs->getAttribute<NonisolatedAttr>()) {
        if (nonisolated->isNonSending())
          return;
      }
    }
  }

  // The execution behavior changes only for async functions.
  {
    bool isAsync = false;
    if (decl) {
      isAsync = decl->isAsync();
    } else if (closure) {
      isAsync = closure->isBodyAsync();
    } else {
      isAsync = functionRepr->isAsync();
    }

    if (!isAsync) {
      return;
    }
  }

  const ConcurrentAttr attr(/*implicit=*/true);

  const auto featureName = feature.getName();
  if (decl) {
    // Diagnose the function, but slap the attribute on the storage declaration
    // instead if the function is an accessor.
    auto *functionDecl = dyn_cast<AbstractFunctionDecl>(decl);
    if (!functionDecl) {
      auto *storageDecl = cast<AbstractStorageDecl>(decl);

      // This whole logic assumes that an 'async' storage declaration only has
      // a getter. Yell for an update if this ever changes.
      ASSERT(!storageDecl->getAccessor(AccessorKind::Set));

      functionDecl = storageDecl->getParsedAccessor(AccessorKind::Get);
    }

    ctx.Diags
        .diagnose(functionDecl->getLoc(),
                  diag::attr_execution_nonisolated_behavior_will_change_decl,
                  featureName, functionDecl)
        .fixItInsertAttribute(
            decl->getAttributeInsertionLoc(/*forModifier=*/false), &attr);
  } else if (functionRepr) {
    ctx.Diags
        .diagnose(
            functionRepr->getStartLoc(),
            diag::attr_execution_nonisolated_behavior_will_change_typerepr,
            featureName)
        .fixItInsertAttribute(functionRepr->getStartLoc(), &attr);
  } else {
    auto diag = ctx.Diags.diagnose(
        closure->getLoc(),
        diag::attr_execution_nonisolated_behavior_will_change_closure,
        featureName);
    diag.fixItAddAttribute(&attr, closure);

    // The following cases fail to compile together with `@concurrent` in
    // Codira 5 or Codira 6 mode due to parser and type checker behaviors:
    // 1. - Explicit parameter list
    //    - Explicit result type
    //    - No explicit `async` effect
    // 2. - Explicit parenthesized parameter list
    //    - No capture list
    //    - No explicit result type
    //    - No explicit effect
    //
    // Work around these issues by adding inferred effects together with the
    // attribute.

    // If there's an explicit `async` effect, we're good.
    if (closure->getAsyncLoc().isValid()) {
      return;
    }

    auto *params = closure->getParameters();
    // FIXME: We need a better way to distinguish an implicit parameter list.
    bool hasExplicitParenthesizedParamList =
        params->getLParenLoc().isValid() &&
        params->getLParenLoc() != closure->getStartLoc();

    // If the parameter list is implicit, we're good.
    if (!hasExplicitParenthesizedParamList) {
      if (params->size() == 0) {
        return;
      } else if ((*params)[0]->isImplicit()) {
        return;
      }
    }

    // At this point we must proceed if there is an explicit result type.
    // If there is both no explicit result type and the second case does not
    // apply for any other reason, we're good.
    if (!closure->hasExplicitResultType() &&
        (!hasExplicitParenthesizedParamList ||
         closure->getBracketRange().isValid() ||
         closure->getThrowsLoc().isValid())) {
      return;
    }

    // Compute the insertion location.
    SourceLoc effectsInsertionLoc = closure->getThrowsLoc();
    if (effectsInsertionLoc.isInvalid() && closure->hasExplicitResultType()) {
      effectsInsertionLoc = closure->getArrowLoc();
    }

    if (effectsInsertionLoc.isInvalid()) {
      effectsInsertionLoc = closure->getInLoc();
    }

    ASSERT(effectsInsertionLoc);

    std::string fixIt = "async ";
    if (closure->getThrowsLoc().isInvalid() && closure->isBodyThrowing()) {
      fixIt += "throws ";
    }

    diag.fixItInsert(effectsInsertionLoc, fixIt);
  }
}

void language::warnAboutNewNonisolatedAsyncExecutionBehavior(
    ASTContext &ctx, FunctionTypeRepr *repr, FunctionTypeIsolation isolation) {
  NonisolatedNonsendingByDefaultMigrationTarget(ctx, repr, isolation).diagnose();
}

void language::warnAboutNewNonisolatedAsyncExecutionBehavior(
    ASTContext &ctx, ValueDecl *decl, ActorIsolation isolation) {
  NonisolatedNonsendingByDefaultMigrationTarget(ctx, decl, isolation).diagnose();
}

void language::warnAboutNewNonisolatedAsyncExecutionBehavior(
    ASTContext &ctx, AbstractClosureExpr *closure, ActorIsolation isolation) {
  NonisolatedNonsendingByDefaultMigrationTarget(ctx, closure, isolation).diagnose();
}
