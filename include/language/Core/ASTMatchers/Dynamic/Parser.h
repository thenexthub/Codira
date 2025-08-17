//===- Parser.h - Matcher expression parser ---------------------*- C++ -*-===//
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
/// \file
/// Simple matcher expression parser.
///
/// The parser understands matcher expressions of the form:
///   MatcherName(Arg0, Arg1, ..., ArgN)
/// as well as simple types like strings.
/// The parser does not know how to process the matchers. It delegates this task
/// to a Sema object received as an argument.
///
/// \code
/// Grammar for the expressions supported:
/// <Expression>        := <Literal> | <NamedValue> | <MatcherExpression>
/// <Literal>           := <StringLiteral> | <Boolean> | <Double> | <Unsigned>
/// <StringLiteral>     := "quoted string"
/// <Boolean>           := true | false
/// <Double>            := [0-9]+.[0-9]* | [0-9]+.[0-9]*[eE][-+]?[0-9]+
/// <Unsigned>          := [0-9]+
/// <NamedValue>        := <Identifier>
/// <MatcherExpression> := <Identifier>(<ArgumentList>) |
///                        <Identifier>(<ArgumentList>).bind(<StringLiteral>)
/// <Identifier>        := [a-zA-Z]+
/// <ArgumentList>      := <Expression> | <Expression>,<ArgumentList>
/// \endcode
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ASTMATCHERS_DYNAMIC_PARSER_H
#define LANGUAGE_CORE_ASTMATCHERS_DYNAMIC_PARSER_H

#include "language/Core/ASTMatchers/ASTMatchersInternal.h"
#include "language/Core/ASTMatchers/Dynamic/Registry.h"
#include "language/Core/ASTMatchers/Dynamic/VariantValue.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>
#include <utility>
#include <vector>

namespace language::Core {
namespace ast_matchers {
namespace dynamic {

class Diagnostics;

/// Matcher expression parser.
class Parser {
public:
  /// Interface to connect the parser with the registry and more.
  ///
  /// The parser uses the Sema instance passed into
  /// parseMatcherExpression() to handle all matcher tokens. The simplest
  /// processor implementation would simply call into the registry to create
  /// the matchers.
  /// However, a more complex processor might decide to intercept the matcher
  /// creation and do some extra work. For example, it could apply some
  /// transformation to the matcher by adding some id() nodes, or could detect
  /// specific matcher nodes for more efficient lookup.
  class Sema {
  public:
    virtual ~Sema();

    /// Process a matcher expression.
    ///
    /// All the arguments passed here have already been processed.
    ///
    /// \param Ctor A matcher constructor looked up by lookupMatcherCtor.
    ///
    /// \param NameRange The location of the name in the matcher source.
    ///   Useful for error reporting.
    ///
    /// \param BindID The ID to use to bind the matcher, or a null \c StringRef
    ///   if no ID is specified.
    ///
    /// \param Args The argument list for the matcher.
    ///
    /// \return The matcher objects constructed by the processor, or a null
    ///   matcher if an error occurred. In that case, \c Error will contain a
    ///   description of the error.
    virtual VariantMatcher actOnMatcherExpression(MatcherCtor Ctor,
                                                  SourceRange NameRange,
                                                  StringRef BindID,
                                                  ArrayRef<ParserValue> Args,
                                                  Diagnostics *Error) = 0;

    /// Look up a matcher by name.
    ///
    /// \param MatcherName The matcher name found by the parser.
    ///
    /// \return The matcher constructor, or std::optional<MatcherCtor>() if not
    /// found.
    virtual std::optional<MatcherCtor>
    lookupMatcherCtor(StringRef MatcherName) = 0;

    virtual bool isBuilderMatcher(MatcherCtor) const = 0;

    virtual ASTNodeKind nodeMatcherType(MatcherCtor) const = 0;

    virtual internal::MatcherDescriptorPtr
    buildMatcherCtor(MatcherCtor, SourceRange NameRange,
                     ArrayRef<ParserValue> Args, Diagnostics *Error) const = 0;

    /// Compute the list of completion types for \p Context.
    ///
    /// Each element of \p Context represents a matcher invocation, going from
    /// outermost to innermost. Elements are pairs consisting of a reference to
    /// the matcher constructor and the index of the next element in the
    /// argument list of that matcher (or for the last element, the index of
    /// the completion point in the argument list). An empty list requests
    /// completion for the root matcher.
    virtual std::vector<ArgKind> getAcceptedCompletionTypes(
        toolchain::ArrayRef<std::pair<MatcherCtor, unsigned>> Context);

    /// Compute the list of completions that match any of
    /// \p AcceptedTypes.
    ///
    /// \param AcceptedTypes All types accepted for this completion.
    ///
    /// \return All completions for the specified types.
    /// Completions should be valid when used in \c lookupMatcherCtor().
    /// The matcher constructed from the return of \c lookupMatcherCtor()
    /// should be convertible to some type in \p AcceptedTypes.
    virtual std::vector<MatcherCompletion>
    getMatcherCompletions(toolchain::ArrayRef<ArgKind> AcceptedTypes);
  };

  /// Sema implementation that uses the matcher registry to process the
  ///   tokens.
  class RegistrySema : public Parser::Sema {
  public:
    ~RegistrySema() override;

    std::optional<MatcherCtor>
    lookupMatcherCtor(StringRef MatcherName) override;

    VariantMatcher actOnMatcherExpression(MatcherCtor Ctor,
                                          SourceRange NameRange,
                                          StringRef BindID,
                                          ArrayRef<ParserValue> Args,
                                          Diagnostics *Error) override;

