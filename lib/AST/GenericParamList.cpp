//===--- GenericParamList.cpp - Swift Language Decl ASTs ------------------===//
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
//
//  This file implements the GenericParamList class and related classes.
//
//===----------------------------------------------------------------------===//

#include "language/AST/GenericParamList.h"

#include "language/AST/ASTContext.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/TypeRepr.h"
#include "language/Basic/Assertions.h"

using namespace language;
SourceRange RequirementRepr::getSourceRange() const {
  if (getKind() == RequirementReprKind::LayoutConstraint)
    return SourceRange(FirstType->getSourceRange().Start,
                       SecondLayout.getSourceRange().End);
  return SourceRange(FirstType->getSourceRange().Start,
                     SecondType->getSourceRange().End);
}

GenericParamList::GenericParamList(SourceLoc LAngleLoc,
                                   ArrayRef<GenericTypeParamDecl *> Params,
                                   SourceLoc WhereLoc,
                                   MutableArrayRef<RequirementRepr> Requirements,
                                   SourceLoc RAngleLoc)
  : Brackets(LAngleLoc, RAngleLoc), NumParams(Params.size()),
    WhereLoc(WhereLoc), Requirements(Requirements),
    OuterParameters(nullptr)
{
  std::uninitialized_copy(Params.begin(), Params.end(),
                          getTrailingObjects<GenericTypeParamDecl *>());
}

GenericParamList *
GenericParamList::create(ASTContext &Context,
                         SourceLoc LAngleLoc,
                         ArrayRef<GenericTypeParamDecl *> Params,
                         SourceLoc RAngleLoc) {
  unsigned Size = totalSizeToAlloc<GenericTypeParamDecl *>(Params.size());
  void *Mem = Context.Allocate(Size, alignof(GenericParamList));
  return new (Mem) GenericParamList(LAngleLoc, Params, SourceLoc(),
                                    MutableArrayRef<RequirementRepr>(),
                                    RAngleLoc);
}

GenericParamList *
GenericParamList::create(const ASTContext &Context,
                         SourceLoc LAngleLoc,
                         ArrayRef<GenericTypeParamDecl *> Params,
                         SourceLoc WhereLoc,
                         ArrayRef<RequirementRepr> Requirements,
                         SourceLoc RAngleLoc) {
  unsigned Size = totalSizeToAlloc<GenericTypeParamDecl *>(Params.size());
  void *Mem = Context.Allocate(Size, alignof(GenericParamList));
  return new (Mem) GenericParamList(LAngleLoc, Params,
                                    WhereLoc,
                                    Context.AllocateCopy(Requirements),
                                    RAngleLoc);
}

GenericParamList *
GenericParamList::clone(DeclContext *dc) const {
  auto &ctx = dc->getASTContext();
  SmallVector<GenericTypeParamDecl *, 2> params;
  for (auto param : getParams()) {
    auto *newParam = GenericTypeParamDecl::createImplicit(
        dc, param->getName(), GenericTypeParamDecl::InvalidDepth,
        param->getIndex(), param->getParamKind(), param->getOpaqueTypeRepr());
    newParam->setInherited(param->getInherited().getEntries());

    // Cache the value type computed from the previous param to the new one.
    ctx.evaluator.cacheOutput(
        GenericTypeParamDeclGetValueTypeRequest{newParam},
        param->getValueType());

    params.push_back(newParam);
  }

  return GenericParamList::create(ctx, SourceLoc(), params, SourceLoc());
}

void GenericParamList::setDepth(unsigned depth) {
  for (auto param : *this)
    param->setDepth(depth);
}

void GenericParamList::setDeclContext(DeclContext *dc) {
  for (auto param : *this)
    param->setDeclContext(dc);
}

GenericTypeParamDecl *GenericParamList::lookUpGenericParam(
    Identifier name) const {
  for (const auto *innerParams = this;
       innerParams != nullptr;
       innerParams = innerParams->getOuterParameters()) {
    for (auto *paramDecl : *innerParams) {
      if (name == paramDecl->getName()) {
        return const_cast<GenericTypeParamDecl *>(paramDecl);
      }
    }
  }

  return nullptr;
}

TrailingWhereClause::TrailingWhereClause(
                       SourceLoc whereLoc, SourceLoc endLoc,
                       ArrayRef<RequirementRepr> requirements)
  : WhereLoc(whereLoc), EndLoc(endLoc),
    NumRequirements(requirements.size())
{
  std::uninitialized_copy(requirements.begin(), requirements.end(),
                          getTrailingObjects<RequirementRepr>());
}

TrailingWhereClause *TrailingWhereClause::create(
                       ASTContext &ctx,
                       SourceLoc whereLoc,
                       SourceLoc endLoc,
                       ArrayRef<RequirementRepr> requirements) {
  unsigned size = totalSizeToAlloc<RequirementRepr>(requirements.size());
  void *mem = ctx.Allocate(size, alignof(TrailingWhereClause));
  return new (mem) TrailingWhereClause(whereLoc, endLoc, requirements);
}
