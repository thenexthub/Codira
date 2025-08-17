//===-- language/Compability/Evaluate/fold.h ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_FOLD_H_
#define LANGUAGE_COMPABILITY_EVALUATE_FOLD_H_

// Implements expression tree rewriting, particularly constant expression
// and designator reference evaluation.

#include "common.h"
#include "constant.h"
#include "expression.h"
#include "tools.h"
#include "type.h"
#include <variant>

namespace language::Compability::evaluate::characteristics {
class TypeAndShape;
}

namespace language::Compability::evaluate {

using namespace language::Compability::parser::literals;

// Fold() rewrites an expression and returns it.  When the rewritten expression
// is a constant, UnwrapConstantValue() and GetScalarConstantValue() below will
// be able to extract it.
// Note the rvalue reference argument: the rewrites are performed in place
// for efficiency.
template <typename T> Expr<T> Fold(FoldingContext &context, Expr<T> &&expr) {
  return Expr<T>::Rewrite(context, std::move(expr));
}

characteristics::TypeAndShape Fold(
    FoldingContext &, characteristics::TypeAndShape &&);

template <typename A>
std::optional<A> Fold(FoldingContext &context, std::optional<A> &&x) {
  if (x) {
    return Fold(context, std::move(*x));
  } else {
    return std::nullopt;
  }
}

// UnwrapConstantValue() isolates the known constant value of
// an expression, if it has one.  It returns a pointer, which is
// const-qualified when the expression is so.  The value can be
// parenthesized.
template <typename T, typename EXPR>
auto UnwrapConstantValue(EXPR &expr) -> common::Constify<Constant<T>, EXPR> * {
  if (auto *c{UnwrapExpr<Constant<T>>(expr)}) {
    return c;
  } else {
    if (auto *parens{UnwrapExpr<Parentheses<T>>(expr)}) {
      return UnwrapConstantValue<T>(parens->left());
    }
    return nullptr;
  }
}

// GetScalarConstantValue() extracts the known scalar constant value of
// an expression, if it has one.  The value can be parenthesized.
template <typename T, typename EXPR>
constexpr auto GetScalarConstantValue(const EXPR &expr)
    -> std::optional<Scalar<T>> {
  if (const Constant<T> *constant{UnwrapConstantValue<T>(expr)}) {
    return constant->GetScalarValue();
  } else {
    return std::nullopt;
  }
}

// When an expression is a constant integer, ToInt64() extracts its value.
// Ensure that the expression has been folded beforehand when folding might
// be required.
template <int KIND>
constexpr std::optional<std::int64_t> ToInt64(
    const Expr<Type<TypeCategory::Integer, KIND>> &expr) {
  if (auto scalar{
          GetScalarConstantValue<Type<TypeCategory::Integer, KIND>>(expr)}) {
    return scalar->ToInt64();
  } else {
    return std::nullopt;
  }
}
template <int KIND>
constexpr std::optional<std::int64_t> ToInt64(
    const Expr<Type<TypeCategory::Unsigned, KIND>> &expr) {
  if (auto scalar{
          GetScalarConstantValue<Type<TypeCategory::Unsigned, KIND>>(expr)}) {
    return scalar->ToInt64();
  } else {
    return std::nullopt;
  }
}

std::optional<std::int64_t> ToInt64(const Expr<SomeInteger> &);
std::optional<std::int64_t> ToInt64(const Expr<SomeUnsigned> &);
std::optional<std::int64_t> ToInt64(const Expr<SomeType> &);
std::optional<std::int64_t> ToInt64(const ActualArgument &);

template <typename A>
std::optional<std::int64_t> ToInt64(const std::optional<A> &x) {
  if (x) {
    return ToInt64(*x);
  } else {
    return std::nullopt;
  }
}

template <typename A> std::optional<std::int64_t> ToInt64(A *p) {
  if (p) {
    return ToInt64(std::as_const(*p));
  } else {
    return std::nullopt;
  }
}

} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_FOLD_H_