    std::vector<ArgKind> getAcceptedCompletionTypes(
        toolchain::ArrayRef<std::pair<MatcherCtor, unsigned>> Context) override;

    bool isBuilderMatcher(MatcherCtor Ctor) const override;

    ASTNodeKind nodeMatcherType(MatcherCtor) const override;

    internal::MatcherDescriptorPtr
    buildMatcherCtor(MatcherCtor, SourceRange NameRange,
                     ArrayRef<ParserValue> Args,
                     Diagnostics *Error) const override;

    std::vector<MatcherCompletion>
    getMatcherCompletions(toolchain::ArrayRef<ArgKind> AcceptedTypes) override;
  };

  using NamedValueMap = toolchain::StringMap<VariantValue>;

  /// Parse a matcher expression.
  ///
  /// \param MatcherCode The matcher expression to parse.
  ///
  /// \param S The Sema instance that will help the parser
  ///   construct the matchers. If null, it uses the default registry.
  ///
  /// \param NamedValues A map of precomputed named values.  This provides
  ///   the dictionary for the <NamedValue> rule of the grammar.
  ///   If null, it is ignored.
  ///
  /// \return The matcher object constructed by the processor, or an empty
  ///   Optional if an error occurred. In that case, \c Error will contain a
  ///   description of the error.
  ///   The caller takes ownership of the DynTypedMatcher object returned.
  static std::optional<DynTypedMatcher>
  parseMatcherExpression(StringRef &MatcherCode, Sema *S,
                         const NamedValueMap *NamedValues, Diagnostics *Error);
  static std::optional<DynTypedMatcher>
  parseMatcherExpression(StringRef &MatcherCode, Sema *S, Diagnostics *Error) {
    return parseMatcherExpression(MatcherCode, S, nullptr, Error);
  }
  static std::optional<DynTypedMatcher>
  parseMatcherExpression(StringRef &MatcherCode, Diagnostics *Error) {
    return parseMatcherExpression(MatcherCode, nullptr, Error);
  }

  /// Parse an expression.
  ///
  /// Parses any expression supported by this parser. In general, the
  /// \c parseMatcherExpression function is a better approach to get a matcher
  /// object.
  ///
  /// \param S The Sema instance that will help the parser
  ///   construct the matchers. If null, it uses the default registry.
  ///
  /// \param NamedValues A map of precomputed named values.  This provides
  ///   the dictionary for the <NamedValue> rule of the grammar.
  ///   If null, it is ignored.
  static bool parseExpression(StringRef &Code, Sema *S,
                              const NamedValueMap *NamedValues,
                              VariantValue *Value, Diagnostics *Error);
  static bool parseExpression(StringRef &Code, Sema *S, VariantValue *Value,
                              Diagnostics *Error) {
    return parseExpression(Code, S, nullptr, Value, Error);
  }
  static bool parseExpression(StringRef &Code, VariantValue *Value,
                              Diagnostics *Error) {
    return parseExpression(Code, nullptr, Value, Error);
  }

  /// Complete an expression at the given offset.
  ///
  /// \param S The Sema instance that will help the parser
  ///   construct the matchers. If null, it uses the default registry.
  ///
  /// \param NamedValues A map of precomputed named values.  This provides
  ///   the dictionary for the <NamedValue> rule of the grammar.
  ///   If null, it is ignored.
  ///
  /// \return The list of completions, which may be empty if there are no
  /// available completions or if an error occurred.
  static std::vector<MatcherCompletion>
  completeExpression(StringRef &Code, unsigned CompletionOffset, Sema *S,
                     const NamedValueMap *NamedValues);
  static std::vector<MatcherCompletion>
  completeExpression(StringRef &Code, unsigned CompletionOffset, Sema *S) {
    return completeExpression(Code, CompletionOffset, S, nullptr);
  }
  static std::vector<MatcherCompletion>
  completeExpression(StringRef &Code, unsigned CompletionOffset) {
    return completeExpression(Code, CompletionOffset, nullptr);
  }

private:
  class CodeTokenizer;
  struct ScopedContextEntry;
  struct TokenInfo;

  Parser(CodeTokenizer *Tokenizer, Sema *S,
         const NamedValueMap *NamedValues,
         Diagnostics *Error);

  bool parseBindID(std::string &BindID);
  bool parseExpressionImpl(VariantValue *Value);
  bool parseMatcherBuilder(MatcherCtor Ctor, const TokenInfo &NameToken,
                           const TokenInfo &OpenToken, VariantValue *Value);
  bool parseMatcherExpressionImpl(const TokenInfo &NameToken,
                                  const TokenInfo &OpenToken,
                                  std::optional<MatcherCtor> Ctor,
                                  VariantValue *Value);
  bool parseIdentifierPrefixImpl(VariantValue *Value);

  void addCompletion(const TokenInfo &CompToken,
                     const MatcherCompletion &Completion);
  void addExpressionCompletions();

  std::vector<MatcherCompletion>
  getNamedValueCompletions(ArrayRef<ArgKind> AcceptedTypes);

  CodeTokenizer *const Tokenizer;
  Sema *const S;
  const NamedValueMap *const NamedValues;
  Diagnostics *const Error;

  using ContextStackTy = std::vector<std::pair<MatcherCtor, unsigned>>;

  ContextStackTy ContextStack;
  std::vector<MatcherCompletion> Completions;
};

} // namespace dynamic
} // namespace ast_matchers
} // namespace language::Core

#endif // LANGUAGE_CORE_ASTMATCHERS_DYNAMIC_PARSER_H
