//===--- SourceExtraction.cpp - Clang refactoring library -----------------===//
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

#include "language/Core/Tooling/Refactoring/Extract/SourceExtraction.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/AST/StmtCXX.h"
#include "language/Core/AST/StmtObjC.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Lexer.h"
#include <optional>

using namespace language::Core;

namespace {

/// Returns true if the token at the given location is a semicolon.
bool isSemicolonAtLocation(SourceLocation TokenLoc, const SourceManager &SM,
                           const LangOptions &LangOpts) {
  return Lexer::getSourceText(
             CharSourceRange::getTokenRange(TokenLoc, TokenLoc), SM,
             LangOpts) == ";";
}

/// Returns true if there should be a semicolon after the given statement.
bool isSemicolonRequiredAfter(const Stmt *S) {
  if (isa<CompoundStmt>(S))
    return false;
  if (const auto *If = dyn_cast<IfStmt>(S))
    return isSemicolonRequiredAfter(If->getElse() ? If->getElse()
                                                  : If->getThen());
  if (const auto *While = dyn_cast<WhileStmt>(S))
    return isSemicolonRequiredAfter(While->getBody());
  if (const auto *For = dyn_cast<ForStmt>(S))
    return isSemicolonRequiredAfter(For->getBody());
  if (const auto *CXXFor = dyn_cast<CXXForRangeStmt>(S))
    return isSemicolonRequiredAfter(CXXFor->getBody());
  if (const auto *ObjCFor = dyn_cast<ObjCForCollectionStmt>(S))
    return isSemicolonRequiredAfter(ObjCFor->getBody());
  if(const auto *Switch = dyn_cast<SwitchStmt>(S))
    return isSemicolonRequiredAfter(Switch->getBody());
  if(const auto *Case = dyn_cast<SwitchCase>(S))
    return isSemicolonRequiredAfter(Case->getSubStmt());
  switch (S->getStmtClass()) {
  case Stmt::DeclStmtClass:
  case Stmt::CXXTryStmtClass:
  case Stmt::ObjCAtSynchronizedStmtClass:
  case Stmt::ObjCAutoreleasePoolStmtClass:
  case Stmt::ObjCAtTryStmtClass:
    return false;
  default:
    return true;
  }
}

/// Returns true if the two source locations are on the same line.
bool areOnSameLine(SourceLocation Loc1, SourceLocation Loc2,
                   const SourceManager &SM) {
  return !Loc1.isMacroID() && !Loc2.isMacroID() &&
         SM.getSpellingLineNumber(Loc1) == SM.getSpellingLineNumber(Loc2);
}

} // end anonymous namespace

namespace language::Core {
namespace tooling {

ExtractionSemicolonPolicy
ExtractionSemicolonPolicy::compute(const Stmt *S, SourceRange &ExtractedRange,
                                   const SourceManager &SM,
                                   const LangOptions &LangOpts) {
  auto neededInExtractedFunction = []() {
    return ExtractionSemicolonPolicy(true, false);
  };
  auto neededInOriginalFunction = []() {
    return ExtractionSemicolonPolicy(false, true);
  };

  /// The extracted expression should be terminated with a ';'. The call to
  /// the extracted function will replace this expression, so it won't need
  /// a terminating ';'.
  if (isa<Expr>(S))
    return neededInExtractedFunction();

  /// Some statements don't need to be terminated with ';'. The call to the
  /// extracted function will be a standalone statement, so it should be
  /// terminated with a ';'.
  bool NeedsSemi = isSemicolonRequiredAfter(S);
  if (!NeedsSemi)
    return neededInOriginalFunction();

  /// Some statements might end at ';'. The extraction will move that ';', so
  /// the call to the extracted function should be terminated with a ';'.
  SourceLocation End = ExtractedRange.getEnd();
  if (isSemicolonAtLocation(End, SM, LangOpts))
    return neededInOriginalFunction();

  /// Other statements should generally have a trailing ';'. We can try to find
  /// it and move it together it with the extracted code.
  std::optional<Token> NextToken = Lexer::findNextToken(End, SM, LangOpts);
  if (NextToken && NextToken->is(tok::semi) &&
      areOnSameLine(NextToken->getLocation(), End, SM)) {
    ExtractedRange.setEnd(NextToken->getLocation());
    return neededInOriginalFunction();
  }

  /// Otherwise insert semicolons in both places.
  return ExtractionSemicolonPolicy(true, true);
}

} // end namespace tooling
} // end namespace language::Core
