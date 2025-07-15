//===--- Bridging/PatternBridging.cpp -------------------------------------===//
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

#include "language/AST/ASTBridging.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Expr.h"
#include "language/AST/Identifier.h"
#include "language/AST/Pattern.h"
#include "language/AST/TypeRepr.h"
#include "language/Basic/Assertions.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: Patterns
//===----------------------------------------------------------------------===//

// Define `.asPattern` on each BridgedXXXPattern type.
#define PATTERN(Id, Parent)                                                    \
  BridgedPattern Bridged##Id##Pattern_asPattern(                               \
      Bridged##Id##Pattern pattern) {                                          \
    return static_cast<Pattern *>(pattern.unbridged());                        \
  }
#include "language/AST/PatternNodes.def"

BridgedNullableVarDecl BridgedPattern_getSingleVar(BridgedPattern cPattern) {
  return cPattern.unbridged()->getSingleVar();
}

BridgedAnyPattern BridgedAnyPattern_createParsed(BridgedASTContext cContext,
                                                 BridgedSourceLoc cLoc) {
  return new (cContext.unbridged()) AnyPattern(cLoc.unbridged());
}

BridgedAnyPattern BridgedAnyPattern_createImplicit(BridgedASTContext cContext) {
  return AnyPattern::createImplicit(cContext.unbridged());
}

BridgedBindingPattern
BridgedBindingPattern_createParsed(BridgedASTContext cContext,
                                   BridgedSourceLoc cKeywordLoc, bool isLet,
                                   BridgedPattern cSubPattern) {
  VarDecl::Introducer introducer =
      isLet ? VarDecl::Introducer::Let : VarDecl::Introducer::Var;
  return BindingPattern::createParsed(cContext.unbridged(),
                                      cKeywordLoc.unbridged(), introducer,
                                      cSubPattern.unbridged());
}

BridgedBindingPattern
BridgedBindingPattern_createImplicitCatch(BridgedDeclContext cDeclContext,
                                          BridgedSourceLoc cLoc) {
  return BindingPattern::createImplicitCatch(cDeclContext.unbridged(),
                                             cLoc.unbridged());
}

BridgedExprPattern
BridgedExprPattern_createParsed(BridgedDeclContext cDeclContext,
                                BridgedExpr cExpr) {
  auto *DC = cDeclContext.unbridged();
  auto &context = DC->getASTContext();
  return ExprPattern::createParsed(context, cExpr.unbridged(), DC);
}

BridgedIsPattern BridgedIsPattern_createParsed(BridgedASTContext cContext,
                                               BridgedSourceLoc cIsLoc,
                                               BridgedTypeExpr cTypeExpr) {
  return new (cContext.unbridged())
      IsPattern(cIsLoc.unbridged(), cTypeExpr.unbridged(),
                /*subPattern=*/nullptr, CheckedCastKind::Unresolved);
}

BridgedNamedPattern
BridgedNamedPattern_createParsed(BridgedASTContext cContext,
                                 BridgedDeclContext cDeclContext,
                                 BridgedIdentifier name, BridgedSourceLoc loc) {
  auto &context = cContext.unbridged();
  auto *dc = cDeclContext.unbridged();

  // Note 'isStatic' and the introducer value are temporary.
  // The outer context should set the correct values.
  auto *varDecl = new (context) VarDecl(
      /*isStatic=*/false, VarDecl::Introducer::Let, loc.unbridged(),
      name.unbridged(), dc);
  auto *pattern = new (context) NamedPattern(varDecl);
  return pattern;
}

BridgedParenPattern BridgedParenPattern_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cLParenLoc,
    BridgedPattern cSubPattern, BridgedSourceLoc cRParenLoc) {
  return new (cContext.unbridged()) ParenPattern(
      cLParenLoc.unbridged(), cSubPattern.unbridged(), cRParenLoc.unbridged());
}

BridgedTuplePattern BridgedTuplePattern_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cLParenLoc,
    BridgedArrayRef cElements, BridgedSourceLoc cRParenLoc) {
  ASTContext &context = cContext.unbridged();
  toolchain::SmallVector<TuplePatternElt, 4> elements;
  elements.reserve(cElements.Length);
  toolchain::transform(cElements.unbridged<BridgedTuplePatternElt>(),
                  std::back_inserter(elements),
                  [](const BridgedTuplePatternElt &elt) {
                    return TuplePatternElt(elt.Label.unbridged(),
                                           elt.LabelLoc.unbridged(),
                                           elt.ThePattern.unbridged());
                  });

  return TuplePattern::create(context, cLParenLoc.unbridged(), elements,
                              cRParenLoc.unbridged());
}

BridgedTypedPattern BridgedTypedPattern_createParsed(BridgedASTContext cContext,
                                                     BridgedPattern cPattern,
                                                     BridgedTypeRepr cType) {
  return new (cContext.unbridged())
      TypedPattern(cPattern.unbridged(), cType.unbridged());
}

BridgedTypedPattern
BridgedTypedPattern_createPropagated(BridgedASTContext cContext,
                                     BridgedPattern cPattern,
                                     BridgedTypeRepr cType) {
  return TypedPattern::createPropagated(
      cContext.unbridged(), cPattern.unbridged(), cType.unbridged());
}

void BridgedPattern_setImplicit(BridgedPattern cPattern) {
  cPattern.unbridged()->setImplicit();
}

BridgedIdentifier BridgedPattern_getBoundName(BridgedPattern cPattern) {
  return cPattern.unbridged()->getBoundName();
}
