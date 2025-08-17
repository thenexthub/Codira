//===-- Lower/Support/Utils.cpp -- utilities --------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Lower/Support/Utils.h"

#include "language/Compability/Common/indirection.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/ConvertVariable.h"
#include "language/Compability/Lower/IterationSpace.h"
#include "language/Compability/Lower/Support/PrivateReductionUtils.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/HLFIR/HLFIRDialect.h"
#include "language/Compability/Semantics/tools.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include <cstdint>
#include <optional>
#include <type_traits>

namespace language::Compability::lower {
// language::Compability::evaluate::Expr are functional values organized like an AST. A
// language::Compability::evaluate::Expr is meant to be moved and cloned. Using the front end
// tools can often cause copies and extra wrapper classes to be added to any
// language::Compability::evaluate::Expr. These values should not be assumed or relied upon to
// have an *object* identity. They are deeply recursive, irregular structures
// built from a large number of classes which do not use inheritance and
// necessitate a large volume of boilerplate code as a result.
//
// Contrastingly, LLVM data structures make ubiquitous assumptions about an
// object's identity via pointers to the object. An object's location in memory
// is thus very often an identifying relation.

// This class defines a hash computation of a language::Compability::evaluate::Expr tree value
// so it can be used with toolchain::DenseMap. The language::Compability::evaluate::Expr need not
// have the same address.
class HashEvaluateExpr {
public:
  // A Se::Symbol is the only part of an language::Compability::evaluate::Expr with an
  // identity property.
  static unsigned getHashValue(const language::Compability::semantics::Symbol &x) {
    return static_cast<unsigned>(reinterpret_cast<std::intptr_t>(&x));
  }
  template <typename A, bool COPY>
  static unsigned getHashValue(const language::Compability::common::Indirection<A, COPY> &x) {
    return getHashValue(x.value());
  }
  template <typename A>
  static unsigned getHashValue(const std::optional<A> &x) {
    if (x.has_value())
      return getHashValue(x.value());
    return 0u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::Subscript &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  static unsigned getHashValue(const language::Compability::evaluate::Triplet &x) {
    return getHashValue(x.lower()) - getHashValue(x.upper()) * 5u -
           getHashValue(x.stride()) * 11u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::Component &x) {
    return getHashValue(x.base()) * 83u - getHashValue(x.GetLastSymbol());
  }
  static unsigned getHashValue(const language::Compability::evaluate::ArrayRef &x) {
    unsigned subs = 1u;
    for (const language::Compability::evaluate::Subscript &v : x.subscript())
      subs -= getHashValue(v);
    return getHashValue(x.base()) * 89u - subs;
  }
  static unsigned getHashValue(const language::Compability::evaluate::CoarrayRef &x) {
    unsigned cosubs = 3u;
    for (const language::Compability::evaluate::Expr<language::Compability::evaluate::SubscriptInteger> &v :
         x.cosubscript())
      cosubs -= getHashValue(v);
    return getHashValue(x.base()) * 97u - cosubs + getHashValue(x.stat()) +
           257u + getHashValue(x.team());
  }
  static unsigned getHashValue(const language::Compability::evaluate::NamedEntity &x) {
    if (x.IsSymbol())
      return getHashValue(x.GetFirstSymbol()) * 11u;
    return getHashValue(x.GetComponent()) * 13u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::DataRef &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  static unsigned getHashValue(const language::Compability::evaluate::ComplexPart &x) {
    return getHashValue(x.complex()) - static_cast<unsigned>(x.part());
  }
  template <language::Compability::common::TypeCategory TC1, int KIND,
            language::Compability::common::TypeCategory TC2>
  static unsigned getHashValue(
      const language::Compability::evaluate::Convert<language::Compability::evaluate::Type<TC1, KIND>, TC2>
          &x) {
    return getHashValue(x.left()) - (static_cast<unsigned>(TC1) + 2u) -
           (static_cast<unsigned>(KIND) + 5u);
  }
  template <int KIND>
  static unsigned
  getHashValue(const language::Compability::evaluate::ComplexComponent<KIND> &x) {
    return getHashValue(x.left()) -
           (static_cast<unsigned>(x.isImaginaryPart) + 1u) * 3u;
  }
  template <typename T>
  static unsigned getHashValue(const language::Compability::evaluate::Parentheses<T> &x) {
    return getHashValue(x.left()) * 17u;
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Negate<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return getHashValue(x.left()) - (static_cast<unsigned>(TC) + 5u) -
           (static_cast<unsigned>(KIND) + 7u);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Add<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) + getHashValue(x.right())) * 23u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Subtract<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 19u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Multiply<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) + getHashValue(x.right())) * 29u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Divide<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 31u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Power<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 37u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Extremum<language::Compability::evaluate::Type<TC, KIND>> &x) {
    return (getHashValue(x.left()) + getHashValue(x.right())) * 41u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND) +
           static_cast<unsigned>(x.ordering) * 7u;
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::RealToIntPower<language::Compability::evaluate::Type<TC, KIND>>
          &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 43u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND);
  }
  template <int KIND>
  static unsigned
  getHashValue(const language::Compability::evaluate::ComplexConstructor<KIND> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 47u +
           static_cast<unsigned>(KIND);
  }
  template <int KIND>
  static unsigned getHashValue(const language::Compability::evaluate::Concat<KIND> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 53u +
           static_cast<unsigned>(KIND);
  }
  template <int KIND>
  static unsigned getHashValue(const language::Compability::evaluate::SetLength<KIND> &x) {
    return (getHashValue(x.left()) - getHashValue(x.right())) * 59u +
           static_cast<unsigned>(KIND);
  }
  static unsigned getHashValue(const language::Compability::semantics::SymbolRef &sym) {
    return getHashValue(sym.get());
  }
  static unsigned getHashValue(const language::Compability::evaluate::Substring &x) {
    return 61u *
               language::Compability::common::visit(
                   [&](const auto &p) { return getHashValue(p); }, x.parent()) -
           getHashValue(x.lower()) - (getHashValue(x.lower()) + 1u);
  }
  static unsigned
  getHashValue(const language::Compability::evaluate::StaticDataObject::Pointer &x) {
    return toolchain::hash_value(x->name());
  }
  static unsigned getHashValue(const language::Compability::evaluate::SpecificIntrinsic &x) {
    return toolchain::hash_value(x.name);
  }
  template <typename A>
  static unsigned getHashValue(const language::Compability::evaluate::Constant<A> &x) {
    // FIXME: Should hash the content.
    return 103u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::ActualArgument &x) {
    if (const language::Compability::evaluate::Symbol *sym = x.GetAssumedTypeDummy())
      return getHashValue(*sym);
    return getHashValue(*x.UnwrapExpr());
  }
  static unsigned
  getHashValue(const language::Compability::evaluate::ProcedureDesignator &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  static unsigned getHashValue(const language::Compability::evaluate::ProcedureRef &x) {
    unsigned args = 13u;
    for (const std::optional<language::Compability::evaluate::ActualArgument> &v :
         x.arguments())
      args -= getHashValue(v);
    return getHashValue(x.proc()) * 101u - args;
  }
  template <typename A>
  static unsigned
  getHashValue(const language::Compability::evaluate::ArrayConstructor<A> &x) {
    // FIXME: hash the contents.
    return 127u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::ImpliedDoIndex &x) {
    return toolchain::hash_value(toStringRef(x.name).str()) * 131u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::TypeParamInquiry &x) {
    return getHashValue(x.base()) * 137u - getHashValue(x.parameter()) * 3u;
  }
  static unsigned getHashValue(const language::Compability::evaluate::DescriptorInquiry &x) {
    return getHashValue(x.base()) * 139u -
           static_cast<unsigned>(x.field()) * 13u +
           static_cast<unsigned>(x.dimension());
  }
  static unsigned
  getHashValue(const language::Compability::evaluate::StructureConstructor &x) {
    // FIXME: hash the contents.
    return 149u;
  }
  template <int KIND>
  static unsigned getHashValue(const language::Compability::evaluate::Not<KIND> &x) {
    return getHashValue(x.left()) * 61u + static_cast<unsigned>(KIND);
  }
  template <int KIND>
  static unsigned
  getHashValue(const language::Compability::evaluate::LogicalOperation<KIND> &x) {
    unsigned result = getHashValue(x.left()) + getHashValue(x.right());
    return result * 67u + static_cast<unsigned>(x.logicalOperator) * 5u;
  }
  template <language::Compability::common::TypeCategory TC, int KIND>
  static unsigned getHashValue(
      const language::Compability::evaluate::Relational<language::Compability::evaluate::Type<TC, KIND>>
          &x) {
    return (getHashValue(x.left()) + getHashValue(x.right())) * 71u +
           static_cast<unsigned>(TC) + static_cast<unsigned>(KIND) +
           static_cast<unsigned>(x.opr) * 11u;
  }
  template <typename A>
  static unsigned getHashValue(const language::Compability::evaluate::Expr<A> &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  static unsigned getHashValue(
      const language::Compability::evaluate::Relational<language::Compability::evaluate::SomeType> &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  template <typename A>
  static unsigned getHashValue(const language::Compability::evaluate::Designator<A> &x) {
    return language::Compability::common::visit(
        [&](const auto &v) { return getHashValue(v); }, x.u);
  }
  template <int BITS>
  static unsigned
  getHashValue(const language::Compability::evaluate::value::Integer<BITS> &x) {
    return static_cast<unsigned>(x.ToSInt());
  }
  static unsigned getHashValue(const language::Compability::evaluate::NullPointer &x) {
    return ~179u;
  }
};

// Define the is equals test for using language::Compability::evaluate::Expr values with
// toolchain::DenseMap.
class IsEqualEvaluateExpr {
public:
  // A Se::Symbol is the only part of an language::Compability::evaluate::Expr with an
  // identity property.
  static bool isEqual(const language::Compability::semantics::Symbol &x,
                      const language::Compability::semantics::Symbol &y) {
    return isEqual(&x, &y);
  }
  static bool isEqual(const language::Compability::semantics::Symbol *x,
                      const language::Compability::semantics::Symbol *y) {
    return x == y;
  }
  template <typename A, bool COPY>
  static bool isEqual(const language::Compability::common::Indirection<A, COPY> &x,
                      const language::Compability::common::Indirection<A, COPY> &y) {
    return isEqual(x.value(), y.value());
  }
  template <typename A>
  static bool isEqual(const std::optional<A> &x, const std::optional<A> &y) {
    if (x.has_value() && y.has_value())
      return isEqual(x.value(), y.value());
    return !x.has_value() && !y.has_value();
  }
  template <typename A>
  static bool isEqual(const std::vector<A> &x, const std::vector<A> &y) {
    if (x.size() != y.size())
      return false;
    const std::size_t size = x.size();
    for (std::remove_const_t<decltype(size)> i = 0; i < size; ++i)
      if (!isEqual(x[i], y[i]))
        return false;
    return true;
  }
  static bool isEqual(const language::Compability::evaluate::Subscript &x,
                      const language::Compability::evaluate::Subscript &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  static bool isEqual(const language::Compability::evaluate::Triplet &x,
                      const language::Compability::evaluate::Triplet &y) {
    return isEqual(x.lower(), y.lower()) && isEqual(x.upper(), y.upper()) &&
           isEqual(x.stride(), y.stride());
  }
  static bool isEqual(const language::Compability::evaluate::Component &x,
                      const language::Compability::evaluate::Component &y) {
    return isEqual(x.base(), y.base()) &&
           isEqual(x.GetLastSymbol(), y.GetLastSymbol());
  }
  static bool isEqual(const language::Compability::evaluate::ArrayRef &x,
                      const language::Compability::evaluate::ArrayRef &y) {
    return isEqual(x.base(), y.base()) && isEqual(x.subscript(), y.subscript());
  }
  static bool isEqual(const language::Compability::evaluate::CoarrayRef &x,
                      const language::Compability::evaluate::CoarrayRef &y) {
    return isEqual(x.base(), y.base()) &&
           isEqual(x.cosubscript(), y.cosubscript()) &&
           isEqual(x.stat(), y.stat()) && isEqual(x.team(), y.team());
  }
  static bool isEqual(const language::Compability::evaluate::NamedEntity &x,
                      const language::Compability::evaluate::NamedEntity &y) {
    if (x.IsSymbol() && y.IsSymbol())
      return isEqual(x.GetFirstSymbol(), y.GetFirstSymbol());
    return !x.IsSymbol() && !y.IsSymbol() &&
           isEqual(x.GetComponent(), y.GetComponent());
  }
  static bool isEqual(const language::Compability::evaluate::DataRef &x,
                      const language::Compability::evaluate::DataRef &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  static bool isEqual(const language::Compability::evaluate::ComplexPart &x,
                      const language::Compability::evaluate::ComplexPart &y) {
    return isEqual(x.complex(), y.complex()) && x.part() == y.part();
  }
  template <typename A, language::Compability::common::TypeCategory TC2>
  static bool isEqual(const language::Compability::evaluate::Convert<A, TC2> &x,
                      const language::Compability::evaluate::Convert<A, TC2> &y) {
    return isEqual(x.left(), y.left());
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::ComplexComponent<KIND> &x,
                      const language::Compability::evaluate::ComplexComponent<KIND> &y) {
    return isEqual(x.left(), y.left()) &&
           x.isImaginaryPart == y.isImaginaryPart;
  }
  template <typename T>
  static bool isEqual(const language::Compability::evaluate::Parentheses<T> &x,
                      const language::Compability::evaluate::Parentheses<T> &y) {
    return isEqual(x.left(), y.left());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Negate<A> &x,
                      const language::Compability::evaluate::Negate<A> &y) {
    return isEqual(x.left(), y.left());
  }
  template <typename A>
  static bool isBinaryEqual(const A &x, const A &y) {
    return isEqual(x.left(), y.left()) && isEqual(x.right(), y.right());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Add<A> &x,
                      const language::Compability::evaluate::Add<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Subtract<A> &x,
                      const language::Compability::evaluate::Subtract<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Multiply<A> &x,
                      const language::Compability::evaluate::Multiply<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Divide<A> &x,
                      const language::Compability::evaluate::Divide<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Power<A> &x,
                      const language::Compability::evaluate::Power<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Extremum<A> &x,
                      const language::Compability::evaluate::Extremum<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::RealToIntPower<A> &x,
                      const language::Compability::evaluate::RealToIntPower<A> &y) {
    return isBinaryEqual(x, y);
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::ComplexConstructor<KIND> &x,
                      const language::Compability::evaluate::ComplexConstructor<KIND> &y) {
    return isBinaryEqual(x, y);
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::Concat<KIND> &x,
                      const language::Compability::evaluate::Concat<KIND> &y) {
    return isBinaryEqual(x, y);
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::SetLength<KIND> &x,
                      const language::Compability::evaluate::SetLength<KIND> &y) {
    return isBinaryEqual(x, y);
  }
  static bool isEqual(const language::Compability::semantics::SymbolRef &x,
                      const language::Compability::semantics::SymbolRef &y) {
    return isEqual(x.get(), y.get());
  }
  static bool isEqual(const language::Compability::evaluate::Substring &x,
                      const language::Compability::evaluate::Substring &y) {
    return language::Compability::common::visit(
               [&](const auto &p, const auto &q) { return isEqual(p, q); },
               x.parent(), y.parent()) &&
           isEqual(x.lower(), y.lower()) && isEqual(x.upper(), y.upper());
  }
  static bool isEqual(const language::Compability::evaluate::StaticDataObject::Pointer &x,
                      const language::Compability::evaluate::StaticDataObject::Pointer &y) {
    return x->name() == y->name();
  }
  static bool isEqual(const language::Compability::evaluate::SpecificIntrinsic &x,
                      const language::Compability::evaluate::SpecificIntrinsic &y) {
    return x.name == y.name;
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Constant<A> &x,
                      const language::Compability::evaluate::Constant<A> &y) {
    return x == y;
  }
  static bool isEqual(const language::Compability::evaluate::ActualArgument &x,
                      const language::Compability::evaluate::ActualArgument &y) {
    if (const language::Compability::evaluate::Symbol *xs = x.GetAssumedTypeDummy()) {
      if (const language::Compability::evaluate::Symbol *ys = y.GetAssumedTypeDummy())
        return isEqual(*xs, *ys);
      return false;
    }
    return !y.GetAssumedTypeDummy() &&
           isEqual(*x.UnwrapExpr(), *y.UnwrapExpr());
  }
  static bool isEqual(const language::Compability::evaluate::ProcedureDesignator &x,
                      const language::Compability::evaluate::ProcedureDesignator &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  static bool isEqual(const language::Compability::evaluate::ProcedureRef &x,
                      const language::Compability::evaluate::ProcedureRef &y) {
    return isEqual(x.proc(), y.proc()) && isEqual(x.arguments(), y.arguments());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::ImpliedDo<A> &x,
                      const language::Compability::evaluate::ImpliedDo<A> &y) {
    return isEqual(x.values(), y.values()) && isEqual(x.lower(), y.lower()) &&
           isEqual(x.upper(), y.upper()) && isEqual(x.stride(), y.stride());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::ArrayConstructorValues<A> &x,
                      const language::Compability::evaluate::ArrayConstructorValues<A> &y) {
    using Expr = language::Compability::evaluate::Expr<A>;
    using ImpliedDo = language::Compability::evaluate::ImpliedDo<A>;
    for (const auto &[xValue, yValue] : toolchain::zip(x, y)) {
      bool checkElement = language::Compability::common::visit(
          common::visitors{
              [&](const Expr &v, const Expr &w) { return isEqual(v, w); },
              [&](const ImpliedDo &v, const ImpliedDo &w) {
                return isEqual(v, w);
              },
              [&](const Expr &, const ImpliedDo &) { return false; },
              [&](const ImpliedDo &, const Expr &) { return false; },
          },
          xValue.u, yValue.u);
      if (!checkElement) {
        return false;
      }
    }
    return true;
  }
  static bool isEqual(const language::Compability::evaluate::SubscriptInteger &x,
                      const language::Compability::evaluate::SubscriptInteger &y) {
    return x == y;
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::ArrayConstructor<A> &x,
                      const language::Compability::evaluate::ArrayConstructor<A> &y) {
    bool checkCharacterType = true;
    if constexpr (A::category == language::Compability::common::TypeCategory::Character) {
      checkCharacterType = isEqual(*x.LEN(), *y.LEN());
    }
    using Base = language::Compability::evaluate::ArrayConstructorValues<A>;
    return isEqual((Base)x, (Base)y) &&
           (x.GetType() == y.GetType() && checkCharacterType);
  }
  static bool isEqual(const language::Compability::evaluate::ImpliedDoIndex &x,
                      const language::Compability::evaluate::ImpliedDoIndex &y) {
    return toStringRef(x.name) == toStringRef(y.name);
  }
  static bool isEqual(const language::Compability::evaluate::TypeParamInquiry &x,
                      const language::Compability::evaluate::TypeParamInquiry &y) {
    return isEqual(x.base(), y.base()) && isEqual(x.parameter(), y.parameter());
  }
  static bool isEqual(const language::Compability::evaluate::DescriptorInquiry &x,
                      const language::Compability::evaluate::DescriptorInquiry &y) {
    return isEqual(x.base(), y.base()) && x.field() == y.field() &&
           x.dimension() == y.dimension();
  }
  static bool isEqual(const language::Compability::evaluate::StructureConstructor &x,
                      const language::Compability::evaluate::StructureConstructor &y) {
    const auto &xValues = x.values();
    const auto &yValues = y.values();
    if (xValues.size() != yValues.size())
      return false;
    if (x.derivedTypeSpec() != y.derivedTypeSpec())
      return false;
    for (const auto &[xSymbol, xValue] : xValues) {
      auto yIt = yValues.find(xSymbol);
      // This should probably never happen, since the derived type
      // should be the same.
      if (yIt == yValues.end())
        return false;
      if (!isEqual(xValue, yIt->second))
        return false;
    }
    return true;
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::Not<KIND> &x,
                      const language::Compability::evaluate::Not<KIND> &y) {
    return isEqual(x.left(), y.left());
  }
  template <int KIND>
  static bool isEqual(const language::Compability::evaluate::LogicalOperation<KIND> &x,
                      const language::Compability::evaluate::LogicalOperation<KIND> &y) {
    return isEqual(x.left(), y.left()) && isEqual(x.right(), y.right());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Relational<A> &x,
                      const language::Compability::evaluate::Relational<A> &y) {
    return isEqual(x.left(), y.left()) && isEqual(x.right(), y.right());
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Expr<A> &x,
                      const language::Compability::evaluate::Expr<A> &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  static bool
  isEqual(const language::Compability::evaluate::Relational<language::Compability::evaluate::SomeType> &x,
          const language::Compability::evaluate::Relational<language::Compability::evaluate::SomeType> &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  template <typename A>
  static bool isEqual(const language::Compability::evaluate::Designator<A> &x,
                      const language::Compability::evaluate::Designator<A> &y) {
    return language::Compability::common::visit(
        [&](const auto &v, const auto &w) { return isEqual(v, w); }, x.u, y.u);
  }
  template <int BITS>
  static bool isEqual(const language::Compability::evaluate::value::Integer<BITS> &x,
                      const language::Compability::evaluate::value::Integer<BITS> &y) {
    return x == y;
  }
  static bool isEqual(const language::Compability::evaluate::NullPointer &x,
                      const language::Compability::evaluate::NullPointer &y) {
    return true;
  }
  template <typename A, typename B,
            std::enable_if_t<!std::is_same_v<A, B>, bool> = true>
  static bool isEqual(const A &, const B &) {
    return false;
  }
};

unsigned getHashValue(const language::Compability::lower::SomeExpr *x) {
  return HashEvaluateExpr::getHashValue(*x);
}

unsigned getHashValue(const language::Compability::lower::ExplicitIterSpace::ArrayBases &x) {
  return language::Compability::common::visit(
      [&](const auto *p) { return HashEvaluateExpr::getHashValue(*p); }, x);
}

bool isEqual(const language::Compability::lower::SomeExpr *x,
             const language::Compability::lower::SomeExpr *y) {
  const auto *empty =
      toolchain::DenseMapInfo<const language::Compability::lower::SomeExpr *>::getEmptyKey();
  const auto *tombstone =
      toolchain::DenseMapInfo<const language::Compability::lower::SomeExpr *>::getTombstoneKey();
  if (x == empty || y == empty || x == tombstone || y == tombstone)
    return x == y;
  return x == y || IsEqualEvaluateExpr::isEqual(*x, *y);
}

bool isEqual(const language::Compability::lower::ExplicitIterSpace::ArrayBases &x,
             const language::Compability::lower::ExplicitIterSpace::ArrayBases &y) {
  return language::Compability::common::visit(
      language::Compability::common::visitors{
          // language::Compability::semantics::Symbol * are the exception here. These pointers
          // have identity; if two Symbol * values are the same (different) then
          // they are the same (different) logical symbol.
          [&](language::Compability::lower::FrontEndSymbol p,
              language::Compability::lower::FrontEndSymbol q) { return p == q; },
          [&](const auto *p, const auto *q) {
            if constexpr (std::is_same_v<decltype(p), decltype(q)>) {
              return IsEqualEvaluateExpr::isEqual(*p, *q);
            } else {
              // Different subtree types are never equal.
              return false;
            }
          }},
      x, y);
}

void copyFirstPrivateSymbol(lower::AbstractConverter &converter,
                            const semantics::Symbol *sym,
                            mlir::OpBuilder::InsertPoint *copyAssignIP) {
  if (sym->test(semantics::Symbol::Flag::OmpFirstPrivate) ||
      sym->test(semantics::Symbol::Flag::LocalityLocalInit))
    converter.copyHostAssociateVar(*sym, copyAssignIP);
}

template <typename OpType, typename OperandsStructType>
void privatizeSymbol(
    lower::AbstractConverter &converter, fir::FirOpBuilder &firOpBuilder,
    lower::SymMap &symTable,
    toolchain::SetVector<const semantics::Symbol *> &allPrivatizedSymbols,
    toolchain::SmallSet<const semantics::Symbol *, 16> &mightHaveReadHostSym,
    const semantics::Symbol *symToPrivatize, OperandsStructType *clauseOps) {
  constexpr bool isDoConcurrent =
      std::is_same_v<OpType, fir::LocalitySpecifierOp>;
  mlir::OpBuilder::InsertPoint dcIP;

  if (isDoConcurrent) {
    dcIP = firOpBuilder.saveInsertionPoint();
    firOpBuilder.setInsertionPoint(
        firOpBuilder.getRegion().getParentOfType<fir::DoConcurrentOp>());
  }

  const semantics::Symbol *sym =
      isDoConcurrent ? &symToPrivatize->GetUltimate() : symToPrivatize;
  const lower::SymbolBox hsb = converter.lookupOneLevelUpSymbol(*sym);
  assert(hsb && "Host symbol box not found");

  mlir::Location symLoc = hsb.getAddr().getLoc();
  std::string privatizerName = sym->name().ToString() + ".privatizer";
  bool emitCopyRegion =
      symToPrivatize->test(semantics::Symbol::Flag::OmpFirstPrivate) ||
      symToPrivatize->test(semantics::Symbol::Flag::LocalityLocalInit);

  mlir::Value privVal = hsb.getAddr();
  mlir::Type allocType = privVal.getType();
  if (!mlir::isa<fir::PointerType>(privVal.getType()))
    allocType = fir::unwrapRefType(privVal.getType());

  if (auto poly = mlir::dyn_cast<fir::ClassType>(allocType)) {
    if (!mlir::isa<fir::PointerType>(poly.getEleTy()) && emitCopyRegion)
      TODO(symLoc, "create polymorphic host associated copy");
  }

  // fir.array<> cannot be converted to any single toolchain type and fir helpers
  // are not available in openmp to toolchainir translation so we cannot generate
  // an alloca for a fir.array type there. Get around this by boxing all
  // arrays.
  if (mlir::isa<fir::SequenceType>(allocType)) {
    hlfir::Entity entity{hsb.getAddr()};
    entity = genVariableBox(symLoc, firOpBuilder, entity);
    privVal = entity.getBase();
    allocType = privVal.getType();
  }

  if (mlir::isa<fir::BaseBoxType>(privVal.getType())) {
    // Boxes should be passed by reference into nested regions:
    auto oldIP = firOpBuilder.saveInsertionPoint();
    firOpBuilder.setInsertionPointToStart(firOpBuilder.getAllocaBlock());
    auto alloca =
        fir::AllocaOp::create(firOpBuilder, symLoc, privVal.getType());
    firOpBuilder.restoreInsertionPoint(oldIP);
    fir::StoreOp::create(firOpBuilder, symLoc, privVal, alloca);
    privVal = alloca;
  }

  mlir::Type argType = privVal.getType();

  OpType privatizerOp = [&]() {
    auto moduleOp = firOpBuilder.getModule();
    auto uniquePrivatizerName = fir::getTypeAsString(
        allocType, converter.getKindMap(),
        converter.mangleName(*sym) +
            (emitCopyRegion ? "_firstprivate" : "_private"));

    if (auto existingPrivatizer =
            moduleOp.lookupSymbol<OpType>(uniquePrivatizerName))
      return existingPrivatizer;

    mlir::OpBuilder::InsertionGuard guard(firOpBuilder);
    firOpBuilder.setInsertionPointToStart(moduleOp.getBody());
    OpType result;

    if constexpr (std::is_same_v<OpType, mlir::omp::PrivateClauseOp>) {
      result = OpType::create(
          firOpBuilder, symLoc, uniquePrivatizerName, allocType,
          emitCopyRegion ? mlir::omp::DataSharingClauseType::FirstPrivate
                         : mlir::omp::DataSharingClauseType::Private);
    } else {
      result =
          OpType::create(firOpBuilder, symLoc, uniquePrivatizerName, allocType,
                         emitCopyRegion ? fir::LocalitySpecifierType::LocalInit
                                        : fir::LocalitySpecifierType::Local);
    }

    fir::ExtendedValue symExV = converter.getSymbolExtendedValue(*sym);
    lower::SymMapScope outerScope(symTable);

    // Populate the `init` region.
    // We need to initialize in the following cases:
    // 1. The allocation was for a derived type which requires initialization
    //    (this can be skipped if it will be initialized anyway by the copy
    //    region, unless the derived type has allocatable components)
    // 2. The allocation was for any kind of box
    // 3. The allocation was for a boxed character
    const bool needsInitialization =
        (language::Compability::lower::hasDefaultInitialization(sym->GetUltimate()) &&
         (!emitCopyRegion || hlfir::mayHaveAllocatableComponent(allocType))) ||
        mlir::isa<fir::BaseBoxType>(allocType) ||
        mlir::isa<fir::BoxCharType>(allocType);
    if (needsInitialization) {
      lower::SymbolBox hsb = converter.lookupOneLevelUpSymbol(
          isDoConcurrent ? symToPrivatize->GetUltimate() : *symToPrivatize);

      assert(hsb && "Host symbol box not found");
      hlfir::Entity entity{hsb.getAddr()};
      bool cannotHaveNonDefaultLowerBounds =
          !entity.mayHaveNonDefaultLowerBounds();

      mlir::Region &initRegion = result.getInitRegion();
      mlir::Location symLoc = hsb.getAddr().getLoc();
      mlir::Block *initBlock = firOpBuilder.createBlock(
          &initRegion, /*insertPt=*/{}, {argType, argType}, {symLoc, symLoc});

      bool emitCopyRegion =
          symToPrivatize->test(semantics::Symbol::Flag::OmpFirstPrivate) ||
          symToPrivatize->test(
              language::Compability::semantics::Symbol::Flag::LocalityLocalInit);

      populateByRefInitAndCleanupRegions(
          converter, symLoc, argType, /*scalarInitValue=*/nullptr, initBlock,
          result.getInitPrivateArg(), result.getInitMoldArg(),
          result.getDeallocRegion(),
          emitCopyRegion ? DeclOperationKind::FirstPrivateOrLocalInit
                         : DeclOperationKind::PrivateOrLocal,
          symToPrivatize, cannotHaveNonDefaultLowerBounds, isDoConcurrent);
      // TODO: currently there are false positives from dead uses of the mold
      // arg
      if (result.initReadsFromMold())
        mightHaveReadHostSym.insert(symToPrivatize);
    }

    // Populate the `copy` region if this is a `firstprivate`.
    if (emitCopyRegion) {
      mlir::Region &copyRegion = result.getCopyRegion();
      // First block argument corresponding to the original/host value while
      // second block argument corresponding to the privatized value.
      mlir::Block *copyEntryBlock = firOpBuilder.createBlock(
          &copyRegion, /*insertPt=*/{}, {argType, argType}, {symLoc, symLoc});
      firOpBuilder.setInsertionPointToEnd(copyEntryBlock);

      auto addSymbol = [&](unsigned argIdx, const semantics::Symbol *symToMap,
                           bool force = false) {
        symExV.match(
            [&](const fir::MutableBoxValue &box) {
              symTable.addSymbol(
                  *symToMap,
                  fir::substBase(box, copyRegion.getArgument(argIdx)), force);
            },
            [&](const auto &box) {
              symTable.addSymbol(*symToMap, copyRegion.getArgument(argIdx),
                                 force);
            });
      };

      addSymbol(0, sym, true);
      lower::SymMapScope innerScope(symTable);
      addSymbol(1, symToPrivatize);

      auto ip = firOpBuilder.saveInsertionPoint();
      copyFirstPrivateSymbol(converter, symToPrivatize, &ip);

      if constexpr (std::is_same_v<OpType, mlir::omp::PrivateClauseOp>) {
        mlir::omp::YieldOp::create(
            firOpBuilder, hsb.getAddr().getLoc(),
            symTable.shallowLookupSymbol(*symToPrivatize).getAddr());
      } else {
        fir::YieldOp::create(
            firOpBuilder, hsb.getAddr().getLoc(),
            symTable.shallowLookupSymbol(*symToPrivatize).getAddr());
      }
    }

    return result;
  }();

  if (clauseOps) {
    clauseOps->privateSyms.push_back(mlir::SymbolRefAttr::get(privatizerOp));
    clauseOps->privateVars.push_back(privVal);
  }

  if (isDoConcurrent)
    allPrivatizedSymbols.insert(symToPrivatize);

  if (isDoConcurrent)
    firOpBuilder.restoreInsertionPoint(dcIP);
}

template void
privatizeSymbol<mlir::omp::PrivateClauseOp, mlir::omp::PrivateClauseOps>(
    lower::AbstractConverter &converter, fir::FirOpBuilder &firOpBuilder,
    lower::SymMap &symTable,
    toolchain::SetVector<const semantics::Symbol *> &allPrivatizedSymbols,
    toolchain::SmallSet<const semantics::Symbol *, 16> &mightHaveReadHostSym,
    const semantics::Symbol *symToPrivatize,
    mlir::omp::PrivateClauseOps *clauseOps);

template void
privatizeSymbol<fir::LocalitySpecifierOp, fir::LocalitySpecifierOperands>(
    lower::AbstractConverter &converter, fir::FirOpBuilder &firOpBuilder,
    lower::SymMap &symTable,
    toolchain::SetVector<const semantics::Symbol *> &allPrivatizedSymbols,
    toolchain::SmallSet<const semantics::Symbol *, 16> &mightHaveReadHostSym,
    const semantics::Symbol *symToPrivatize,
    fir::LocalitySpecifierOperands *clauseOps);

} // end namespace language::Compability::lower
