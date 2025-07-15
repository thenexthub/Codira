//===--- FreestandingMacroExpansion.cpp -----------------------------------===//
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

#include "language/AST/FreestandingMacroExpansion.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/MacroDiscriminatorContext.h"
#include "language/Basic/Assertions.h"

using namespace language;

SourceRange MacroExpansionInfo::getSourceRange() const {
  SourceLoc endLoc;
  if (ArgList && !ArgList->isImplicit())
    endLoc = ArgList->getEndLoc();
  else if (RightAngleLoc.isValid())
    endLoc = RightAngleLoc;
  else
    endLoc = MacroNameLoc.getEndLoc();

  return SourceRange(SigilLoc, endLoc);
}

#define FORWARD_VARIANT(NAME)                                                  \
  switch (getFreestandingMacroKind()) {                                        \
  case FreestandingMacroKind::Expr:                                            \
    return cast<MacroExpansionExpr>(this)->NAME();                             \
  case FreestandingMacroKind::Decl:                                            \
    return cast<MacroExpansionDecl>(this)->NAME();                             \
  }

DeclContext *FreestandingMacroExpansion::getDeclContext() const {
  FORWARD_VARIANT(getDeclContext);
}
SourceRange FreestandingMacroExpansion::getSourceRange() const {
  FORWARD_VARIANT(getSourceRange);
}
unsigned FreestandingMacroExpansion::getDiscriminator() const {
  auto info = getExpansionInfo();
  if (info->Discriminator != MacroExpansionInfo::InvalidDiscriminator)
    return info->Discriminator;

  auto mutableThis = const_cast<FreestandingMacroExpansion *>(this);
  auto dc = getDeclContext();
  ASTContext &ctx = dc->getASTContext();
  auto discriminatorContext =
      MacroDiscriminatorContext::getParentOf(mutableThis);
  info->Discriminator = ctx.getNextMacroDiscriminator(
      discriminatorContext, getMacroName().getBaseName());

  assert(info->Discriminator != MacroExpansionInfo::InvalidDiscriminator);
  return info->Discriminator;
}

unsigned FreestandingMacroExpansion::getRawDiscriminator() const {
  return getExpansionInfo()->Discriminator;
}

ASTNode FreestandingMacroExpansion::getASTNode() {
  switch (getFreestandingMacroKind()) {
  case FreestandingMacroKind::Expr:
    return cast<MacroExpansionExpr>(this);
  case FreestandingMacroKind::Decl:
    return cast<MacroExpansionDecl>(this);
  }
}
