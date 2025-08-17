//===--- DeclOpenMP.cpp - Declaration OpenMP AST Node Implementation ------===//
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
/// \file
/// This file implements OMPThreadPrivateDecl, OMPCapturedExprDecl
/// classes.
///
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/AST/DeclOpenMP.h"
#include "language/Core/AST/Expr.h"

using namespace language::Core;

//===----------------------------------------------------------------------===//
// OMPThreadPrivateDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPThreadPrivateDecl::anchor() {}

OMPThreadPrivateDecl *OMPThreadPrivateDecl::Create(ASTContext &C,
                                                   DeclContext *DC,
                                                   SourceLocation L,
                                                   ArrayRef<Expr *> VL) {
  auto *D = OMPDeclarativeDirective::createDirective<OMPThreadPrivateDecl>(
      C, DC, {}, VL.size(), L);
  D->setVars(VL);
  return D;
}

OMPThreadPrivateDecl *OMPThreadPrivateDecl::CreateDeserialized(ASTContext &C,
                                                               GlobalDeclID ID,
                                                               unsigned N) {
  return OMPDeclarativeDirective::createEmptyDirective<OMPThreadPrivateDecl>(
      C, ID, 0, N);
}

void OMPThreadPrivateDecl::setVars(ArrayRef<Expr *> VL) {
  assert(VL.size() == Data->getNumChildren() &&
         "Number of variables is not the same as the preallocated buffer");
  toolchain::copy(VL, getVars().begin());
}

//===----------------------------------------------------------------------===//
// OMPAllocateDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPAllocateDecl::anchor() { }

OMPAllocateDecl *OMPAllocateDecl::Create(ASTContext &C, DeclContext *DC,
                                         SourceLocation L, ArrayRef<Expr *> VL,
                                         ArrayRef<OMPClause *> CL) {
  auto *D = OMPDeclarativeDirective::createDirective<OMPAllocateDecl>(
      C, DC, CL, VL.size(), L);
  D->setVars(VL);
  return D;
}

OMPAllocateDecl *OMPAllocateDecl::CreateDeserialized(ASTContext &C,
                                                     GlobalDeclID ID,
                                                     unsigned NVars,
                                                     unsigned NClauses) {
  return OMPDeclarativeDirective::createEmptyDirective<OMPAllocateDecl>(
      C, ID, NClauses, NVars, SourceLocation());
}

void OMPAllocateDecl::setVars(ArrayRef<Expr *> VL) {
  assert(VL.size() == Data->getNumChildren() &&
         "Number of variables is not the same as the preallocated buffer");
  toolchain::copy(VL, getVars().begin());
}

//===----------------------------------------------------------------------===//
// OMPRequiresDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPRequiresDecl::anchor() {}

OMPRequiresDecl *OMPRequiresDecl::Create(ASTContext &C, DeclContext *DC,
                                         SourceLocation L,
                                         ArrayRef<OMPClause *> CL) {
  return OMPDeclarativeDirective::createDirective<OMPRequiresDecl>(C, DC, CL, 0,
                                                                   L);
}

OMPRequiresDecl *OMPRequiresDecl::CreateDeserialized(ASTContext &C,
                                                     GlobalDeclID ID,
                                                     unsigned N) {
  return OMPDeclarativeDirective::createEmptyDirective<OMPRequiresDecl>(
      C, ID, N, 0, SourceLocation());
}

//===----------------------------------------------------------------------===//
// OMPDeclareReductionDecl Implementation.
//===----------------------------------------------------------------------===//

OMPDeclareReductionDecl::OMPDeclareReductionDecl(
    Kind DK, DeclContext *DC, SourceLocation L, DeclarationName Name,
    QualType Ty, OMPDeclareReductionDecl *PrevDeclInScope)
    : ValueDecl(DK, DC, L, Name, Ty), DeclContext(DK), Combiner(nullptr),
      PrevDeclInScope(PrevDeclInScope) {
  setInitializer(nullptr, OMPDeclareReductionInitKind::Call);
}

