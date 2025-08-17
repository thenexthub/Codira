//===-- language/Compability/Evaluate/match.h --------------------------*- C++ -*-===//
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
#ifndef LANGUAGE_COMPABILITY_EVALUATE_MATCH_H_
#define LANGUAGE_COMPABILITY_EVALUATE_MATCH_H_

#include "language/Compability/Common/visit.h"
#include "language/Compability/Evaluate/expression.h"
#include "toolchain/ADT/STLExtras.h"

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace language::Compability::evaluate {
namespace match {
namespace detail {
template <typename, typename = void> //
struct IsOperation {
  static constexpr bool value{false};
};

template <typename T>
struct IsOperation<T, std::void_t<decltype(T::operands)>> {
  static constexpr bool value{true};
};
} // namespace detail

template <typename T>
constexpr bool is_operation_v{detail::IsOperation<T>::value};

template <typename T>
const evaluate::Expr<T> &deparen(const evaluate::Expr<T> &x) {
  if (auto *parens{std::get_if<evaluate::Parentheses<T>>(&x.u)}) {
    return deparen(parens->template operand<0>());
  } else {
    return x;
  }
}

// Expr<T> matchers (patterns)
//
// Each pattern should implement
//   bool match(const U &input) const
// member function that returns `true` when the match was successful,
// and `false` otherwise.
//
// Patterns are intended to be composable, i.e. a pattern can take operands
// which themselves are patterns. This composition is expected to match if
// the root pattern and all its operands match given input.

/// Matches any input as long as it has the expected type `MatchType`.
/// Additionally, it sets the member `ref` to the matched input.
template <typename T> struct TypePattern {
  using MatchType = toolchain::remove_cvref_t<T>;

  template <typename U> bool match(const U &input) const {
    if constexpr (std::is_same_v<MatchType, U>) {
      ref = &input;
      return true;
    } else {
      return false;
    }
  }

  mutable const MatchType *ref{nullptr};
};

/// Matches one of the patterns provided as template arguments. All of these
/// patterns should have the same number of operands, i.e. they all should
/// try to match input expression with the same number of children, i.e.
/// AnyOfPattern<SomeBinaryOp, OtherBinaryOp> is ok, whereas
/// AnyOfPattern<SomeBinaryOp, SomeTernaryOp> is not.
template <typename... Patterns> struct AnyOfPattern {
  static_assert(sizeof...(Patterns) != 0);

private:
  using PatternTuple = std::tuple<Patterns...>;

  template <size_t I>
  using Pattern = typename std::tuple_element<I, PatternTuple>::type;

  template <size_t... Is, typename... Ops>
  AnyOfPattern(std::index_sequence<Is...>, const Ops &...ops)
      : patterns(std::make_tuple(Pattern<Is>(ops...)...)) {}

  template <typename P, typename U>
  bool matchOne(const P &pattern, const U &input) const {
    if (pattern.match(input)) {
      ref = &pattern;
      return true;
    }
    return false;
  }

  template <typename U, size_t... Is>
  bool matchImpl(const U &input, std::index_sequence<Is...>) const {
    return (matchOne(std::get<Is>(patterns), input) || ...);
  }

  PatternTuple patterns;

public:
  using Indexes = std::index_sequence_for<Patterns...>;
  using MatchTypes = std::tuple<typename Patterns::MatchType...>;

  template <typename... Ops>
  AnyOfPattern(const Ops &...ops) : AnyOfPattern(Indexes{}, ops...) {}

  template <typename U> bool match(const U &input) const {
    return matchImpl(input, Indexes{});
  }

  mutable std::variant<const Patterns *..., std::monostate> ref{
      std::monostate{}};
};

/// Matches any input of type Expr<T>
/// The indent if this pattern is to be a leaf in multi-operand patterns.
template <typename T> //
struct ExprPattern : public TypePattern<evaluate::Expr<T>> {};

/// Matches evaluate::Expr<T> that contains evaluate::Opreration<OpType>.
template <typename OpType, typename... Ops>
struct OperationPattern : public TypePattern<OpType> {
private:
  using Indexes = std::index_sequence_for<Ops...>;

  template <typename S, size_t... Is>
  bool matchImpl(const S &op, std::index_sequence<Is...>) const {
    using TypeS = toolchain::remove_cvref_t<S>;
    if constexpr (is_operation_v<TypeS>) {
      if constexpr (TypeS::operands == Indexes::size()) {
        return TypePattern<OpType>::match(op) &&
            (std::get<Is>(operands).match(op.template operand<Is>()) && ...);
      }
    }
    return false;
  }

  std::tuple<const Ops &...> operands;

public:
  using MatchType = OpType;

  OperationPattern(const Ops &...ops, toolchain::type_identity<OpType> = {})
      : operands(ops...) {}

  template <typename T> bool match(const evaluate::Expr<T> &input) const {
    return common::visit(
        [&](auto &&s) { return matchImpl(s, Indexes{}); }, deparen(input).u);
  }

  template <typename U> bool match(const U &input) const {
    // Only match Expr<T>
    return false;
  }
};

template <typename OpType, typename... Ops>
OperationPattern(const Ops &...ops, toolchain::type_identity<OpType>)
    -> OperationPattern<OpType, Ops...>;

// Namespace-level definitions

template <typename T> using Expr = ExprPattern<T>;

template <typename OpType, typename... Ops>
using Op = OperationPattern<OpType, Ops...>;

template <typename Pattern, typename Input>
bool match(const Pattern &pattern, const Input &input) {
  return pattern.match(input);
}

// Specific operation patterns

// -- Add
template <typename Type, typename Op0, typename Op1>
struct Add : public Op<evaluate::Add<Type>, Op0, Op1> {
  using Base = Op<evaluate::Add<Type>, Op0, Op1>;

  Add(const Op0 &op0, const Op1 &op1) : Base(op0, op1) {}
};

template <typename Type, typename Op0, typename Op1>
Add<Type, Op0, Op1> add(const Op0 &op0, const Op1 &op1) {
  return Add<Type, Op0, Op1>(op0, op1);
}

// -- Mul
template <typename Type, typename Op0, typename Op1>
struct Mul : public Op<evaluate::Multiply<Type>, Op0, Op1> {
  using Base = Op<evaluate::Multiply<Type>, Op0, Op1>;

  Mul(const Op0 &op0, const Op1 &op1) : Base(op0, op1) {}
};

template <typename Type, typename Op0, typename Op1>
Mul<Type, Op0, Op1> mul(const Op0 &op0, const Op1 &op1) {
  return Mul<Type, Op0, Op1>(op0, op1);
}
} // namespace match
} // namespace language::Compability::evaluate

#endif // FORTRAN_EVALUATE_MATCH_H_
