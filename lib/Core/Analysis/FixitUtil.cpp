//===- FixitUtil.cpp ------------------------------------------------------===//
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

#include "language/Core/Analysis/Support/FixitUtil.h"
#include "language/Core/ASTMatchers/ASTMatchers.h"

using namespace toolchain;
using namespace language::Core;
using namespace ast_matchers;

// Returns the text of the pointee type of `T` from a `VarDecl` of a pointer
// type. The text is obtained through from `TypeLoc`s.  Since `TypeLoc` does not
// have source ranges of qualifiers ( The `QualTypeLoc` looks hacky too me
// :( ), `Qualifiers` of the pointee type is returned separately through the
// output parameter `QualifiersToAppend`.
std::optional<std::string>
language::Core::getPointeeTypeText(const DeclaratorDecl *VD, const SourceManager &SM,
                          const LangOptions &LangOpts,
                          std::optional<Qualifiers> *QualifiersToAppend) {
  QualType Ty = VD->getType();
  QualType PteTy;

  assert(Ty->isPointerType() && !Ty->isFunctionPointerType() &&
         "Expecting a VarDecl of type of pointer to object type");
  PteTy = Ty->getPointeeType();

  TypeLoc TyLoc = VD->getTypeSourceInfo()->getTypeLoc().getUnqualifiedLoc();
  TypeLoc PteTyLoc;

  // We only deal with the cases that we know `TypeLoc::getNextTypeLoc` returns
  // the `TypeLoc` of the pointee type:
  switch (TyLoc.getTypeLocClass()) {
  case TypeLoc::ConstantArray:
  case TypeLoc::IncompleteArray:
  case TypeLoc::VariableArray:
  case TypeLoc::DependentSizedArray:
  case TypeLoc::Decayed:
    assert(isa<ParmVarDecl>(VD) && "An array type shall not be treated as a "
                                   "pointer type unless it decays.");
    PteTyLoc = TyLoc.getNextTypeLoc();
    break;
  case TypeLoc::Pointer:
    PteTyLoc = TyLoc.castAs<PointerTypeLoc>().getPointeeLoc();
    break;
  default:
    return std::nullopt;
  }
  if (PteTyLoc.isNull())
    // Sometimes we cannot get a useful `TypeLoc` for the pointee type, e.g.,
    // when the pointer type is `auto`.
    return std::nullopt;

  // TODO check
  SourceLocation IdentLoc = VD->getLocation();

  if (!(IdentLoc.isValid() && PteTyLoc.getSourceRange().isValid())) {
    // We are expecting these locations to be valid. But in some cases, they are
    // not all valid. It is a Clang bug to me and we are not responsible for
    // fixing it.  So we will just give up for now when it happens.
    return std::nullopt;
  }

  // Note that TypeLoc.getEndLoc() returns the begin location of the last token:
  SourceLocation PteEndOfTokenLoc =
      Lexer::getLocForEndOfToken(PteTyLoc.getEndLoc(), 0, SM, LangOpts);

  if (!PteEndOfTokenLoc.isValid())
    // Sometimes we cannot get the end location of the pointee type, e.g., when
    // there are macros involved.
    return std::nullopt;
  if (!SM.isBeforeInTranslationUnit(PteEndOfTokenLoc, IdentLoc) &&
      PteEndOfTokenLoc != IdentLoc) {
    // We only deal with the cases where the source text of the pointee type
    // appears on the left-hand side of the variable identifier completely,
    // including the following forms:
    // `T ident`,
    // `T ident[]`, where `T` is any type.
    // Examples of excluded cases are `T (*ident)[]` or `T ident[][n]`.
    return std::nullopt;
  }
  if (PteTy.hasQualifiers()) {
    // TypeLoc does not provide source ranges for qualifiers (it says it's
    // intentional but seems fishy to me), so we cannot get the full text
    // `PteTy` via source ranges.
    *QualifiersToAppend = PteTy.getQualifiers();
  }
  return getRangeText({PteTyLoc.getBeginLoc(), PteEndOfTokenLoc}, SM, LangOpts)
      ->str();
}

