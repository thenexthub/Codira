//===- FixitUtil.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_ANALYSIS_SUPPORT_FIXITUTIL_H
#define LANGUAGE_CORE_ANALYSIS_SUPPORT_FIXITUTIL_H

#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Lex/Lexer.h"
#include <optional>
#include <string>

namespace language::Core {

// Returns the text of the pointee type of `T` from a `VarDecl` of a pointer
// type. The text is obtained through from `TypeLoc`s.  Since `TypeLoc` does not
// have source ranges of qualifiers ( The `QualTypeLoc` looks hacky too me
// :( ), `Qualifiers` of the pointee type is returned separately through the
// output parameter `QualifiersToAppend`.
std::optional<std::string>
getPointeeTypeText(const DeclaratorDecl *VD, const SourceManager &SM,
                   const LangOptions &LangOpts,
                   std::optional<Qualifiers> *QualifiersToAppend);

// returns text of pointee to pointee (T*&)
std::optional<std::string>
getPointee2TypeText(const DeclaratorDecl *VD, const SourceManager &SM,
                    const LangOptions &LangOpts,
                    std::optional<Qualifiers> *QualifiersToAppend);

SourceLocation getBeginLocOfNestedIdentifier(const DeclaratorDecl *D);

// Returns the literal text in `SourceRange SR`, if `SR` is a valid range.
std::optional<StringRef> getRangeText(SourceRange SR, const SourceManager &SM,
                                      const LangOptions &LangOpts);

// Returns the literal text of the identifier of the given variable declaration.
std::optional<StringRef> getVarDeclIdentifierText(const DeclaratorDecl *VD,
                                                  const SourceManager &SM,
                                                  const LangOptions &LangOpts);

// Return text representation of an `Expr`.
std::optional<StringRef> getExprText(const Expr *E, const SourceManager &SM,
                                     const LangOptions &LangOpts);

// Return the source location just past the last character of the AST `Node`.
template <typename NodeTy>
std::optional<SourceLocation> getPastLoc(const NodeTy *Node,
                                         const SourceManager &SM,
                                         const LangOptions &LangOpts) {
  SourceLocation Loc =
      Lexer::getLocForEndOfToken(Node->getEndLoc(), 0, SM, LangOpts);
  if (Loc.isValid())
    return Loc;
  return std::nullopt;
}

// Returns the begin location of the identifier of the given variable
// declaration.
SourceLocation getVarDeclIdentifierLoc(const DeclaratorDecl *VD);

} // end namespace language::Core

#endif /* LANGUAGE_CORE_ANALYSIS_SUPPORT_FIXITUTIL_H */
