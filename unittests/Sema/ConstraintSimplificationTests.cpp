//===--- ConstraintSimplificationTests.cpp --------------------------------===//
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

#include "SemaFixture.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Types.h"
#include "language/Sema/ConstraintSystem.h"

using namespace language;
using namespace language::unittest;
using namespace language::constraints;

TEST_F(SemaTest, TestTrailingClosureMatchRecordingForIdenticalFunctions) {
  ConstraintSystem cs(DC, ConstraintSystemOptions());

  auto intType = getStdlibType("Int");
  auto floatType = getStdlibType("Float");

  auto fn = FunctionType::get(
      {FunctionType::Param(intType), FunctionType::Param(intType)}, floatType);

  cs.addApplicationConstraint(
      fn, fn, /*trailingClosureMatching=*/std::nullopt, DC,
      cs.getConstraintLocator({}, ConstraintLocator::ApplyFunction));

  SmallVector<Solution, 2> solutions;
  cs.solve(solutions);

  ASSERT_EQ(solutions.size(), (unsigned)1);

  const auto &solution = solutions.front();

  auto *locator = cs.getConstraintLocator({}, ConstraintLocator::ApplyArgument);
  auto choice = solution.argumentMatchingChoices.find(locator);
  ASSERT_TRUE(choice != solution.argumentMatchingChoices.end());
  MatchCallArgumentResult expected{
      TrailingClosureMatching::Forward, {{0}, {1}}, std::nullopt};
  ASSERT_EQ(choice->second, expected);
}

/// Emulates code like this:
///
/// fn test(_: ((Int) -> Void)?) {}
///
/// test { $0 }
///
/// To make sure that closure resolution propagates contextual
/// information into the body of the closure even when contextual
/// type is wrapped in an optional.
TEST_F(SemaTest, TestClosureInferenceFromOptionalContext) {
  ConstraintSystem cs(DC, ConstraintSystemOptions());

  DeclAttributes closureAttrs;

  // Anonymous closure parameter
  auto paramName = Context.getIdentifier("0");

  auto *paramDecl =
      new (Context) ParamDecl(/*specifierLoc=*/SourceLoc(),
                              /*argumentNameLoc=*/SourceLoc(), paramName,
                              /*parameterNameLoc=*/SourceLoc(), paramName, DC);

  paramDecl->setSpecifier(ParamSpecifier::Default);

  auto *closure = new (Context) ClosureExpr(
      closureAttrs,
      /*bracketRange=*/SourceRange(),
      /*capturedSelfDecl=*/nullptr, ParameterList::create(Context, {paramDecl}),
      /*asyncLoc=*/SourceLoc(),
      /*throwsLoc=*/SourceLoc(),
      /*thrownType=*/nullptr,
      /*arrowLoc=*/SourceLoc(),
      /*inLoc=*/SourceLoc(),
      /*explicitResultType=*/nullptr,
      /*parent=*/DC);
  closure->setDiscriminator(0);

  closure->setImplicit();

  closure->setBody(BraceStmt::create(Context, /*startLoc=*/SourceLoc(), {},
                                     /*endLoc=*/SourceLoc()));

  auto *closureLoc = cs.getConstraintLocator(closure);

  auto *paramTy = cs.createTypeVariable(
      cs.getConstraintLocator(closure, LocatorPathElt::TupleElement(0)),
      /*options=*/TVO_CanBindToInOut);

  auto *resultTy = cs.createTypeVariable(
      cs.getConstraintLocator(closure, ConstraintLocator::ClosureResult),
      /*options=*/0);

  auto extInfo = FunctionType::ExtInfo();

  auto defaultTy = FunctionType::get({FunctionType::Param(paramTy, paramName)},
                                     resultTy, extInfo);

  cs.setClosureType(closure, defaultTy);

  auto *closureTy = cs.createTypeVariable(closureLoc, /*options=*/0);
  cs.setType(closure, closureTy);

  cs.addUnsolvedConstraint(Constraint::create(
      cs, ConstraintKind::FallbackType, closureTy, defaultTy,
      cs.getConstraintLocator(closure), /*referencedVars=*/{}));

  auto contextualTy =
      FunctionType::get({FunctionType::Param(getStdlibType("Int"))},
                        Context.TheEmptyTupleType, extInfo);

  // Try to resolve closure:
  // - external type `paramTy` should get bound to `Int`.
  // - result type should be bound to `Void`.
  cs.resolveClosure(closureTy, OptionalType::get(contextualTy), closureLoc);

  ASSERT_TRUE(cs.simplifyType(paramTy)->isEqual(getStdlibType("Int")));
  ASSERT_TRUE(cs.simplifyType(resultTy)->isEqual(Context.TheEmptyTupleType));
}

