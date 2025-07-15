//===--- PlaceholderTypeInferenceTests.cpp --------------------------------===//
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

TEST_F(SemaTest, TestPlaceholderInferenceForArrayLiteral) {
  auto *intTypeDecl = getStdlibNominalTypeDecl("Int");

  auto *intLiteral = new (Context) IntegerLiteralExpr("0", SourceLoc(), true);
  auto *arrayExpr = ArrayExpr::create(Context, SourceLoc(), {intLiteral}, {}, SourceLoc());

  auto *placeholderRepr = new (Context) PlaceholderTypeRepr(SourceLoc());
  auto *arrayRepr = new (Context) ArrayTypeRepr(placeholderRepr, SourceRange());
  auto placeholderTy = PlaceholderType::get(Context, placeholderRepr);
  auto *arrayTy = ArraySliceType::get(placeholderTy);

  auto *varDecl = new (Context) VarDecl(false, VarDecl::Introducer::Let, SourceLoc(), Context.getIdentifier("x"), DC);
  auto *namedPattern = new (Context) NamedPattern(varDecl);
  auto *typedPattern = new (Context) TypedPattern(namedPattern, arrayRepr);

  auto target = SyntacticElementTarget::forInitialization(
      arrayExpr, DC, arrayTy, typedPattern, /*bindPatternVarsOneWay=*/false);

  ConstraintSystem cs(DC, ConstraintSystemOptions());
  ContextualTypeInfo contextualInfo({arrayRepr, arrayTy}, CTP_Initialization);
  cs.setContextualInfo(arrayExpr, contextualInfo);
  auto failed = cs.generateConstraints(target, FreeTypeVariableBinding::Disallow);
  ASSERT_FALSE(failed);

  SmallVector<Solution, 2> solutions;
  cs.solve(solutions);

  // We should have a solution.
  ASSERT_EQ(solutions.size(), 1u);

  auto &solution = solutions[0];

  auto eltTy = solution.simplifyType(solution.getType(arrayExpr))->getArrayElementType();
  ASSERT_TRUE(eltTy);
  ASSERT_TRUE(eltTy->is<StructType>());
  ASSERT_EQ(eltTy->getAs<StructType>()->getDecl(), intTypeDecl);
}

TEST_F(SemaTest, TestPlaceholderInferenceForDictionaryLiteral) {
  auto *intTypeDecl = getStdlibNominalTypeDecl("Int");
  auto *stringTypeDecl = getStdlibNominalTypeDecl("String");

  auto *intLiteral = new (Context) IntegerLiteralExpr("0", SourceLoc(), true);
  auto *stringLiteral = new (Context) StringLiteralExpr("test", SourceRange(), true);
  auto *kvTupleExpr = TupleExpr::create(Context, SourceLoc(), {stringLiteral, intLiteral}, {}, {}, SourceLoc(), true);
  auto *dictExpr = DictionaryExpr::create(Context, SourceLoc(), {kvTupleExpr}, {}, SourceLoc());

  auto *keyPlaceholderRepr = new (Context) PlaceholderTypeRepr(SourceLoc());
  auto *valPlaceholderRepr = new (Context) PlaceholderTypeRepr(SourceLoc());
  auto *dictRepr = new (Context) DictionaryTypeRepr(keyPlaceholderRepr, valPlaceholderRepr, SourceLoc(), SourceRange());
  auto keyPlaceholderTy = PlaceholderType::get(Context, keyPlaceholderRepr);
  auto valPlaceholderTy = PlaceholderType::get(Context, valPlaceholderRepr);
  auto *dictTy = DictionaryType::get(keyPlaceholderTy, valPlaceholderTy);

  auto *varDecl = new (Context) VarDecl(false, VarDecl::Introducer::Let, SourceLoc(), Context.getIdentifier("x"), DC);
  auto *namedPattern = new (Context) NamedPattern(varDecl);
  auto *typedPattern = new (Context) TypedPattern(namedPattern, dictRepr);

  auto target = SyntacticElementTarget::forInitialization(
      dictExpr, DC, dictTy, typedPattern, /*bindPatternVarsOneWay=*/false);

  ConstraintSystem cs(DC, ConstraintSystemOptions());
  ContextualTypeInfo contextualInfo({dictRepr, dictTy}, CTP_Initialization);
  cs.setContextualInfo(dictExpr, contextualInfo);
  auto failed = cs.generateConstraints(target, FreeTypeVariableBinding::Disallow);
  ASSERT_FALSE(failed);

  SmallVector<Solution, 2> solutions;
  cs.solve(solutions);

  // We should have a solution.
  ASSERT_EQ(solutions.size(), 1u);

  auto &solution = solutions[0];

  auto keyValTys = ConstraintSystem::isDictionaryType(solution.simplifyType(solution.getType(dictExpr)));
  ASSERT_TRUE(keyValTys.has_value());

  Type keyTy;
  Type valTy;
  std::tie(keyTy, valTy) = *keyValTys;
  ASSERT_TRUE(keyTy->is<StructType>());
  ASSERT_EQ(keyTy->getAs<StructType>()->getDecl(), stringTypeDecl);

  ASSERT_TRUE(valTy->is<StructType>());
  ASSERT_EQ(valTy->getAs<StructType>()->getDecl(), intTypeDecl);
}
