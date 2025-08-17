//===--- StmtObjC.cpp - Classes for representing ObjC statements ---------===//
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
// This file implements the subclesses of Stmt class declared in StmtObjC.h
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtObjC.h"

#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ASTContext.h"

using namespace language::Core;

ObjCForCollectionStmt::ObjCForCollectionStmt(Stmt *Elem, Expr *Collect,
                                             Stmt *Body, SourceLocation FCL,
                                             SourceLocation RPL)
    : Stmt(ObjCForCollectionStmtClass) {
  SubExprs[ELEM] = Elem;
  SubExprs[COLLECTION] = Collect;
  SubExprs[BODY] = Body;
  ForLoc = FCL;
  RParenLoc = RPL;
}

ObjCAtTryStmt::ObjCAtTryStmt(SourceLocation atTryLoc, Stmt *atTryStmt,
                             Stmt **CatchStmts, unsigned NumCatchStmts,
                             Stmt *atFinallyStmt)
    : Stmt(ObjCAtTryStmtClass), AtTryLoc(atTryLoc),
      NumCatchStmts(NumCatchStmts), HasFinally(atFinallyStmt != nullptr) {
  Stmt **Stmts = getStmts();
  Stmts[0] = atTryStmt;
  for (unsigned I = 0; I != NumCatchStmts; ++I)
    Stmts[I + 1] = CatchStmts[I];

  if (HasFinally)
    Stmts[NumCatchStmts + 1] = atFinallyStmt;
}

ObjCAtTryStmt *ObjCAtTryStmt::Create(const ASTContext &Context,
                                     SourceLocation atTryLoc, Stmt *atTryStmt,
                                     Stmt **CatchStmts, unsigned NumCatchStmts,
                                     Stmt *atFinallyStmt) {
  size_t Size =
      totalSizeToAlloc<Stmt *>(1 + NumCatchStmts + (atFinallyStmt != nullptr));
  void *Mem = Context.Allocate(Size, alignof(ObjCAtTryStmt));
  return new (Mem) ObjCAtTryStmt(atTryLoc, atTryStmt, CatchStmts, NumCatchStmts,
                                 atFinallyStmt);
}

ObjCAtTryStmt *ObjCAtTryStmt::CreateEmpty(const ASTContext &Context,
                                          unsigned NumCatchStmts,
                                          bool HasFinally) {
  size_t Size = totalSizeToAlloc<Stmt *>(1 + NumCatchStmts + HasFinally);
  void *Mem = Context.Allocate(Size, alignof(ObjCAtTryStmt));
  return new (Mem) ObjCAtTryStmt(EmptyShell(), NumCatchStmts, HasFinally);
}

SourceLocation ObjCAtTryStmt::getEndLoc() const {
  if (HasFinally)
    return getFinallyStmt()->getEndLoc();
  if (NumCatchStmts)
    return getCatchStmt(NumCatchStmts - 1)->getEndLoc();
  return getTryBody()->getEndLoc();
}
