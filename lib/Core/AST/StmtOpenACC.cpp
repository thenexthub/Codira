//===--- StmtOpenACC.cpp - Classes for OpenACC Constructs -----------------===//
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
// This file implements the subclasses of Stmt class declared in StmtOpenACC.h
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtOpenACC.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/StmtCXX.h"
using namespace language::Core;

OpenACCComputeConstruct *
OpenACCComputeConstruct::CreateEmpty(const ASTContext &C, unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCComputeConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCComputeConstruct(NumClauses);
  return Inst;
}

OpenACCComputeConstruct *OpenACCComputeConstruct::Create(
    const ASTContext &C, OpenACCDirectiveKind K, SourceLocation BeginLoc,
    SourceLocation DirLoc, SourceLocation EndLoc,
    ArrayRef<const OpenACCClause *> Clauses, Stmt *StructuredBlock) {
  void *Mem = C.Allocate(
      OpenACCComputeConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem) OpenACCComputeConstruct(K, BeginLoc, DirLoc, EndLoc,
                                                 Clauses, StructuredBlock);
  return Inst;
}

OpenACCLoopConstruct::OpenACCLoopConstruct(unsigned NumClauses)
    : OpenACCAssociatedStmtConstruct(
          OpenACCLoopConstructClass, OpenACCDirectiveKind::Loop,
          SourceLocation{}, SourceLocation{}, SourceLocation{},
          /*AssociatedStmt=*/nullptr) {
  std::uninitialized_value_construct_n(getTrailingObjects(), NumClauses);
  setClauseList(getTrailingObjects(NumClauses));
}

OpenACCLoopConstruct::OpenACCLoopConstruct(
    OpenACCDirectiveKind ParentKind, SourceLocation Start,
    SourceLocation DirLoc, SourceLocation End,
    ArrayRef<const OpenACCClause *> Clauses, Stmt *Loop)
    : OpenACCAssociatedStmtConstruct(OpenACCLoopConstructClass,
                                     OpenACCDirectiveKind::Loop, Start, DirLoc,
                                     End, Loop),
      ParentComputeConstructKind(ParentKind) {
  // accept 'nullptr' for the loop. This is diagnosed somewhere, but this gives
  // us some level of AST fidelity in the error case.
  assert((Loop == nullptr || isa<ForStmt, CXXForRangeStmt>(Loop)) &&
         "Associated Loop not a for loop?");
  // Initialize the trailing storage.
  toolchain::uninitialized_copy(Clauses, getTrailingObjects());

  setClauseList(getTrailingObjects(Clauses.size()));
}

