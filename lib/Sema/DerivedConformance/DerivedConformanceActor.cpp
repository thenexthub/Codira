//===--- DerivedConformanceActor.cpp ----------------------------*- C++ -*-===//
//
// This source file is part of the Codira.org open source project
//
// Copyright (c) 2018 - 2025 Apple Inc. and the Codira project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://language.org/LICENSE.txt for license information
// See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
//
//===----------------------------------------------------------------------===//
//
// This file implements explicit derivation of the Actor protocol
// for actor types.
//
// Codira Evolution pitch thread:
// https://forums.code.org/t/support-custom-executors-in-language-concurrency/44425
//
//===----------------------------------------------------------------------===//

#include "CodeSynthesis.h"
#include "DerivedConformance.h"
#include "TypeChecker.h"
#include "language/AST/AvailabilityInference.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Module.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Pattern.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/Stmt.h"
#include "language/AST/Types.h"
#include "language/Strings.h"

using namespace language;

bool DerivedConformance::canDeriveActor(DeclContext *dc,
                                        NominalTypeDecl *nominal) {
  auto classDecl = dyn_cast<ClassDecl>(nominal);
  return classDecl && classDecl->isActor() && dc == nominal &&
        !classDecl->getUnownedExecutorProperty();
}

/// Turn a Builtin.Executor value into an UnownedSerialExecutor.
static Expr *constructUnownedSerialExecutor(ASTContext &ctx,
                                            Expr *arg) {
  auto executorDecl = ctx.getUnownedSerialExecutorDecl();
  if (!executorDecl) return nullptr;

  for (auto member: executorDecl->getAllMembers()) {
    auto ctor = dyn_cast<ConstructorDecl>(member);
    if (!ctor) continue;
    auto params = ctor->getParameters();
    if (params->size() != 1 ||
        !params->get(0)->getInterfaceType()->is<BuiltinExecutorType>())
      continue;

    Type executorType = executorDecl->getDeclaredInterfaceType();

    Type ctorType = ctor->getInterfaceType();

    // We have the right initializer. Build a reference to it of type:
    //   (UnownedSerialExecutor.Type)
    //      -> (Builtin.Executor) -> UnownedSerialExecutor
    auto initRef = new (ctx) DeclRefExpr(ctor, DeclNameLoc(), /*implicit*/true,
                                         AccessSemantics::Ordinary,
                                         ctorType);

    // Apply the initializer to the metatype, building an expression of type:
    //   (Builtin.Executor) -> UnownedSerialExecutor
    auto metatypeRef = TypeExpr::createImplicit(executorType, ctx);
    Type ctorAppliedType = ctorType->getAs<FunctionType>()->getResult();
    auto selfApply = ConstructorRefCallExpr::create(ctx, initRef, metatypeRef,
                                                    ctorAppliedType);
    selfApply->setImplicit(true);
    selfApply->setThrows(nullptr);

    // Call the constructor, building an expression of type
    // UnownedSerialExecutor.
    auto *argList = ArgumentList::forImplicitUnlabeled(ctx, {arg});
    auto call = CallExpr::createImplicit(ctx, selfApply, argList);
    call->setType(executorType);
    call->setThrows(nullptr);
    return call;
  }

  return nullptr;
}

static std::pair<BraceStmt *, bool>
deriveBodyActor_unownedExecutor(AbstractFunctionDecl *getter, void *) {
  // var unownedExecutor: UnownedSerialExecutor {
  //   get {
  //     return Builtin.buildDefaultActorExecutorRef(self)
  //   }
  // }
  ASTContext &ctx = getter->getASTContext();

  // Produce an empty brace statement on failure.
  auto failure = [&]() -> std::pair<BraceStmt *, bool> {
    auto body = BraceStmt::create(
      ctx, SourceLoc(), { }, SourceLoc(), /*implicit=*/true);
    return { body, /*isTypeChecked=*/true };
  };

  // Build a reference to self.
  Type selfType = getter->getImplicitSelfDecl()->getTypeInContext();
  Expr *selfArg = DerivedConformance::createSelfDeclRef(getter);
  selfArg->setType(selfType);

  // The builtin call gives us a Builtin.Executor.
  auto builtinCall =
    DerivedConformance::createBuiltinCall(ctx,
                        BuiltinValueKind::BuildDefaultActorExecutorRef,
                                          {selfType}, {selfArg});

  // Turn that into an UnownedSerialExecutor.
  auto initCall = constructUnownedSerialExecutor(ctx, builtinCall);
  if (!initCall) return failure();

  auto *ret = ReturnStmt::createImplicit(ctx, initCall);

  auto body = BraceStmt::create(
    ctx, SourceLoc(), { ret }, SourceLoc(), /*implicit=*/true);
  return { body, /*isTypeChecked=*/true };
}

/// Derive the declaration of Actor's unownedExecutor property.
static ValueDecl *deriveActor_unownedExecutor(DerivedConformance &derived) {
  ASTContext &ctx = derived.Context;

  // Retrieve the types and declarations we'll need to form this operation.
  auto executorDecl = ctx.getUnownedSerialExecutorDecl();
  if (!executorDecl) {
    derived.Nominal->diagnose(
      diag::concurrency_lib_missing, "UnownedSerialExecutor");
    return nullptr;
  }
  Type executorType = executorDecl->getDeclaredInterfaceType();

  auto propertyPair = derived.declareDerivedProperty(
      DerivedConformance::SynthesizedIntroducer::Var, ctx.Id_unownedExecutor,
      executorType, /*static*/ false, /*final*/ false);
  auto property = propertyPair.first;
  property->setSynthesized(true);
  property->getAttrs().add(new (ctx) SemanticsAttr(SEMANTICS_DEFAULT_ACTOR,
                                                   SourceLoc(), SourceRange(),
                                                   /*implicit*/ true));
  property->getAttrs().add(NonisolatedAttr::createImplicit(ctx));

  // Make the property implicitly final.
  property->getAttrs().add(new (ctx) FinalAttr(/*IsImplicit=*/true));
  if (property->getFormalAccess() == AccessLevel::Open)
    property->overwriteAccess(AccessLevel::Public);

  // Infer availability.
  SmallVector<const Decl *, 2> asAvailableAs;
  asAvailableAs.push_back(executorDecl);
  if (auto enclosingDecl = property->getInnermostDeclWithAvailability())
    asAvailableAs.push_back(enclosingDecl);

  AvailabilityInference::applyInferredAvailableAttrs(property, asAvailableAs);

  auto getter = derived.addGetterToReadOnlyDerivedProperty(property);
  getter->setBodySynthesizer(deriveBodyActor_unownedExecutor);

  derived.addMembersToConformanceContext(
    { property, propertyPair.second, });
  return property;
}

ValueDecl *DerivedConformance::deriveActor(ValueDecl *requirement) {
  auto var = dyn_cast<VarDecl>(requirement);
  if (!var)
    return nullptr;

  if (var->getName() == Context.Id_unownedExecutor)
    return deriveActor_unownedExecutor(*this);

  return nullptr;
}
