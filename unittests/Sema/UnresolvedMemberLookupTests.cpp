//===--- UnresolvedMemberLookupTests.cpp --------------------------------===//
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
#include "language/Sema/ConstraintSystem.h"

using namespace language;
using namespace language::unittest;
using namespace language::constraints;

TEST_F(SemaTest, TestLookupAlwaysLooksThroughOptionalBase) {
  auto *intTypeDecl = getStdlibNominalTypeDecl("Int");
  auto *optTypeDecl = getStdlibNominalTypeDecl("Optional");
  auto intType = intTypeDecl->getDeclaredType();
  auto intOptType = OptionalType::get(intType);
  auto stringType = getStdlibType("String");

  auto *intMember = addExtensionVarMember(intTypeDecl, "test", intOptType);
  addExtensionVarMember(optTypeDecl, "test", stringType);

  auto *UME = new (Context)
      UnresolvedMemberExpr(SourceLoc(), DeclNameLoc(),
                           DeclNameRef(Context.getIdentifier("test")), true);
  auto *UMCRE = new (Context) UnresolvedMemberChainResultExpr(UME, UME);

  ConstraintSystem cs(DC, ConstraintSystemOptions());
  auto *expr = cs.generateConstraints(UMCRE, DC);
  ASSERT_TRUE(expr);

  cs.addConstraint(
      ConstraintKind::Conversion, cs.getType(expr), intOptType,
      cs.getConstraintLocator(UMCRE, ConstraintLocator::ContextualType));
  SmallVector<Solution, 2> solutions;
  cs.solve(solutions);

  // We should have a solution.
  ASSERT_EQ(solutions.size(), 1u);

  auto &solution = solutions[0];
  auto *locator = cs.getConstraintLocator(UME,
                                          ConstraintLocator::UnresolvedMember);
  auto choice = solution.getOverloadChoice(locator).choice;

  // The `test` member on `Int` should be selected.
  ASSERT_EQ(choice.getDecl(), intMember);
}

TEST_F(SemaTest, TestLookupPrefersResultsOnOptionalRatherThanBase) {
  auto *intTypeDecl = getStdlibNominalTypeDecl("Int");
  auto *optTypeDecl = getStdlibNominalTypeDecl("Optional");
  auto intType = intTypeDecl->getDeclaredType();
  auto intOptType = OptionalType::get(intType);

  addExtensionVarMember(intTypeDecl, "test", intOptType);
  auto *optMember = addExtensionVarMember(optTypeDecl, "test", intType);

  auto *UME = new (Context)
      UnresolvedMemberExpr(SourceLoc(), DeclNameLoc(),
                           DeclNameRef(Context.getIdentifier("test")), true);
  auto *UMCRE = new (Context) UnresolvedMemberChainResultExpr(UME, UME);

  ConstraintSystem cs(DC, ConstraintSystemOptions());
  auto *expr = cs.generateConstraints(UMCRE, DC);
  ASSERT_TRUE(expr);

  cs.addConstraint(
      ConstraintKind::Conversion, cs.getType(expr), intOptType,
      cs.getConstraintLocator(expr, ConstraintLocator::ContextualType));
  SmallVector<Solution, 2> solutions;
  cs.solve(solutions);

  // We should have a solution.
  ASSERT_EQ(solutions.size(), 1u);

  auto &solution = solutions[0];
  auto *locator = cs.getConstraintLocator(UME,
                                          ConstraintLocator::UnresolvedMember);
  auto choice = solution.getOverloadChoice(locator).choice;
  auto score = solution.getFixedScore();

  // The `test` member on `Optional` should be chosen over the member on `Int`,
  // even though the score is otherwise worse.
  ASSERT_EQ(score.Data[SK_ValueToOptional], 1u);
  ASSERT_EQ(choice.getDecl(), optMember);
}
