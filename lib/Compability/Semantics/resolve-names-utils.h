//===-- lib/Semantics/resolve-names-utils.h ---------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_RESOLVE_NAMES_UTILS_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_RESOLVE_NAMES_UTILS_H_

// Utility functions and class for use in resolve-names.cpp.

#include "language/Compability/Evaluate/fold.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/tools.h"
#include "language/Compability/Semantics/expression.h"
#include "language/Compability/Semantics/scope.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Semantics/symbol.h"
#include "language/Compability/Semantics/type.h"
#include "toolchain/Support/raw_ostream.h"
#include <forward_list>

namespace language::Compability::parser {
class CharBlock;
struct ArraySpec;
struct CoarraySpec;
struct ComponentArraySpec;
struct DataRef;
struct DefinedOpName;
struct Designator;
struct Expr;
struct GenericSpec;
struct Name;
} // namespace language::Compability::parser

namespace language::Compability::semantics {

using SourceName = parser::CharBlock;
class SemanticsContext;

// Record that a Name has been resolved to a Symbol
Symbol &Resolve(const parser::Name &, Symbol &);
Symbol *Resolve(const parser::Name &, Symbol *);

// Create a copy of msg with a new severity.
parser::MessageFixedText WithSeverity(
    const parser::MessageFixedText &msg, parser::Severity);

bool IsIntrinsicOperator(const SemanticsContext &, const SourceName &);
bool IsLogicalConstant(const SemanticsContext &, const SourceName &);

template <typename T>
MaybeIntExpr EvaluateIntExpr(SemanticsContext &context, const T &expr) {
  if (MaybeExpr maybeExpr{
          Fold(context.foldingContext(), AnalyzeExpr(context, expr))}) {
    if (auto *intExpr{evaluate::UnwrapExpr<SomeIntExpr>(*maybeExpr)}) {
      return std::move(*intExpr);
    }
  }
  return std::nullopt;
}

template <typename T>
std::optional<std::int64_t> EvaluateInt64(
    SemanticsContext &context, const T &expr) {
  return evaluate::ToInt64(EvaluateIntExpr(context, expr));
}

// Analyze a generic-spec and generate a symbol name and GenericKind for it.
class GenericSpecInfo {
public:
  explicit GenericSpecInfo(const parser::DefinedOpName &x) { Analyze(x); }
  explicit GenericSpecInfo(const parser::GenericSpec &x) { Analyze(x); }

  GenericKind kind() const { return kind_; }
  const SourceName &symbolName() const { return symbolName_.value(); }
  // Set the GenericKind in this symbol and resolve the corresponding
  // name if there is one
  void Resolve(Symbol *) const;
  friend toolchain::raw_ostream &operator<<(
      toolchain::raw_ostream &, const GenericSpecInfo &);

private:
  void Analyze(const parser::DefinedOpName &);
  void Analyze(const parser::GenericSpec &);

  GenericKind kind_;
  const parser::Name *parseName_{nullptr};
  std::optional<SourceName> symbolName_;
};

// Analyze a parser::ArraySpec or parser::CoarraySpec
ArraySpec AnalyzeArraySpec(SemanticsContext &, const parser::ArraySpec &);
ArraySpec AnalyzeArraySpec(
    SemanticsContext &, const parser::ComponentArraySpec &);
ArraySpec AnalyzeDeferredShapeSpecList(
    SemanticsContext &, const parser::DeferredShapeSpecList &);
ArraySpec AnalyzeCoarraySpec(
    SemanticsContext &context, const parser::CoarraySpec &);

// Perform consistency checks on equivalence sets
class EquivalenceSets {
public:
  EquivalenceSets(SemanticsContext &context) : context_{context} {}
  std::vector<EquivalenceSet> &sets() { return sets_; };
  // Resolve this designator and add to the current equivalence set
  void AddToSet(const parser::Designator &);
  // Finish the current equivalence set: determine if it overlaps
  // with any of the others and perform necessary merges if it does.
  void FinishSet(const parser::CharBlock &);

private:
  bool CheckCanEquivalence(
      const parser::CharBlock &, const Symbol &, const Symbol &);
  void MergeInto(const parser::CharBlock &, EquivalenceSet &, std::size_t);
  const EquivalenceObject *Find(const EquivalenceSet &, const Symbol &);
  bool CheckDesignator(const parser::Designator &);
  bool CheckDataRef(const parser::CharBlock &, const parser::DataRef &);
  bool CheckObject(const parser::Name &);
  bool CheckArrayBound(const parser::Expr &);
  bool CheckSubstringBound(const parser::Expr &, bool);
  bool IsCharacterSequenceType(const DeclTypeSpec *);
  bool IsDefaultKindNumericType(const IntrinsicTypeSpec &);
  bool IsDefaultNumericSequenceType(const DeclTypeSpec *);
  static bool IsAnyNumericSequenceType(const DeclTypeSpec *);
  static bool IsSequenceType(
      const DeclTypeSpec *, std::function<bool(const IntrinsicTypeSpec &)>);

  SemanticsContext &context_;
  std::vector<EquivalenceSet> sets_; // all equivalence sets in this scope
  // Map object to index of set it is in
  std::map<EquivalenceObject, std::size_t> objectToSet_;
  EquivalenceSet currSet_; // equivalence set currently being constructed
  struct {
    Symbol *symbol{nullptr};
    std::vector<ConstantSubscript> subscripts;
    std::optional<ConstantSubscript> substringStart;
  } currObject_; // equivalence object currently being constructed
};

// Duplicates a subprogram's dummy arguments and result, if any, and
// maps all of the symbols in their expressions.
struct SymbolAndTypeMappings;
void MapSubprogramToNewSymbols(const Symbol &oldSymbol, Symbol &newSymbol,
    Scope &newScope, SymbolAndTypeMappings * = nullptr);

parser::CharBlock MakeNameFromOperator(
    const parser::DefinedOperator::IntrinsicOperator &op,
    SemanticsContext &context);
parser::CharBlock MangleSpecialFunctions(const parser::CharBlock &name);
std::string MangleDefinedOperator(const parser::CharBlock &name);

} // namespace language::Compability::semantics
#endif // FORTRAN_SEMANTICS_RESOLVE_NAMES_H_
