//===---- CheckerHelpers.cpp - Helper functions for checkers ----*- C++ -*-===//
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
//  This file defines several static functions for use in checkers.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerHelpers.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include <optional>

namespace language::Core {

namespace ento {

// Recursively find any substatements containing macros
bool containsMacro(const Stmt *S) {
  if (S->getBeginLoc().isMacroID())
    return true;

  if (S->getEndLoc().isMacroID())
    return true;

  for (const Stmt *Child : S->children())
    if (Child && containsMacro(Child))
      return true;

  return false;
}

// Recursively find any substatements containing enum constants
bool containsEnum(const Stmt *S) {
  const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(S);

  if (DR && isa<EnumConstantDecl>(DR->getDecl()))
    return true;

  for (const Stmt *Child : S->children())
    if (Child && containsEnum(Child))
      return true;

  return false;
}

// Recursively find any substatements containing static vars
bool containsStaticLocal(const Stmt *S) {
  const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(S);

  if (DR)
    if (const VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl()))
      if (VD->isStaticLocal())
        return true;

  for (const Stmt *Child : S->children())
    if (Child && containsStaticLocal(Child))
      return true;

  return false;
}

// Recursively find any substatements containing __builtin_offsetof
bool containsBuiltinOffsetOf(const Stmt *S) {
  if (isa<OffsetOfExpr>(S))
    return true;

  for (const Stmt *Child : S->children())
    if (Child && containsBuiltinOffsetOf(Child))
      return true;

  return false;
}

// Extract lhs and rhs from assignment statement
std::pair<const language::Core::VarDecl *, const language::Core::Expr *>
parseAssignment(const Stmt *S) {
  const VarDecl *VD = nullptr;
  const Expr *RHS = nullptr;

  if (auto Assign = dyn_cast_or_null<BinaryOperator>(S)) {
    if (Assign->isAssignmentOp()) {
      // Ordinary assignment
      RHS = Assign->getRHS();
      if (auto DE = dyn_cast_or_null<DeclRefExpr>(Assign->getLHS()))
        VD = dyn_cast_or_null<VarDecl>(DE->getDecl());
    }
  } else if (auto PD = dyn_cast_or_null<DeclStmt>(S)) {
    // Initialization
    assert(PD->isSingleDecl() && "We process decls one by one");
    VD = cast<VarDecl>(PD->getSingleDecl());
    RHS = VD->getAnyInitializer();
  }

  return std::make_pair(VD, RHS);
}

Nullability getNullabilityAnnotation(QualType Type) {
  const auto *AttrType = Type->getAs<AttributedType>();
  if (!AttrType)
    return Nullability::Unspecified;
  if (AttrType->getAttrKind() == attr::TypeNullable)
    return Nullability::Nullable;
  else if (AttrType->getAttrKind() == attr::TypeNonNull)
    return Nullability::Nonnull;
  return Nullability::Unspecified;
}

std::optional<int> tryExpandAsInteger(StringRef Macro, const Preprocessor &PP) {
  const auto *MacroII = PP.getIdentifierInfo(Macro);
  if (!MacroII)
    return std::nullopt;
  const MacroInfo *MI = PP.getMacroInfo(MacroII);
  if (!MI)
    return std::nullopt;

  // Filter out parens.
  std::vector<Token> FilteredTokens;
  FilteredTokens.reserve(MI->tokens().size());
  for (auto &T : MI->tokens())
    if (!T.isOneOf(tok::l_paren, tok::r_paren))
      FilteredTokens.push_back(T);

  // Parse an integer at the end of the macro definition.
  const Token &T = FilteredTokens.back();

  if (!T.isLiteral())
    return std::nullopt;

  bool InvalidSpelling = false;
  SmallVector<char> Buffer(T.getLength());
  // `Preprocessor::getSpelling` can get the spelling of the token regardless of
  // whether the macro is defined in a PCH or not:
  StringRef ValueStr = PP.getSpelling(T, Buffer, &InvalidSpelling);

  if (InvalidSpelling)
    return std::nullopt;

  toolchain::APInt IntValue;
  constexpr unsigned AutoSenseRadix = 0;
  if (ValueStr.getAsInteger(AutoSenseRadix, IntValue))
    return std::nullopt;

  // Parse an optional minus sign.
  size_t Size = FilteredTokens.size();
  if (Size >= 2) {
    if (FilteredTokens[Size - 2].is(tok::minus))
      IntValue = -IntValue;
  }

  return IntValue.getSExtValue();
}

OperatorKind operationKindFromOverloadedOperator(OverloadedOperatorKind OOK,
                                                 bool IsBinary) {
  toolchain::StringMap<BinaryOperatorKind> BinOps{
#define BINARY_OPERATION(Name, Spelling) {Spelling, BO_##Name},
#include "language/Core/AST/OperationKinds.def"
  };
  toolchain::StringMap<UnaryOperatorKind> UnOps{
#define UNARY_OPERATION(Name, Spelling) {Spelling, UO_##Name},
#include "language/Core/AST/OperationKinds.def"
  };

  switch (OOK) {
#define OVERLOADED_OPERATOR(Name, Spelling, Token, Unary, Binary, MemberOnly)  \
  case OO_##Name:                                                              \
    if (IsBinary) {                                                            \
      auto BinOpIt = BinOps.find(Spelling);                                    \
      if (BinOpIt != BinOps.end())                                             \
        return OperatorKind(BinOpIt->second);                                  \
      else                                                                     \
        toolchain_unreachable("operator was expected to be binary but is not");     \
    } else {                                                                   \
      auto UnOpIt = UnOps.find(Spelling);                                      \
      if (UnOpIt != UnOps.end())                                               \
        return OperatorKind(UnOpIt->second);                                   \
      else                                                                     \
        toolchain_unreachable("operator was expected to be unary but is not");      \
    }                                                                          \
    break;
#include "language/Core/Basic/OperatorKinds.def"
  default:
    toolchain_unreachable("unexpected operator kind");
  }
}

std::optional<SVal> getPointeeVal(SVal PtrSVal, ProgramStateRef State) {
  if (const auto *Ptr = PtrSVal.getAsRegion()) {
    return State->getSVal(Ptr);
  }
  return std::nullopt;
}

bool isWithinStdNamespace(const Decl *D) {
  const DeclContext *DC = D->getDeclContext();
  while (DC) {
    if (const auto *NS = dyn_cast<NamespaceDecl>(DC);
        NS && NS->isStdNamespace())
      return true;
    DC = DC->getParent();
  }
  return false;
}

} // namespace ento
} // namespace language::Core