OpenACCLoopConstruct *OpenACCLoopConstruct::CreateEmpty(const ASTContext &C,
                                                        unsigned NumClauses) {
  void *Mem =
      C.Allocate(OpenACCLoopConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCLoopConstruct(NumClauses);
  return Inst;
}

OpenACCLoopConstruct *OpenACCLoopConstruct::Create(
    const ASTContext &C, OpenACCDirectiveKind ParentKind,
    SourceLocation BeginLoc, SourceLocation DirLoc, SourceLocation EndLoc,
    ArrayRef<const OpenACCClause *> Clauses, Stmt *Loop) {
  void *Mem =
      C.Allocate(OpenACCLoopConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem)
      OpenACCLoopConstruct(ParentKind, BeginLoc, DirLoc, EndLoc, Clauses, Loop);
  return Inst;
}

OpenACCCombinedConstruct *
OpenACCCombinedConstruct::CreateEmpty(const ASTContext &C,
                                      unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCCombinedConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCCombinedConstruct(NumClauses);
  return Inst;
}

OpenACCCombinedConstruct *OpenACCCombinedConstruct::Create(
    const ASTContext &C, OpenACCDirectiveKind DK, SourceLocation BeginLoc,
    SourceLocation DirLoc, SourceLocation EndLoc,
    ArrayRef<const OpenACCClause *> Clauses, Stmt *Loop) {
  void *Mem = C.Allocate(
      OpenACCCombinedConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem)
      OpenACCCombinedConstruct(DK, BeginLoc, DirLoc, EndLoc, Clauses, Loop);
  return Inst;
}

OpenACCDataConstruct *OpenACCDataConstruct::CreateEmpty(const ASTContext &C,
                                                        unsigned NumClauses) {
  void *Mem =
      C.Allocate(OpenACCDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCDataConstruct(NumClauses);
  return Inst;
}

OpenACCDataConstruct *
OpenACCDataConstruct::Create(const ASTContext &C, SourceLocation Start,
                             SourceLocation DirectiveLoc, SourceLocation End,
                             ArrayRef<const OpenACCClause *> Clauses,
                             Stmt *StructuredBlock) {
  void *Mem =
      C.Allocate(OpenACCDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem)
      OpenACCDataConstruct(Start, DirectiveLoc, End, Clauses, StructuredBlock);
  return Inst;
}

OpenACCEnterDataConstruct *
OpenACCEnterDataConstruct::CreateEmpty(const ASTContext &C,
                                       unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCEnterDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCEnterDataConstruct(NumClauses);
  return Inst;
}

OpenACCEnterDataConstruct *OpenACCEnterDataConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation End, ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem = C.Allocate(
      OpenACCEnterDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst =
      new (Mem) OpenACCEnterDataConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}

OpenACCExitDataConstruct *
OpenACCExitDataConstruct::CreateEmpty(const ASTContext &C,
                                      unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCExitDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCExitDataConstruct(NumClauses);
  return Inst;
}

OpenACCExitDataConstruct *OpenACCExitDataConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation End, ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem = C.Allocate(
      OpenACCExitDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst =
      new (Mem) OpenACCExitDataConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}

OpenACCHostDataConstruct *
OpenACCHostDataConstruct::CreateEmpty(const ASTContext &C,
                                      unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCHostDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCHostDataConstruct(NumClauses);
  return Inst;
}

OpenACCHostDataConstruct *OpenACCHostDataConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation End, ArrayRef<const OpenACCClause *> Clauses,
    Stmt *StructuredBlock) {
  void *Mem = C.Allocate(
      OpenACCHostDataConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem) OpenACCHostDataConstruct(Start, DirectiveLoc, End,
                                                  Clauses, StructuredBlock);
  return Inst;
}

OpenACCWaitConstruct *OpenACCWaitConstruct::CreateEmpty(const ASTContext &C,
                                                        unsigned NumExprs,
                                                        unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCWaitConstruct::totalSizeToAlloc<Expr *, OpenACCClause *>(
          NumExprs, NumClauses));

  auto *Inst = new (Mem) OpenACCWaitConstruct(NumExprs, NumClauses);
  return Inst;
}

