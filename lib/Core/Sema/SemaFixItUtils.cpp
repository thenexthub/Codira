//===--- SemaFixItUtils.cpp - Sema FixIts ---------------------------------===//
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
//  This file defines helper classes for generation of Sema FixItHints.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/ExprObjC.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Sema/Sema.h"
#include "language/Core/Sema/SemaFixItUtils.h"

using namespace language::Core;

bool ConversionFixItGenerator::compareTypesSimple(CanQualType From,
                                                  CanQualType To,
                                                  Sema &S,
                                                  SourceLocation Loc,
                                                  ExprValueKind FromVK) {
  if (!To.isAtLeastAsQualifiedAs(From, S.getASTContext()))
    return false;

  From = From.getNonReferenceType();
  To = To.getNonReferenceType();

  // If both are pointer types, work with the pointee types.
  if (isa<PointerType>(From) && isa<PointerType>(To)) {
    From = S.Context.getCanonicalType(
        (cast<PointerType>(From))->getPointeeType());
    To = S.Context.getCanonicalType(
        (cast<PointerType>(To))->getPointeeType());
  }

  const CanQualType FromUnq = From.getUnqualifiedType();
  const CanQualType ToUnq = To.getUnqualifiedType();

  if ((FromUnq == ToUnq || (S.IsDerivedFrom(Loc, FromUnq, ToUnq))) &&
      To.isAtLeastAsQualifiedAs(From, S.getASTContext()))
    return true;
  return false;
}

bool ConversionFixItGenerator::tryToFixConversion(const Expr *FullExpr,
                                                  const QualType FromTy,
                                                  const QualType ToTy,
                                                  Sema &S) {
  if (!FullExpr)
    return false;

  const CanQualType FromQTy = S.Context.getCanonicalType(FromTy);
  const CanQualType ToQTy = S.Context.getCanonicalType(ToTy);
  const SourceLocation Begin = FullExpr->getSourceRange().getBegin();
  const SourceLocation End = S.getLocForEndOfToken(FullExpr->getSourceRange()
                                                   .getEnd());

  // Strip the implicit casts - those are implied by the compiler, not the
  // original source code.
  const Expr* Expr = FullExpr->IgnoreImpCasts();

  bool NeedParen = true;
  if (isa<ArraySubscriptExpr>(Expr) ||
      isa<CallExpr>(Expr) ||
      isa<DeclRefExpr>(Expr) ||
      isa<CastExpr>(Expr) ||
      isa<CXXNewExpr>(Expr) ||
      isa<CXXConstructExpr>(Expr) ||
      isa<CXXDeleteExpr>(Expr) ||
      isa<CXXNoexceptExpr>(Expr) ||
      isa<CXXPseudoDestructorExpr>(Expr) ||
      isa<CXXScalarValueInitExpr>(Expr) ||
      isa<CXXThisExpr>(Expr) ||
      isa<CXXTypeidExpr>(Expr) ||
      isa<CXXUnresolvedConstructExpr>(Expr) ||
      isa<ObjCMessageExpr>(Expr) ||
      isa<ObjCPropertyRefExpr>(Expr) ||
      isa<ObjCProtocolExpr>(Expr) ||
      isa<MemberExpr>(Expr) ||
      isa<ParenExpr>(FullExpr) ||
      isa<ParenListExpr>(Expr) ||
      isa<SizeOfPackExpr>(Expr) ||
      isa<UnaryOperator>(Expr))
    NeedParen = false;

  // Check if the argument needs to be dereferenced:
  //   (type * -> type) or (type * -> type &).
  if (const PointerType *FromPtrTy = dyn_cast<PointerType>(FromQTy)) {
    OverloadFixItKind FixKind = OFIK_Dereference;

    bool CanConvert = CompareTypes(
      S.Context.getCanonicalType(FromPtrTy->getPointeeType()), ToQTy,
                                 S, Begin, VK_LValue);
    if (CanConvert) {
      // Do not suggest dereferencing a Null pointer.
      if (Expr->IgnoreParenCasts()->
          isNullPointerConstant(S.Context, Expr::NPC_ValueDependentIsNotNull))
        return false;

      if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(Expr)) {
        if (UO->getOpcode() == UO_AddrOf) {
          FixKind = OFIK_RemoveTakeAddress;
          Hints.push_back(FixItHint::CreateRemoval(
                            CharSourceRange::getTokenRange(Begin, Begin)));
        }
      } else if (NeedParen) {
        Hints.push_back(FixItHint::CreateInsertion(Begin, "*("));
        Hints.push_back(FixItHint::CreateInsertion(End, ")"));
      } else {
        Hints.push_back(FixItHint::CreateInsertion(Begin, "*"));
      }

      NumConversionsFixed++;
      if (NumConversionsFixed == 1)
        Kind = FixKind;
      return true;
    }
  }

  // Check if the pointer to the argument needs to be passed:
  //   (type -> type *) or (type & -> type *).
  if (const auto *ToPtrTy = dyn_cast<PointerType>(ToQTy)) {
    bool CanConvert = false;
    OverloadFixItKind FixKind = OFIK_TakeAddress;

    // Only suggest taking address of L-values.
    if (!Expr->isLValue() || Expr->getObjectKind() != OK_Ordinary)
      return false;

    // Do no take address of const pointer to get void*
    if (isa<PointerType>(FromQTy) && ToPtrTy->isVoidPointerType())
      return false;

    CanConvert = CompareTypes(S.Context.getPointerType(FromQTy), ToQTy, S,
                              Begin, VK_PRValue);
    if (CanConvert) {

      if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(Expr)) {
        if (UO->getOpcode() == UO_Deref) {
          FixKind = OFIK_RemoveDereference;
          Hints.push_back(FixItHint::CreateRemoval(
                            CharSourceRange::getTokenRange(Begin, Begin)));
        }
      } else if (NeedParen) {
        Hints.push_back(FixItHint::CreateInsertion(Begin, "&("));
        Hints.push_back(FixItHint::CreateInsertion(End, ")"));
      } else {
        Hints.push_back(FixItHint::CreateInsertion(Begin, "&"));
      }

      NumConversionsFixed++;
      if (NumConversionsFixed == 1)
        Kind = FixKind;
      return true;
    }
  }

  return false;
}

