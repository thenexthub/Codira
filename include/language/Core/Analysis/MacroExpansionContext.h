//===- MacroExpansionContext.h - Macro expansion information ----*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_ANALYSIS_MACROEXPANSIONCONTEXT_H
#define LANGUAGE_CORE_ANALYSIS_MACROEXPANSIONCONTEXT_H

#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Lex/Preprocessor.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/SmallVector.h"
#include <optional>

namespace language::Core {

namespace detail {
class MacroExpansionRangeRecorder;
} // namespace detail

/// MacroExpansionContext tracks the macro expansions processed by the
/// Preprocessor. It means that it can track source locations from a single
/// translation unit. For every macro expansion it can tell you what text will
/// be substituted.
///
/// It was designed to deal with:
///  - regular macros
///  - macro functions
///  - variadic macros
///  - transitive macro expansions
///  - macro redefinition
///  - unbalanced parenthesis
///
/// \code{.c}
///   void bar();
///   #define retArg(x) x
///   #define retArgUnclosed retArg(bar()
///   #define BB CC
///   #define applyInt BB(int)
///   #define CC(x) retArgUnclosed
///
///   void unbalancedMacros() {
///     applyInt  );
///   //^~~~~~~~~~^ is the substituted range
///   // Substituted text is "applyInt  )"
///   // Expanded text is "bar()"
///   }
///
///   #define expandArgUnclosedCommaExpr(x) (x, bar(), 1
///   #define f expandArgUnclosedCommaExpr
///
///   void unbalancedMacros2() {
///     int x =  f(f(1))  ));  // Look at the parenthesis!
///   //         ^~~~~~^ is the substituted range
///   // Substituted text is "f(f(1))"
///   // Expanded text is "((1,bar(),1,bar(),1"
///   }
/// \endcode
/// \remark Currently we don't respect the whitespaces between expanded tokens,
///         so the output for this example might differ from the -E compiler
///         invocation.
/// \remark All whitespaces are consumed while constructing the expansion.
///         After all identifier a single space inserted to produce a valid C
///         code even if identifier follows an other identifiers such as
///         variable declarations.
/// \remark MacroExpansionContext object must outlive the Preprocessor
///         parameter.
class MacroExpansionContext {
public:
  /// Creates a MacroExpansionContext.
  /// \remark You must call registerForPreprocessor to set the required
  ///         onTokenLexed callback and the PPCallbacks.
  explicit MacroExpansionContext(const LangOptions &LangOpts);

  /// Register the necessary callbacks to the Preprocessor to record the
  /// expansion events and the generated tokens. Must ensure that this object
  /// outlives the given Preprocessor.
  void registerForPreprocessor(Preprocessor &PP);

  /// \param MacroExpansionLoc Must be the expansion location of a macro.
  /// \return The textual representation of the token sequence which was
  ///         substituted in place of the macro after the preprocessing.
  ///         If no macro was expanded at that location, returns std::nullopt.
  std::optional<StringRef>
  getExpandedText(SourceLocation MacroExpansionLoc) const;

  /// \param MacroExpansionLoc Must be the expansion location of a macro.
  /// \return The text from the original source code which were substituted by
  ///         the macro expansion chain from the given location.
  ///         If no macro was expanded at that location, returns std::nullopt.
  std::optional<StringRef>
  getOriginalText(SourceLocation MacroExpansionLoc) const;

  LLVM_DUMP_METHOD void dumpExpansionRangesToStream(raw_ostream &OS) const;
  LLVM_DUMP_METHOD void dumpExpandedTextsToStream(raw_ostream &OS) const;
  LLVM_DUMP_METHOD void dumpExpansionRanges() const;
  LLVM_DUMP_METHOD void dumpExpandedTexts() const;

private:
  friend class detail::MacroExpansionRangeRecorder;
  using MacroExpansionText = SmallString<40>;
  using ExpansionMap = toolchain::DenseMap<SourceLocation, MacroExpansionText>;
  using ExpansionRangeMap = toolchain::DenseMap<SourceLocation, SourceLocation>;

  /// Associates the textual representation of the expanded tokens at the given
  /// macro expansion location.
  ExpansionMap ExpandedTokens;

  /// Tracks which source location was the last affected by any macro
  /// substitution starting from a given macro expansion location.
  ExpansionRangeMap ExpansionRanges;

  Preprocessor *PP = nullptr;
  SourceManager *SM = nullptr;
  const LangOptions &LangOpts;

  /// This callback is called by the preprocessor.
  /// It stores the textual representation of the expanded token sequence for a
  /// macro expansion location.
  void onTokenLexed(const Token &Tok);
};
} // end namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_MACROEXPANSIONCONTEXT_H
