//===--- VarBypassDetector.cpp - Bypass jumps detector ------------*- C++ -*-=//
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

#include "VarBypassDetector.h"

#include "CodeGenModule.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/Stmt.h"

using namespace language::Core;
using namespace CodeGen;

/// Clear the object and pre-process for the given statement, usually function
/// body statement.
void VarBypassDetector::Init(CodeGenModule &CGM, const Stmt *Body) {
  FromScopes.clear();
  ToScopes.clear();
  Bypasses.clear();
  Scopes = {{~0U, nullptr}};
  unsigned ParentScope = 0;
  AlwaysBypassed = !BuildScopeInformation(CGM, Body, ParentScope);
  if (!AlwaysBypassed)
    Detect();
}

/// Build scope information for a declaration that is part of a DeclStmt.
/// Returns false if we failed to build scope information and can't tell for
/// which vars are being bypassed.
bool VarBypassDetector::BuildScopeInformation(CodeGenModule &CGM, const Decl *D,
                                              unsigned &ParentScope) {
  const VarDecl *VD = dyn_cast<VarDecl>(D);
  if (VD && VD->hasLocalStorage()) {
    Scopes.push_back({ParentScope, VD});
    ParentScope = Scopes.size() - 1;
  }

  if (const VarDecl *VD = dyn_cast<VarDecl>(D))
    if (const Expr *Init = VD->getInit())
      return BuildScopeInformation(CGM, Init, ParentScope);

  return true;
}

/// Walk through the statements, adding any labels or gotos to
/// LabelAndGotoScopes and recursively walking the AST as needed.
/// Returns false if we failed to build scope information and can't tell for
/// which vars are being bypassed.
bool VarBypassDetector::BuildScopeInformation(CodeGenModule &CGM, const Stmt *S,
                                              unsigned &origParentScope) {
  // If this is a statement, rather than an expression, scopes within it don't
  // propagate out into the enclosing scope. Otherwise we have to worry about
  // block literals, which have the lifetime of their enclosing statement.
  unsigned independentParentScope = origParentScope;
  unsigned &ParentScope =
      ((isa<Expr>(S) && !isa<StmtExpr>(S)) ? origParentScope
                                           : independentParentScope);

  unsigned StmtsToSkip = 0u;

  switch (S->getStmtClass()) {
  case Stmt::IndirectGotoStmtClass:
    return false;

  case Stmt::SwitchStmtClass:
    if (const Stmt *Init = cast<SwitchStmt>(S)->getInit()) {
      if (!BuildScopeInformation(CGM, Init, ParentScope))
        return false;
      ++StmtsToSkip;
    }
    if (const VarDecl *Var = cast<SwitchStmt>(S)->getConditionVariable()) {
      if (!BuildScopeInformation(CGM, Var, ParentScope))
        return false;
      ++StmtsToSkip;
    }
    [[fallthrough]];

  case Stmt::GotoStmtClass:
    FromScopes.push_back({S, ParentScope});
    break;

  case Stmt::DeclStmtClass: {
    const DeclStmt *DS = cast<DeclStmt>(S);
    for (auto *I : DS->decls())
      if (!BuildScopeInformation(CGM, I, origParentScope))
        return false;
    return true;
  }

  case Stmt::CaseStmtClass:
  case Stmt::DefaultStmtClass:
  case Stmt::LabelStmtClass:
    toolchain_unreachable("the loop below handles labels and cases");
    break;

  default:
    break;
  }

  for (const Stmt *SubStmt : S->children()) {
    if (!SubStmt)
      continue;
    if (StmtsToSkip) {
      --StmtsToSkip;
      continue;
    }

    // Cases, labels, and defaults aren't "scope parents".  It's also
    // important to handle these iteratively instead of recursively in
    // order to avoid blowing out the stack.
    while (true) {
      const Stmt *Next;
      if (const SwitchCase *SC = dyn_cast<SwitchCase>(SubStmt))
        Next = SC->getSubStmt();
      else if (const LabelStmt *LS = dyn_cast<LabelStmt>(SubStmt))
        Next = LS->getSubStmt();
      else
        break;

      ToScopes[SubStmt] = ParentScope;
      SubStmt = Next;
    }

    // Recursively walk the AST.
    bool Result;
    CGM.runWithSufficientStackSpace(S->getEndLoc(), [&] {
      Result = BuildScopeInformation(CGM, SubStmt, ParentScope);
    });
    if (!Result)
      return false;
  }
  return true;
}

/// Checks each jump and stores each variable declaration they bypass.
void VarBypassDetector::Detect() {
  for (const auto &S : FromScopes) {
    const Stmt *St = S.first;
    unsigned from = S.second;
    if (const GotoStmt *GS = dyn_cast<GotoStmt>(St)) {
      if (const LabelStmt *LS = GS->getLabel()->getStmt())
        Detect(from, ToScopes[LS]);
    } else if (const SwitchStmt *SS = dyn_cast<SwitchStmt>(St)) {
      for (const SwitchCase *SC = SS->getSwitchCaseList(); SC;
           SC = SC->getNextSwitchCase()) {
        Detect(from, ToScopes[SC]);
      }
    } else {
      toolchain_unreachable("goto or switch was expected");
    }
  }
}

/// Checks the jump and stores each variable declaration it bypasses.
void VarBypassDetector::Detect(unsigned From, unsigned To) {
  while (From != To) {
    if (From < To) {
      assert(Scopes[To].first < To);
      const auto &ScopeTo = Scopes[To];
      To = ScopeTo.first;
      Bypasses.insert(ScopeTo.second);
    } else {
      assert(Scopes[From].first < From);
      From = Scopes[From].first;
    }
  }
}
