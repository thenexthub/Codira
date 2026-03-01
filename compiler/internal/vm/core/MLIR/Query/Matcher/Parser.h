//===--- Parser.h - Matcher expression parser -------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// Simple matcher expression parser.
//
// This file contains the Parser class, which is responsible for parsing
// expressions in a specific format: matcherName(Arg0, Arg1, ..., ArgN). The
// parser can also interpret simple types, like strings.
//
// The actual processing of the matchers is handled by a Sema object that is
// provided to the parser.
//
// The grammar for the supported expressions is as follows:
// <Expression>        := <Literal> | <MatcherExpression>
// <Literal>           := <StringLiteral> | <NumericLiteral> | <BooleanLiteral>
// <StringLiteral>     := "quoted string"
// <BooleanLiteral>    := "true" | "false"
// <NumericLiteral>    := [0-9]+
// <MatcherExpression> := <MatcherName>(<ArgumentList>)
// <MatcherName>       := [a-zA-Z]+
// <ArgumentList>      := <Expression> | <Expression>,<ArgumentList>
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TOOLS_MLIRQUERY_MATCHER_PARSER_H
#define MLIR_TOOLS_MLIRQUERY_MATCHER_PARSER_H

#include "Diagnostics.h"
#include "RegistryManager.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include <memory>
#include <vector>

namespace mlir::query::matcher::internal {

// Matcher expression parser.
class Parser {
public:
  // Different possible tokens.
  enum class TokenKind {
    Eof,
    NewLine,
    OpenParen,
    CloseParen,
    Comma,
    Period,
    Literal,
    Ident,
    InvalidChar,
    CodeCompletion,
    Error
  };

  // Interface to connect the parser with the registry and more. The parser uses
  // the Sema instance passed into parseMatcherExpression() to handle all
  // matcher tokens.
  class Sema {
  public:
    virtual ~Sema();

    // Process a matcher expression. The caller takes ownership of the Matcher
    // object returned.
    virtual VariantMatcher actOnMatcherExpression(
        MatcherCtor ctor, SourceRange nameRange, toolchain::StringRef functionName,
        toolchain::ArrayRef<ParserValue> args, Diagnostics *error) = 0;

    // Look up a matcher by name in the matcher name found by the parser.
    virtual std::optional<MatcherCtor>
    lookupMatcherCtor(toolchain::StringRef matcherName) = 0;

    // Compute the list of completion types for Context.
    virtual std::vector<ArgKind> getAcceptedCompletionTypes(
        toolchain::ArrayRef<std::pair<MatcherCtor, unsigned>> Context);

    // Compute the list of completions that match any of acceptedTypes.
    virtual std::vector<MatcherCompletion>
    getMatcherCompletions(toolchain::ArrayRef<ArgKind> acceptedTypes);
  };

  // An implementation of the Sema interface that uses the matcher registry to
  // process tokens.
  class RegistrySema : public Parser::Sema {
  public:
    RegistrySema(const Registry &matcherRegistry)
        : matcherRegistry(matcherRegistry) {}
    ~RegistrySema() override;

    std::optional<MatcherCtor>
    lookupMatcherCtor(toolchain::StringRef matcherName) override;

    VariantMatcher actOnMatcherExpression(MatcherCtor Ctor,
                                          SourceRange NameRange,
                                          StringRef functionName,
                                          ArrayRef<ParserValue> Args,
                                          Diagnostics *Error) override;

    std::vector<ArgKind> getAcceptedCompletionTypes(
        toolchain::ArrayRef<std::pair<MatcherCtor, unsigned>> context) override;

    std::vector<MatcherCompletion>
    getMatcherCompletions(toolchain::ArrayRef<ArgKind> acceptedTypes) override;

  private:
    const Registry &matcherRegistry;
  };

  using NamedValueMap = toolchain::StringMap<VariantValue>;

  // Methods to parse a matcher expression and return a DynMatcher object,
  // transferring ownership to the caller.
  static std::optional<DynMatcher>
  parseMatcherExpression(toolchain::StringRef &matcherCode,
                         const Registry &matcherRegistry,
                         const NamedValueMap *namedValues, Diagnostics *error);
  static std::optional<DynMatcher>
  parseMatcherExpression(toolchain::StringRef &matcherCode,
                         const Registry &matcherRegistry, Diagnostics *error) {
    return parseMatcherExpression(matcherCode, matcherRegistry, nullptr, error);
  }

  // Methods to parse any expression supported by this parser.
  static bool parseExpression(toolchain::StringRef &code,
                              const Registry &matcherRegistry,
                              const NamedValueMap *namedValues,
                              VariantValue *value, Diagnostics *error);

  static bool parseExpression(toolchain::StringRef &code,
                              const Registry &matcherRegistry,
                              VariantValue *value, Diagnostics *error) {
    return parseExpression(code, matcherRegistry, nullptr, value, error);
  }

  // Methods to complete an expression at a given offset.
  static std::vector<MatcherCompletion>
  completeExpression(toolchain::StringRef &code, unsigned completionOffset,
                     const Registry &matcherRegistry,
                     const NamedValueMap *namedValues);
  static std::vector<MatcherCompletion>
  completeExpression(toolchain::StringRef &code, unsigned completionOffset,
                     const Registry &matcherRegistry) {
    return completeExpression(code, completionOffset, matcherRegistry, nullptr);
  }

private:
  class CodeTokenizer;
  struct ScopedContextEntry;
  struct TokenInfo;

  Parser(CodeTokenizer *tokenizer, const Registry &matcherRegistry,
         const NamedValueMap *namedValues, Diagnostics *error);

  bool parseChainedExpression(std::string &argument);

  bool parseExpressionImpl(VariantValue *value);

  bool parseMatcherArgs(std::vector<ParserValue> &args, MatcherCtor ctor,
                        const TokenInfo &nameToken, TokenInfo &endToken);

  bool parseMatcherExpressionImpl(const TokenInfo &nameToken,
                                  const TokenInfo &openToken,
                                  std::optional<MatcherCtor> ctor,
                                  VariantValue *value);

  bool parseIdentifierPrefixImpl(VariantValue *value);

  void addCompletion(const TokenInfo &compToken,
                     const MatcherCompletion &completion);
  void addExpressionCompletions();

  std::vector<MatcherCompletion>
  getNamedValueCompletions(toolchain::ArrayRef<ArgKind> acceptedTypes);

  CodeTokenizer *const tokenizer;
  std::unique_ptr<RegistrySema> sema;
  const NamedValueMap *const namedValues;
  Diagnostics *const error;

  using ContextStackTy = std::vector<std::pair<MatcherCtor, unsigned>>;

  ContextStackTy contextStack;
  std::vector<MatcherCompletion> completions;
};

} // namespace mlir::query::matcher::internal

#endif // MLIR_TOOLS_MLIRQUERY_MATCHER_PARSER_H