// returns text of pointee to pointee (T*&)
std::optional<std::string>
getPointee2TypeText(const DeclaratorDecl *VD, const SourceManager &SM,
                    const LangOptions &LangOpts,
                    std::optional<Qualifiers> *QualifiersToAppend) {

  QualType Ty = VD->getType();
  assert(Ty->isReferenceType() &&
         "Expecting a VarDecl of reference to pointer type");

  Ty = Ty->getPointeeType();
  QualType PteTy;

  assert(Ty->isPointerType() && !Ty->isFunctionPointerType() &&
         "Expecting a VarDecl of type of pointer to object type");
  PteTy = Ty->getPointeeType();

  TypeLoc TyLoc = VD->getTypeSourceInfo()->getTypeLoc().getUnqualifiedLoc();
  TypeLoc PtrTyLoc;
  TypeLoc PteTyLoc;

  // We only deal with the cases that we know `TypeLoc::getNextTypeLoc` returns
  // the `TypeLoc` of the pointee type:
  switch (TyLoc.getTypeLocClass()) {
  case TypeLoc::ConstantArray:
  case TypeLoc::IncompleteArray:
  case TypeLoc::VariableArray:
  case TypeLoc::DependentSizedArray:
  case TypeLoc::LValueReference:
    PtrTyLoc = TyLoc.castAs<ReferenceTypeLoc>().getPointeeLoc();
    if (PtrTyLoc.getTypeLocClass() == TypeLoc::Pointer) {
      PteTyLoc = PtrTyLoc.castAs<PointerTypeLoc>().getPointeeLoc();
      break;
    }
    return std::nullopt;
    break;
  default:
    return std::nullopt;
  }
  if (PteTyLoc.isNull())
    // Sometimes we cannot get a useful `TypeLoc` for the pointee type, e.g.,
    // when the pointer type is `auto`.
    return std::nullopt;

  // TODO make sure this works
  SourceLocation IdentLoc = VD->getLocation();

  if (!(IdentLoc.isValid() && PteTyLoc.getSourceRange().isValid())) {
    // We are expecting these locations to be valid. But in some cases, they are
    // not all valid. It is a Clang bug to me and we are not responsible for
    // fixing it.  So we will just give up for now when it happens.
    return std::nullopt;
  }

  // Note that TypeLoc.getEndLoc() returns the begin location of the last token:
  SourceLocation PteEndOfTokenLoc =
      Lexer::getLocForEndOfToken(PteTyLoc.getEndLoc(), 0, SM, LangOpts);

  if (!PteEndOfTokenLoc.isValid())
    // Sometimes we cannot get the end location of the pointee type, e.g., when
    // there are macros involved.
    return std::nullopt;
  if (!SM.isBeforeInTranslationUnit(PteEndOfTokenLoc, IdentLoc)) {
    // We only deal with the cases where the source text of the pointee type
    // appears on the left-hand side of the variable identifier completely,
    // including the following forms:
    // `T ident`,
    // `T ident[]`, where `T` is any type.
    // Examples of excluded cases are `T (*ident)[]` or `T ident[][n]`.
    return std::nullopt;
  }
  if (PteTy.hasQualifiers()) {
    // TypeLoc does not provide source ranges for qualifiers (it says it's
    // intentional but seems fishy to me), so we cannot get the full text
    // `PteTy` via source ranges.
    *QualifiersToAppend = PteTy.getQualifiers();
  }
  return getRangeText({PteTyLoc.getBeginLoc(), PteEndOfTokenLoc}, SM, LangOpts)
      ->str();
}

SourceLocation language::Core::getBeginLocOfNestedIdentifier(const DeclaratorDecl *D) {
  if (D->getQualifier()) {
    return D->getQualifierLoc().getBeginLoc();
  }
  return getVarDeclIdentifierLoc(D);
}

// Returns the literal text in `SourceRange SR`, if `SR` is a valid range.
std::optional<StringRef> language::Core::getRangeText(SourceRange SR,
                                             const SourceManager &SM,
                                             const LangOptions &LangOpts) {
  bool Invalid = false;
  CharSourceRange CSR = CharSourceRange::getCharRange(SR);
  StringRef Text = Lexer::getSourceText(CSR, SM, LangOpts, &Invalid);

  if (!Invalid)
    return Text;
  return std::nullopt;
}

// Returns the literal text of the identifier of the given variable declaration.
std::optional<StringRef>
language::Core::getVarDeclIdentifierText(const DeclaratorDecl *VD,
                                const SourceManager &SM,
                                const LangOptions &LangOpts) {
  SourceLocation ParmIdentBeginLoc = getBeginLocOfNestedIdentifier(VD);
  SourceLocation ParmIdentEndLoc =
      Lexer::getLocForEndOfToken(getVarDeclIdentifierLoc(VD), 0, SM, LangOpts);

  if (VD->getQualifier()) {
    ParmIdentBeginLoc = VD->getQualifierLoc().getBeginLoc();
  }

  if (ParmIdentEndLoc.isMacroID() &&
      !Lexer::isAtEndOfMacroExpansion(ParmIdentEndLoc, SM, LangOpts))
    return std::nullopt;
  return getRangeText({ParmIdentBeginLoc, ParmIdentEndLoc}, SM, LangOpts);
}

// Return text representation of an `Expr`.
std::optional<StringRef> language::Core::getExprText(const Expr *E,
                                            const SourceManager &SM,
                                            const LangOptions &LangOpts) {
  std::optional<SourceLocation> LastCharLoc = getPastLoc(E, SM, LangOpts);

  if (LastCharLoc)
    return Lexer::getSourceText(
        CharSourceRange::getCharRange(E->getBeginLoc(), *LastCharLoc), SM,
        LangOpts);

  return std::nullopt;
}

// Returns the begin location of the identifier of the given variable
// declaration.
SourceLocation language::Core::getVarDeclIdentifierLoc(const DeclaratorDecl *VD) {
  // According to the implementation of `VarDecl`, `VD->getLocation()` actually
  // returns the begin location of the identifier of the declaration:
  return VD->getLocation();
}
