//===--- DeclOpenACC.cpp - Classes for OpenACC Constructs -----------------===//
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
// This file implements the subclasses of Decl class declared in Decl.h
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/DeclOpenACC.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/OpenACCClause.h"

using namespace language::Core;

bool OpenACCConstructDecl::classofKind(Kind K) {
  return OpenACCDeclareDecl::classofKind(K) ||
         OpenACCRoutineDecl::classofKind(K);
}

OpenACCDeclareDecl *
OpenACCDeclareDecl::Create(ASTContext &Ctx, DeclContext *DC,
                           SourceLocation StartLoc, SourceLocation DirLoc,
                           SourceLocation EndLoc,
                           ArrayRef<const OpenACCClause *> Clauses) {
  return new (Ctx, DC,
              additionalSizeToAlloc<const OpenACCClause *>(Clauses.size()))
      OpenACCDeclareDecl(DC, StartLoc, DirLoc, EndLoc, Clauses);
}

OpenACCDeclareDecl *
OpenACCDeclareDecl::CreateDeserialized(ASTContext &Ctx, GlobalDeclID ID,
                                       unsigned NumClauses) {
  return new (Ctx, ID, additionalSizeToAlloc<const OpenACCClause *>(NumClauses))
      OpenACCDeclareDecl(NumClauses);
}

OpenACCRoutineDecl *
OpenACCRoutineDecl::Create(ASTContext &Ctx, DeclContext *DC,
                           SourceLocation StartLoc, SourceLocation DirLoc,
                           SourceLocation LParenLoc, Expr *FuncRef,
                           SourceLocation RParenLoc, SourceLocation EndLoc,
                           ArrayRef<const OpenACCClause *> Clauses) {
  return new (Ctx, DC,
              additionalSizeToAlloc<const OpenACCClause *>(Clauses.size()))
      OpenACCRoutineDecl(DC, StartLoc, DirLoc, LParenLoc, FuncRef, RParenLoc,
                         EndLoc, Clauses);
}

OpenACCRoutineDecl *
OpenACCRoutineDecl::CreateDeserialized(ASTContext &Ctx, GlobalDeclID ID,
                                       unsigned NumClauses) {
  return new (Ctx, ID, additionalSizeToAlloc<const OpenACCClause *>(NumClauses))
      OpenACCRoutineDecl(NumClauses);
}

void OpenACCRoutineDeclAttr::printPrettyPragma(
    toolchain::raw_ostream &OS, const language::Core::PrintingPolicy &P) const {
  if (Clauses.size() > 0) {
    OS << ' ';
    OpenACCClausePrinter Printer{OS, P};
    Printer.VisitClauseList(Clauses);
  }
}
