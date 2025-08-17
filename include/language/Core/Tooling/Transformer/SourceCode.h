//===--- SourceCode.h - Source code manipulation routines -------*- C++ -*-===//
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
//  This file provides functions that simplify extraction of source code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_TRANSFORMER_SOURCECODE_H
#define LANGUAGE_CORE_TOOLING_TRANSFORMER_SOURCECODE_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/TokenKinds.h"
#include <optional>

namespace language::Core {
namespace tooling {

/// Extends \p Range to include the token \p Terminator, if it immediately
/// follows the end of the range. Otherwise, returns \p Range unchanged.
CharSourceRange maybeExtendRange(CharSourceRange Range,
                                 tok::TokenKind Terminator,
                                 ASTContext &Context);

/// Returns the source range spanning the node, extended to include \p Next, if
/// it immediately follows \p Node. Otherwise, returns the normal range of \p
/// Node.  See comments on `getExtendedText()` for examples.
template <typename T>
CharSourceRange getExtendedRange(const T &Node, tok::TokenKind Next,
                                 ASTContext &Context) {
  return maybeExtendRange(CharSourceRange::getTokenRange(Node.getSourceRange()),
                          Next, Context);
}

/// Returns the logical source range of the node extended to include associated
/// comments and whitespace before and after the node, and associated
/// terminators. The returned range consists of file locations, if valid file
/// locations can be found for the associated content; otherwise, an invalid
/// range is returned.
///
/// Note that parsing comments is disabled by default. In order to select a
/// range containing associated comments, you may need to invoke the tool with
/// `-fparse-all-comments`.
CharSourceRange getAssociatedRange(const Decl &D, ASTContext &Context);

/// Returns the source-code text in the specified range.
StringRef getText(CharSourceRange Range, const ASTContext &Context);

/// Returns the source-code text corresponding to \p Node.
template <typename T>
StringRef getText(const T &Node, const ASTContext &Context) {
  return getText(CharSourceRange::getTokenRange(Node.getSourceRange()),
                 Context);
}

/// Returns the source text of the node, extended to include \p Next, if it
/// immediately follows the node. Otherwise, returns the text of just \p Node.
///
/// For example, given statements S1 and S2 below:
/// \code
///   {
///     // S1:
///     if (!x) return foo();
///     // S2:
///     if (!x) { return 3; }
///   }
/// \endcode
/// then
/// \code
///   getText(S1, Context) = "if (!x) return foo()"
///   getExtendedText(S1, tok::TokenKind::semi, Context)
///     = "if (!x) return foo();"
///   getExtendedText(*S1.getThen(), tok::TokenKind::semi, Context)
///     = "return foo();"
///   getExtendedText(*S2.getThen(), tok::TokenKind::semi, Context)
///     = getText(S2, Context) = "{ return 3; }"
/// \endcode
template <typename T>
StringRef getExtendedText(const T &Node, tok::TokenKind Next,
                          ASTContext &Context) {
  return getText(getExtendedRange(Node, Next, Context), Context);
}

/// Determines whether \p Range is one that can be edited by a rewrite;
/// generally, one that starts and ends within a particular file.
toolchain::Error validateEditRange(const CharSourceRange &Range,
                              const SourceManager &SM);

/// Determines whether \p Range is one that can be read from. If
/// `AllowSystemHeaders` is false, a range that falls within a system header
/// fails validation.
toolchain::Error validateRange(const CharSourceRange &Range, const SourceManager &SM,
                          bool AllowSystemHeaders);

/// Attempts to resolve the given range to one that can be edited by a rewrite;
/// generally, one that starts and ends within a particular file. If a value is
/// returned, it satisfies \c validateEditRange.
///
/// If \c IncludeMacroExpansion is true, a limited set of cases involving source
/// locations in macro expansions is supported. For example, if we're looking to
/// rewrite the int literal 3 to 6, and we have the following definition:
///    #define DO_NOTHING(x) x
/// then
///    foo(DO_NOTHING(3))
/// will be rewritten to
///    foo(6)
std::optional<CharSourceRange>
getFileRangeForEdit(const CharSourceRange &EditRange, const SourceManager &SM,
                    const LangOptions &LangOpts,
                    bool IncludeMacroExpansion = true);
inline std::optional<CharSourceRange>
getFileRangeForEdit(const CharSourceRange &EditRange, const ASTContext &Context,
                    bool IncludeMacroExpansion = true) {
  return getFileRangeForEdit(EditRange, Context.getSourceManager(),
                             Context.getLangOpts(), IncludeMacroExpansion);
}

/// Attempts to resolve the given range to one that starts and ends in a
/// particular file.
///
/// If \c IncludeMacroExpansion is true, a limited set of cases involving source
/// locations in macro expansions is supported. For example, if we're looking to
/// get the range of the int literal 3, and we have the following definition:
///    #define DO_NOTHING(x) x
///    foo(DO_NOTHING(3))
/// the returned range will hold the source text `DO_NOTHING(3)`.
std::optional<CharSourceRange> getFileRange(const CharSourceRange &EditRange,
                                            const SourceManager &SM,
                                            const LangOptions &LangOpts,
                                            bool IncludeMacroExpansion);
inline std::optional<CharSourceRange>
getFileRange(const CharSourceRange &EditRange, const ASTContext &Context,
             bool IncludeMacroExpansion) {
  return getFileRange(EditRange, Context.getSourceManager(),
                      Context.getLangOpts(), IncludeMacroExpansion);
}

} // namespace tooling
} // namespace language::Core
#endif // LANGUAGE_CORE_TOOLING_TRANSFORMER_SOURCECODE_H