OpenACCWaitConstruct *OpenACCWaitConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation LParenLoc, Expr *DevNumExpr, SourceLocation QueuesLoc,
    ArrayRef<Expr *> QueueIdExprs, SourceLocation RParenLoc, SourceLocation End,
    ArrayRef<const OpenACCClause *> Clauses) {

  assert(!toolchain::is_contained(QueueIdExprs, nullptr));

  void *Mem = C.Allocate(
      OpenACCWaitConstruct::totalSizeToAlloc<Expr *, OpenACCClause *>(
          QueueIdExprs.size() + 1, Clauses.size()));

  auto *Inst = new (Mem)
      OpenACCWaitConstruct(Start, DirectiveLoc, LParenLoc, DevNumExpr,
                           QueuesLoc, QueueIdExprs, RParenLoc, End, Clauses);
  return Inst;
}
OpenACCInitConstruct *OpenACCInitConstruct::CreateEmpty(const ASTContext &C,
                                                        unsigned NumClauses) {
  void *Mem =
      C.Allocate(OpenACCInitConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCInitConstruct(NumClauses);
  return Inst;
}

OpenACCInitConstruct *
OpenACCInitConstruct::Create(const ASTContext &C, SourceLocation Start,
                             SourceLocation DirectiveLoc, SourceLocation End,
                             ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem =
      C.Allocate(OpenACCInitConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst =
      new (Mem) OpenACCInitConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}
OpenACCShutdownConstruct *
OpenACCShutdownConstruct::CreateEmpty(const ASTContext &C,
                                      unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCShutdownConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCShutdownConstruct(NumClauses);
  return Inst;
}

OpenACCShutdownConstruct *OpenACCShutdownConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation End, ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem = C.Allocate(
      OpenACCShutdownConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst =
      new (Mem) OpenACCShutdownConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}

OpenACCSetConstruct *OpenACCSetConstruct::CreateEmpty(const ASTContext &C,
                                                      unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCSetConstruct::totalSizeToAlloc<const OpenACCClause *>(NumClauses));
  auto *Inst = new (Mem) OpenACCSetConstruct(NumClauses);
  return Inst;
}

OpenACCSetConstruct *
OpenACCSetConstruct::Create(const ASTContext &C, SourceLocation Start,
                            SourceLocation DirectiveLoc, SourceLocation End,
                            ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem =
      C.Allocate(OpenACCSetConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem) OpenACCSetConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}

OpenACCUpdateConstruct *
OpenACCUpdateConstruct::CreateEmpty(const ASTContext &C, unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCUpdateConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCUpdateConstruct(NumClauses);
  return Inst;
}

OpenACCUpdateConstruct *
OpenACCUpdateConstruct::Create(const ASTContext &C, SourceLocation Start,
                               SourceLocation DirectiveLoc, SourceLocation End,
                               ArrayRef<const OpenACCClause *> Clauses) {
  void *Mem = C.Allocate(
      OpenACCUpdateConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst =
      new (Mem) OpenACCUpdateConstruct(Start, DirectiveLoc, End, Clauses);
  return Inst;
}

OpenACCAtomicConstruct *
OpenACCAtomicConstruct::CreateEmpty(const ASTContext &C, unsigned NumClauses) {
  void *Mem = C.Allocate(
      OpenACCAtomicConstruct::totalSizeToAlloc<const OpenACCClause *>(
          NumClauses));
  auto *Inst = new (Mem) OpenACCAtomicConstruct(NumClauses);
  return Inst;
}

OpenACCAtomicConstruct *OpenACCAtomicConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    OpenACCAtomicKind AtKind, SourceLocation End,
    ArrayRef<const OpenACCClause *> Clauses, Stmt *AssociatedStmt) {
  void *Mem = C.Allocate(
      OpenACCAtomicConstruct::totalSizeToAlloc<const OpenACCClause *>(
          Clauses.size()));
  auto *Inst = new (Mem) OpenACCAtomicConstruct(Start, DirectiveLoc, AtKind,
                                                End, Clauses, AssociatedStmt);
  return Inst;
}

OpenACCCacheConstruct *OpenACCCacheConstruct::CreateEmpty(const ASTContext &C,
                                                          unsigned NumVars) {
  void *Mem =
      C.Allocate(OpenACCCacheConstruct::totalSizeToAlloc<Expr *>(NumVars));
  auto *Inst = new (Mem) OpenACCCacheConstruct(NumVars);
  return Inst;
}

OpenACCCacheConstruct *OpenACCCacheConstruct::Create(
    const ASTContext &C, SourceLocation Start, SourceLocation DirectiveLoc,
    SourceLocation LParenLoc, SourceLocation ReadOnlyLoc,
    ArrayRef<Expr *> VarList, SourceLocation RParenLoc, SourceLocation End) {
  void *Mem = C.Allocate(
      OpenACCCacheConstruct::totalSizeToAlloc<Expr *>(VarList.size()));
  auto *Inst = new (Mem) OpenACCCacheConstruct(
      Start, DirectiveLoc, LParenLoc, ReadOnlyLoc, VarList, RParenLoc, End);
  return Inst;
}
