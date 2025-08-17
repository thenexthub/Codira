//===--- InterpreterUtils.cpp - Incremental Utils --------*- C++ -*-===//
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
// This file implements some common utils used in the incremental library.
//
//===----------------------------------------------------------------------===//

#include "InterpreterUtils.h"
#include "language/Core/AST/QualTypeNames.h"

namespace language::Core {

IntegerLiteral *IntegerLiteralExpr(ASTContext &C, uint64_t Val) {
  return IntegerLiteral::Create(C, toolchain::APSInt::getUnsigned(Val),
                                C.UnsignedLongLongTy, SourceLocation());
}

Expr *CStyleCastPtrExpr(Sema &S, QualType Ty, Expr *E) {
  ASTContext &Ctx = S.getASTContext();
  if (!Ty->isPointerType())
    Ty = Ctx.getPointerType(Ty);

  TypeSourceInfo *TSI = Ctx.getTrivialTypeSourceInfo(Ty, SourceLocation());
  Expr *Result =
      S.BuildCStyleCastExpr(SourceLocation(), TSI, SourceLocation(), E).get();
  assert(Result && "Cannot create CStyleCastPtrExpr");
  return Result;
}

Expr *CStyleCastPtrExpr(Sema &S, QualType Ty, uintptr_t Ptr) {
  ASTContext &Ctx = S.getASTContext();
  return CStyleCastPtrExpr(S, Ty, IntegerLiteralExpr(Ctx, (uint64_t)Ptr));
}

Sema::DeclGroupPtrTy CreateDGPtrFrom(Sema &S, Decl *D) {
  SmallVector<Decl *, 1> DeclsInGroup;
  DeclsInGroup.push_back(D);
  Sema::DeclGroupPtrTy DeclGroupPtr = S.BuildDeclaratorGroup(DeclsInGroup);
  return DeclGroupPtr;
}

NamespaceDecl *LookupNamespace(Sema &S, toolchain::StringRef Name,
                               const DeclContext *Within) {
  DeclarationName DName = &S.Context.Idents.get(Name);
  LookupResult R(S, DName, SourceLocation(),
                 Sema::LookupNestedNameSpecifierName);
  R.suppressDiagnostics();
  if (!Within)
    S.LookupName(R, S.TUScope);
  else {
    if (const auto *TD = dyn_cast<language::Core::TagDecl>(Within);
        TD && !TD->getDefinition())
      // No definition, no lookup result.
      return nullptr;

    S.LookupQualifiedName(R, const_cast<DeclContext *>(Within));
  }

  if (R.empty())
    return nullptr;

  R.resolveKind();

  return dyn_cast<NamespaceDecl>(R.getFoundDecl());
}

NamedDecl *LookupNamed(Sema &S, toolchain::StringRef Name,
                       const DeclContext *Within) {
  DeclarationName DName = &S.Context.Idents.get(Name);
  LookupResult R(S, DName, SourceLocation(), Sema::LookupOrdinaryName,
                 RedeclarationKind::ForVisibleRedeclaration);

  R.suppressDiagnostics();

  if (!Within)
    S.LookupName(R, S.TUScope);
  else {
    const DeclContext *PrimaryWithin = nullptr;
    if (const auto *TD = dyn_cast<TagDecl>(Within))
      PrimaryWithin = dyn_cast_if_present<DeclContext>(TD->getDefinition());
    else
      PrimaryWithin = Within->getPrimaryContext();

    // No definition, no lookup result.
    if (!PrimaryWithin)
      return nullptr;

    S.LookupQualifiedName(R, const_cast<DeclContext *>(PrimaryWithin));
  }

  if (R.empty())
    return nullptr;
  R.resolveKind();

  if (R.isSingleResult())
    return dyn_cast<NamedDecl>(R.getFoundDecl());

  return nullptr;
}

std::string GetFullTypeName(ASTContext &Ctx, QualType QT) {
  QualType FQT = TypeName::getFullyQualifiedType(QT, Ctx);
  PrintingPolicy Policy(Ctx.getPrintingPolicy());
  Policy.SuppressScope = false;
  Policy.AnonymousTagLocations = false;
  return FQT.getAsString(Policy);
}
} // namespace language::Core