static bool isMacroDefined(const Sema &S, SourceLocation Loc, StringRef Name) {
  return (bool)S.PP.getMacroDefinitionAtLoc(&S.getASTContext().Idents.get(Name),
                                            Loc);
}

static std::string getScalarZeroExpressionForType(
    const Type &T, SourceLocation Loc, const Sema &S) {
  assert(T.isScalarType() && "use scalar types only");
  // Suggest "0" for non-enumeration scalar types, unless we can find a
  // better initializer.
  if (T.isEnumeralType())
    return std::string();
  if ((T.isObjCObjectPointerType() || T.isBlockPointerType()) &&
      isMacroDefined(S, Loc, "nil"))
    return "nil";
  if (T.isRealFloatingType())
    return "0.0";
  if (T.isBooleanType() &&
      (S.LangOpts.CPlusPlus || isMacroDefined(S, Loc, "false")))
    return "false";
  if (T.isPointerType() || T.isMemberPointerType()) {
    if (S.LangOpts.CPlusPlus11)
      return "nullptr";
    if (isMacroDefined(S, Loc, "NULL"))
      return "NULL";
  }
  if (T.isCharType())
    return "'\\0'";
  if (T.isWideCharType())
    return "L'\\0'";
  if (T.isChar16Type())
    return "u'\\0'";
  if (T.isChar32Type())
    return "U'\\0'";
  return "0";
}

std::string
Sema::getFixItZeroInitializerForType(QualType T, SourceLocation Loc) const {
  if (T->isScalarType()) {
    std::string s = getScalarZeroExpressionForType(*T, Loc, *this);
    if (!s.empty())
      s = " = " + s;
    return s;
  }

  const CXXRecordDecl *RD = T->getAsCXXRecordDecl();
  if (!RD || !RD->hasDefinition())
    return std::string();
  if (LangOpts.CPlusPlus11 && !RD->hasUserProvidedDefaultConstructor())
    return "{}";
  if (RD->isAggregate())
    return " = {}";
  return std::string();
}

std::string
Sema::getFixItZeroLiteralForType(QualType T, SourceLocation Loc) const {
  return getScalarZeroExpressionForType(*T, Loc, *this);
}