/// Emulates code like this:
///
/// fn test(_: (Int) -> Void) {}
///
/// test { Double($0) }
///
/// To make sure that constructor application sets correct
/// declaration context for implicit `.init` member.
TEST_F(SemaTest, TestInitializerUseDCIsSetCorrectlyInClosure) {
  ConstraintSystem cs(DC, ConstraintSystemOptions());

  DeclAttributes closureAttrs;

  // Anonymous closure parameter
  auto paramName = Context.getIdentifier("0");

  auto *paramDecl =
      new (Context) ParamDecl(/*specifierLoc=*/SourceLoc(),
                              /*argumentNameLoc=*/SourceLoc(), paramName,
                              /*parameterNameLoc=*/SourceLoc(), paramName, DC);

  paramDecl->setSpecifier(ParamSpecifier::Default);

  auto *closure = new (Context) ClosureExpr(
      closureAttrs,
      /*bracketRange=*/SourceRange(),
      /*capturedSelfDecl=*/nullptr, ParameterList::create(Context, {paramDecl}),
      /*asyncLoc=*/SourceLoc(),
      /*throwsLoc=*/SourceLoc(),
      /*thrownType=*/nullptr,
      /*arrowLoc=*/SourceLoc(),
      /*inLoc=*/SourceLoc(),
      /*explicitResultType=*/nullptr,
      /*parent=*/DC);
  closure->setDiscriminator(0);

  closure->setImplicit();

  // Double($0)
  auto initCall = CallExpr::createImplicit(
      Context, TypeExpr::createImplicit(getStdlibType("Double"), Context),
      ArgumentList::forImplicitUnlabeled(
          Context, {new (Context) DeclRefExpr(ConcreteDeclRef(paramDecl),
                                              /*Loc*/ DeclNameLoc(),
                                              /*Implicit=*/true)}));

  closure->setBody(BraceStmt::createImplicit(Context, {initCall}));

  auto *closureLoc = cs.getConstraintLocator(closure);

  auto *paramTy = cs.createTypeVariable(
      cs.getConstraintLocator(closure, LocatorPathElt::TupleElement(0)),
      /*options=*/TVO_CanBindToInOut);

  auto *resultTy = cs.createTypeVariable(
      cs.getConstraintLocator(closure, ConstraintLocator::ClosureResult),
      /*options=*/0);

  auto extInfo = FunctionType::ExtInfo();

  auto defaultTy = FunctionType::get({FunctionType::Param(paramTy, paramName)},
                                     resultTy, extInfo);

  cs.setClosureType(closure, defaultTy);

  auto *closureTy = cs.createTypeVariable(closureLoc, /*options=*/0);
  cs.setType(closure, closureTy);

  cs.addUnsolvedConstraint(Constraint::create(
      cs, ConstraintKind::FallbackType, closureTy, defaultTy,
      cs.getConstraintLocator(closure), /*referencedVars=*/{}));

  auto contextualTy =
      FunctionType::get({FunctionType::Param(getStdlibType("Int"))},
                        Context.TheEmptyTupleType, extInfo);

  cs.resolveClosure(closureTy, contextualTy, closureLoc);

  auto &graph = cs.getConstraintGraph();

  for (const auto &component :
       graph.computeConnectedComponents(cs.getTypeVariables())) {
    for (auto *constraint : component.getConstraints()) {
      if (constraint->getKind() != ConstraintKind::Disjunction)
        continue;

      ASSERT_TRUE(constraint->getLocator()
                      ->isLastElement<LocatorPathElt::ConstructorMember>());

      for (auto *choice : constraint->getNestedConstraints())
        ASSERT_EQ(choice->getDeclContext(), closure);
    }
  }
}