void OMPDeclareReductionDecl::anchor() {}

OMPDeclareReductionDecl *OMPDeclareReductionDecl::Create(
    ASTContext &C, DeclContext *DC, SourceLocation L, DeclarationName Name,
    QualType T, OMPDeclareReductionDecl *PrevDeclInScope) {
  return new (C, DC) OMPDeclareReductionDecl(OMPDeclareReduction, DC, L, Name,
                                             T, PrevDeclInScope);
}

OMPDeclareReductionDecl *
OMPDeclareReductionDecl::CreateDeserialized(ASTContext &C, GlobalDeclID ID) {
  return new (C, ID) OMPDeclareReductionDecl(
      OMPDeclareReduction, /*DC=*/nullptr, SourceLocation(), DeclarationName(),
      QualType(), /*PrevDeclInScope=*/nullptr);
}

OMPDeclareReductionDecl *OMPDeclareReductionDecl::getPrevDeclInScope() {
  return cast_or_null<OMPDeclareReductionDecl>(
      PrevDeclInScope.get(getASTContext().getExternalSource()));
}
const OMPDeclareReductionDecl *
OMPDeclareReductionDecl::getPrevDeclInScope() const {
  return cast_or_null<OMPDeclareReductionDecl>(
      PrevDeclInScope.get(getASTContext().getExternalSource()));
}

//===----------------------------------------------------------------------===//
// OMPDeclareMapperDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPDeclareMapperDecl::anchor() {}

OMPDeclareMapperDecl *OMPDeclareMapperDecl::Create(
    ASTContext &C, DeclContext *DC, SourceLocation L, DeclarationName Name,
    QualType T, DeclarationName VarName, ArrayRef<OMPClause *> Clauses,
    OMPDeclareMapperDecl *PrevDeclInScope) {
  return OMPDeclarativeDirective::createDirective<OMPDeclareMapperDecl>(
      C, DC, Clauses, 1, L, Name, T, VarName, PrevDeclInScope);
}

OMPDeclareMapperDecl *OMPDeclareMapperDecl::CreateDeserialized(ASTContext &C,
                                                               GlobalDeclID ID,
                                                               unsigned N) {
  return OMPDeclarativeDirective::createEmptyDirective<OMPDeclareMapperDecl>(
      C, ID, N, 1, SourceLocation(), DeclarationName(), QualType(),
      DeclarationName(), /*PrevDeclInScope=*/nullptr);
}

OMPDeclareMapperDecl *OMPDeclareMapperDecl::getPrevDeclInScope() {
  return cast_or_null<OMPDeclareMapperDecl>(
      PrevDeclInScope.get(getASTContext().getExternalSource()));
}

const OMPDeclareMapperDecl *OMPDeclareMapperDecl::getPrevDeclInScope() const {
  return cast_or_null<OMPDeclareMapperDecl>(
      PrevDeclInScope.get(getASTContext().getExternalSource()));
}

//===----------------------------------------------------------------------===//
// OMPCapturedExprDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPCapturedExprDecl::anchor() {}

OMPCapturedExprDecl *OMPCapturedExprDecl::Create(ASTContext &C, DeclContext *DC,
                                                 IdentifierInfo *Id, QualType T,
                                                 SourceLocation StartLoc) {
  return new (C, DC) OMPCapturedExprDecl(
      C, DC, Id, T, C.getTrivialTypeSourceInfo(T), StartLoc);
}

OMPCapturedExprDecl *OMPCapturedExprDecl::CreateDeserialized(ASTContext &C,
                                                             GlobalDeclID ID) {
  return new (C, ID) OMPCapturedExprDecl(C, nullptr, nullptr, QualType(),
                                         /*TInfo=*/nullptr, SourceLocation());
}

SourceRange OMPCapturedExprDecl::getSourceRange() const {
  assert(hasInit());
  return SourceRange(getInit()->getBeginLoc(), getInit()->getEndLoc());
}
