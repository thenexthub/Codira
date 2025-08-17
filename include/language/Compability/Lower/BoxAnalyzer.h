//===-- BoxAnalyzer.h -------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_BOXANALYZER_H
#define LANGUAGE_COMPABILITY_LOWER_BOXANALYZER_H

#include "language/Compability/Evaluate/fold.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Support/Matcher.h"
#include <optional>

namespace language::Compability::lower {

//===----------------------------------------------------------------------===//
// Classifications of a symbol.
//
// Each classification is a distinct class and can be used in pattern matching.
//===----------------------------------------------------------------------===//

namespace details {

using FromBox = std::monostate;

/// Base class for all box analysis results.
struct ScalarSym {
  ScalarSym(const language::Compability::semantics::Symbol &sym) : sym{&sym} {}
  ScalarSym &operator=(const ScalarSym &) = default;

  const language::Compability::semantics::Symbol &symbol() const { return *sym; }

  static constexpr bool staticSize() { return true; }
  static constexpr bool isChar() { return false; }
  static constexpr bool isArray() { return false; }

private:
  const language::Compability::semantics::Symbol *sym;
};

/// Scalar of dependent type CHARACTER, constant LEN.
struct ScalarStaticChar : ScalarSym {
  ScalarStaticChar(const language::Compability::semantics::Symbol &sym, int64_t len)
      : ScalarSym{sym}, len{len} {}

  int64_t charLen() const { return len; }

  static constexpr bool isChar() { return true; }

private:
  int64_t len;
};

/// Scalar of dependent type Derived, constant LEN(s).
struct ScalarStaticDerived : ScalarSym {
  ScalarStaticDerived(const language::Compability::semantics::Symbol &sym,
                      toolchain::SmallVectorImpl<int64_t> &&lens)
      : ScalarSym{sym}, lens{std::move(lens)} {}

private:
  toolchain::SmallVector<int64_t> lens;
};

/// Scalar of dependent type CHARACTER, dynamic LEN.
struct ScalarDynamicChar : ScalarSym {
  ScalarDynamicChar(const language::Compability::semantics::Symbol &sym,
                    const language::Compability::lower::SomeExpr &len)
      : ScalarSym{sym}, len{len} {}
  ScalarDynamicChar(const language::Compability::semantics::Symbol &sym)
      : ScalarSym{sym}, len{FromBox{}} {}

  std::optional<language::Compability::lower::SomeExpr> charLen() const {
    if (auto *l = std::get_if<language::Compability::lower::SomeExpr>(&len))
      return {*l};
    return std::nullopt;
  }

  static constexpr bool staticSize() { return false; }
  static constexpr bool isChar() { return true; }

private:
  std::variant<FromBox, language::Compability::lower::SomeExpr> len;
};

/// Scalar of dependent type Derived, dynamic LEN(s).
struct ScalarDynamicDerived : ScalarSym {
  ScalarDynamicDerived(const language::Compability::semantics::Symbol &sym,
                       toolchain::SmallVectorImpl<language::Compability::lower::SomeExpr> &&lens)
      : ScalarSym{sym}, lens{std::move(lens)} {}

private:
  toolchain::SmallVector<language::Compability::lower::SomeExpr, 1> lens;
};

struct LBoundsAndShape {
  LBoundsAndShape(toolchain::SmallVectorImpl<int64_t> &&lbounds,
                  toolchain::SmallVectorImpl<int64_t> &&shapes)
      : lbounds{std::move(lbounds)}, shapes{std::move(shapes)} {}

  static constexpr bool staticSize() { return true; }
  static constexpr bool isArray() { return true; }
  bool lboundAllOnes() const {
    return toolchain::all_of(lbounds, [](int64_t v) { return v == 1; });
  }

  toolchain::SmallVector<int64_t> lbounds;
  toolchain::SmallVector<int64_t> shapes;
};

/// Array of T with statically known origin (lbounds) and shape.
struct StaticArray : ScalarSym, LBoundsAndShape {
  StaticArray(const language::Compability::semantics::Symbol &sym,
              toolchain::SmallVectorImpl<int64_t> &&lbounds,
              toolchain::SmallVectorImpl<int64_t> &&shapes)
      : ScalarSym{sym}, LBoundsAndShape{std::move(lbounds), std::move(shapes)} {
  }

  static constexpr bool staticSize() { return LBoundsAndShape::staticSize(); }
};

struct DynamicBound {
  DynamicBound(
      toolchain::SmallVectorImpl<const language::Compability::semantics::ShapeSpec *> &&bounds)
      : bounds{std::move(bounds)} {}

  static constexpr bool staticSize() { return false; }
  static constexpr bool isArray() { return true; }
  bool lboundAllOnes() const {
    return toolchain::all_of(bounds, [](const language::Compability::semantics::ShapeSpec *p) {
      if (auto low = p->lbound().GetExplicit())
        if (auto lb = language::Compability::evaluate::ToInt64(*low))
          return *lb == 1;
      return false;
    });
  }

  toolchain::SmallVector<const language::Compability::semantics::ShapeSpec *> bounds;
};

/// Array of T with dynamic origin and/or shape.
struct DynamicArray : ScalarSym, DynamicBound {
  DynamicArray(
      const language::Compability::semantics::Symbol &sym,
      toolchain::SmallVectorImpl<const language::Compability::semantics::ShapeSpec *> &&bounds)
      : ScalarSym{sym}, DynamicBound{std::move(bounds)} {}

  static constexpr bool staticSize() { return DynamicBound::staticSize(); }
};

/// Array of CHARACTER with statically known LEN, origin, and shape.
struct StaticArrayStaticChar : ScalarStaticChar, LBoundsAndShape {
  StaticArrayStaticChar(const language::Compability::semantics::Symbol &sym, int64_t len,
                        toolchain::SmallVectorImpl<int64_t> &&lbounds,
                        toolchain::SmallVectorImpl<int64_t> &&shapes)
      : ScalarStaticChar{sym, len}, LBoundsAndShape{std::move(lbounds),
                                                    std::move(shapes)} {}

  static constexpr bool staticSize() {
    return ScalarStaticChar::staticSize() && LBoundsAndShape::staticSize();
  }
};

/// Array of CHARACTER with dynamic LEN but constant origin, shape.
struct StaticArrayDynamicChar : ScalarDynamicChar, LBoundsAndShape {
  StaticArrayDynamicChar(const language::Compability::semantics::Symbol &sym,
                         const language::Compability::lower::SomeExpr &len,
                         toolchain::SmallVectorImpl<int64_t> &&lbounds,
                         toolchain::SmallVectorImpl<int64_t> &&shapes)
      : ScalarDynamicChar{sym, len}, LBoundsAndShape{std::move(lbounds),
                                                     std::move(shapes)} {}
  StaticArrayDynamicChar(const language::Compability::semantics::Symbol &sym,
                         toolchain::SmallVectorImpl<int64_t> &&lbounds,
                         toolchain::SmallVectorImpl<int64_t> &&shapes)
      : ScalarDynamicChar{sym}, LBoundsAndShape{std::move(lbounds),
                                                std::move(shapes)} {}

  static constexpr bool staticSize() {
    return ScalarDynamicChar::staticSize() && LBoundsAndShape::staticSize();
  }
};

/// Array of CHARACTER with constant LEN but dynamic origin, shape.
struct DynamicArrayStaticChar : ScalarStaticChar, DynamicBound {
  DynamicArrayStaticChar(
      const language::Compability::semantics::Symbol &sym, int64_t len,
      toolchain::SmallVectorImpl<const language::Compability::semantics::ShapeSpec *> &&bounds)
      : ScalarStaticChar{sym, len}, DynamicBound{std::move(bounds)} {}

  static constexpr bool staticSize() {
    return ScalarStaticChar::staticSize() && DynamicBound::staticSize();
  }
};

/// Array of CHARACTER with dynamic LEN, origin, and shape.
struct DynamicArrayDynamicChar : ScalarDynamicChar, DynamicBound {
  DynamicArrayDynamicChar(
      const language::Compability::semantics::Symbol &sym,
      const language::Compability::lower::SomeExpr &len,
      toolchain::SmallVectorImpl<const language::Compability::semantics::ShapeSpec *> &&bounds)
      : ScalarDynamicChar{sym, len}, DynamicBound{std::move(bounds)} {}
  DynamicArrayDynamicChar(
      const language::Compability::semantics::Symbol &sym,
      toolchain::SmallVectorImpl<const language::Compability::semantics::ShapeSpec *> &&bounds)
      : ScalarDynamicChar{sym}, DynamicBound{std::move(bounds)} {}

  static constexpr bool staticSize() {
    return ScalarDynamicChar::staticSize() && DynamicBound::staticSize();
  }
};

// TODO: Arrays of derived types with LEN(s)...

} // namespace details

inline bool symIsChar(const language::Compability::semantics::Symbol &sym) {
  return sym.GetType()->category() ==
         language::Compability::semantics::DeclTypeSpec::Character;
}

inline bool symIsArray(const language::Compability::semantics::Symbol &sym) {
  const auto *det =
      sym.GetUltimate().detailsIf<language::Compability::semantics::ObjectEntityDetails>();
  return det && det->IsArray();
}

inline bool isExplicitShape(const language::Compability::semantics::Symbol &sym) {
  const auto *det =
      sym.GetUltimate().detailsIf<language::Compability::semantics::ObjectEntityDetails>();
  return det && det->IsArray() && det->shape().IsExplicitShape();
}

inline bool isAssumedSize(const language::Compability::semantics::Symbol &sym) {
  return language::Compability::semantics::IsAssumedSizeArray(sym.GetUltimate());
}

//===----------------------------------------------------------------------===//
// Perform analysis to determine a box's parameter values
//===----------------------------------------------------------------------===//

/// Analyze a symbol, classify it as to whether it just a scalar, a CHARACTER
/// scalar, an array entity, a combination thereof, and whether the LEN, shape,
/// and lbounds are constant or not.
class BoxAnalyzer : public fir::details::matcher<BoxAnalyzer> {
public:
  // Analysis default state
  using None = std::monostate;

  using ScalarSym = details::ScalarSym;
  using ScalarStaticChar = details::ScalarStaticChar;
  using ScalarDynamicChar = details::ScalarDynamicChar;
  using StaticArray = details::StaticArray;
  using DynamicArray = details::DynamicArray;
  using StaticArrayStaticChar = details::StaticArrayStaticChar;
  using StaticArrayDynamicChar = details::StaticArrayDynamicChar;
  using DynamicArrayStaticChar = details::DynamicArrayStaticChar;
  using DynamicArrayDynamicChar = details::DynamicArrayDynamicChar;
  // TODO: derived types

  using VT = std::variant<None, ScalarSym, ScalarStaticChar, ScalarDynamicChar,
                          StaticArray, DynamicArray, StaticArrayStaticChar,
                          StaticArrayDynamicChar, DynamicArrayStaticChar,
                          DynamicArrayDynamicChar>;

  //===--------------------------------------------------------------------===//
  // Constructor
  //===--------------------------------------------------------------------===//

  BoxAnalyzer() : box{None{}} {}

  operator bool() const { return !std::holds_alternative<None>(box); }

  bool isTrivial() const { return std::holds_alternative<ScalarSym>(box); }

  /// Returns true for any sort of CHARACTER.
  bool isChar() const {
    return match([](const ScalarStaticChar &) { return true; },
                 [](const ScalarDynamicChar &) { return true; },
                 [](const StaticArrayStaticChar &) { return true; },
                 [](const StaticArrayDynamicChar &) { return true; },
                 [](const DynamicArrayStaticChar &) { return true; },
                 [](const DynamicArrayDynamicChar &) { return true; },
                 [](const auto &) { return false; });
  }

  /// Returns true for any sort of array.
  bool isArray() const {
    return match([](const StaticArray &) { return true; },
                 [](const DynamicArray &) { return true; },
                 [](const StaticArrayStaticChar &) { return true; },
                 [](const StaticArrayDynamicChar &) { return true; },
                 [](const DynamicArrayStaticChar &) { return true; },
                 [](const DynamicArrayDynamicChar &) { return true; },
                 [](const auto &) { return false; });
  }

  /// Returns true iff this is an array with constant extents and lbounds. This
  /// returns true for arrays of CHARACTER, even if the LEN is not a constant.
  bool isStaticArray() const {
    return match([](const StaticArray &) { return true; },
                 [](const StaticArrayStaticChar &) { return true; },
                 [](const StaticArrayDynamicChar &) { return true; },
                 [](const auto &) { return false; });
  }

  bool isConstant() const {
    return match(
        [](const None &) -> bool {
          toolchain::report_fatal_error("internal: analysis failed");
        },
        [](const auto &x) { return x.staticSize(); });
  }

  std::optional<int64_t> getCharLenConst() const {
    using A = std::optional<int64_t>;
    return match(
        [](const ScalarStaticChar &x) -> A { return {x.charLen()}; },
        [](const StaticArrayStaticChar &x) -> A { return {x.charLen()}; },
        [](const DynamicArrayStaticChar &x) -> A { return {x.charLen()}; },
        [](const auto &) -> A { return std::nullopt; });
  }

  std::optional<language::Compability::lower::SomeExpr> getCharLenExpr() const {
    using A = std::optional<language::Compability::lower::SomeExpr>;
    return match([](const ScalarDynamicChar &x) { return x.charLen(); },
                 [](const StaticArrayDynamicChar &x) { return x.charLen(); },
                 [](const DynamicArrayDynamicChar &x) { return x.charLen(); },
                 [](const auto &) -> A { return std::nullopt; });
  }

  /// Is the origin of this array the default of vector of `1`?
  bool lboundIsAllOnes() const {
    return match(
        [&](const StaticArray &x) { return x.lboundAllOnes(); },
        [&](const DynamicArray &x) { return x.lboundAllOnes(); },
        [&](const StaticArrayStaticChar &x) { return x.lboundAllOnes(); },
        [&](const StaticArrayDynamicChar &x) { return x.lboundAllOnes(); },
        [&](const DynamicArrayStaticChar &x) { return x.lboundAllOnes(); },
        [&](const DynamicArrayDynamicChar &x) { return x.lboundAllOnes(); },
        [](const auto &) -> bool { toolchain::report_fatal_error("not an array"); });
  }

  /// Get the static lbound values (the origin of the array).
  toolchain::ArrayRef<int64_t> staticLBound() const {
    using A = toolchain::ArrayRef<int64_t>;
    return match([](const StaticArray &x) -> A { return x.lbounds; },
                 [](const StaticArrayStaticChar &x) -> A { return x.lbounds; },
                 [](const StaticArrayDynamicChar &x) -> A { return x.lbounds; },
                 [](const auto &) -> A {
                   toolchain::report_fatal_error("does not have static lbounds");
                 });
  }

  /// Get the static extents of the array.
  toolchain::ArrayRef<int64_t> staticShape() const {
    using A = toolchain::ArrayRef<int64_t>;
    return match([](const StaticArray &x) -> A { return x.shapes; },
                 [](const StaticArrayStaticChar &x) -> A { return x.shapes; },
                 [](const StaticArrayDynamicChar &x) -> A { return x.shapes; },
                 [](const auto &) -> A {
                   toolchain::report_fatal_error("does not have static shape");
                 });
  }

  /// Get the dynamic bounds information of the array (both origin, shape).
  toolchain::ArrayRef<const language::Compability::semantics::ShapeSpec *> dynamicBound() const {
    using A = toolchain::ArrayRef<const language::Compability::semantics::ShapeSpec *>;
    return match([](const DynamicArray &x) -> A { return x.bounds; },
                 [](const DynamicArrayStaticChar &x) -> A { return x.bounds; },
                 [](const DynamicArrayDynamicChar &x) -> A { return x.bounds; },
                 [](const auto &) -> A {
                   toolchain::report_fatal_error("does not have bounds");
                 });
  }

  /// Run the analysis on `sym`.
  void analyze(const language::Compability::semantics::Symbol &sym) {
    if (language::Compability::semantics::IsProcedurePointer(sym))
      return;
    if (symIsArray(sym)) {
      bool isConstant = !isAssumedSize(sym);
      toolchain::SmallVector<int64_t> lbounds;
      toolchain::SmallVector<int64_t> shapes;
      toolchain::SmallVector<const language::Compability::semantics::ShapeSpec *> bounds;
      for (const language::Compability::semantics::ShapeSpec &subs : getSymShape(sym)) {
        bounds.push_back(&subs);
        if (!isConstant)
          continue;
        if (auto low = subs.lbound().GetExplicit()) {
          if (auto lb = language::Compability::evaluate::ToInt64(*low)) {
            lbounds.push_back(*lb); // origin for this dim
            if (auto high = subs.ubound().GetExplicit()) {
              if (auto ub = language::Compability::evaluate::ToInt64(*high)) {
                int64_t extent = *ub - *lb + 1;
                shapes.push_back(extent < 0 ? 0 : extent);
                continue;
              }
            } else if (subs.ubound().isStar()) {
              assert((language::Compability::semantics::IsNamedConstant(sym) ||
                      language::Compability::semantics::IsCUDAShared(sym)) &&
                     "expect implied shape constant");
              shapes.push_back(fir::SequenceType::getUnknownExtent());
              continue;
            }
          }
        }
        isConstant = false;
      }

      // sym : array<CHARACTER>
      if (symIsChar(sym)) {
        if (auto len = charLenConstant(sym)) {
          if (isConstant)
            box = StaticArrayStaticChar(sym, *len, std::move(lbounds),
                                        std::move(shapes));
          else
            box = DynamicArrayStaticChar(sym, *len, std::move(bounds));
          return;
        }
        if (auto var = charLenVariable(sym)) {
          if (isConstant)
            box = StaticArrayDynamicChar(sym, *var, std::move(lbounds),
                                         std::move(shapes));
          else
            box = DynamicArrayDynamicChar(sym, *var, std::move(bounds));
          return;
        }
        if (isConstant)
          box = StaticArrayDynamicChar(sym, std::move(lbounds),
                                       std::move(shapes));
        else
          box = DynamicArrayDynamicChar(sym, std::move(bounds));
        return;
      }

      // sym : array<other>
      if (isConstant)
        box = StaticArray(sym, std::move(lbounds), std::move(shapes));
      else
        box = DynamicArray(sym, std::move(bounds));
      return;
    }

    // sym : CHARACTER
    if (symIsChar(sym)) {
      if (auto len = charLenConstant(sym))
        box = ScalarStaticChar(sym, *len);
      else if (auto var = charLenVariable(sym))
        box = ScalarDynamicChar(sym, *var);
      else
        box = ScalarDynamicChar(sym);
      return;
    }

    // sym : other
    box = ScalarSym(sym);
  }

  const VT &matchee() const { return box; }

private:
  // Get the shape of a symbol.
  const language::Compability::semantics::ArraySpec &
  getSymShape(const language::Compability::semantics::Symbol &sym) {
    return sym.GetUltimate()
        .get<language::Compability::semantics::ObjectEntityDetails>()
        .shape();
  }

  // Get the constant LEN of a CHARACTER, if it exists.
  std::optional<int64_t>
  charLenConstant(const language::Compability::semantics::Symbol &sym) {
    if (std::optional<language::Compability::lower::SomeExpr> expr = charLenVariable(sym))
      if (std::optional<int64_t> asInt = language::Compability::evaluate::ToInt64(*expr)) {
        // Length is max(0, *asInt) (F2018 7.4.4.2 point 5.).
        if (*asInt < 0)
          return 0;
        return *asInt;
      }
    return std::nullopt;
  }

  // Get the `SomeExpr` that describes the CHARACTER's LEN.
  std::optional<language::Compability::lower::SomeExpr>
  charLenVariable(const language::Compability::semantics::Symbol &sym) {
    const language::Compability::semantics::ParamValue &lenParam =
        sym.GetType()->characterTypeSpec().length();
    if (language::Compability::semantics::MaybeIntExpr expr = lenParam.GetExplicit())
      return {language::Compability::evaluate::AsGenericExpr(std::move(*expr))};
    // For assumed LEN parameters, the length comes from the initialization
    // expression.
    if (sym.attrs().test(language::Compability::semantics::Attr::PARAMETER))
      if (const auto *objectDetails =
              sym.GetUltimate()
                  .detailsIf<language::Compability::semantics::ObjectEntityDetails>())
        if (objectDetails->init())
          if (const auto *charExpr = std::get_if<
                  language::Compability::evaluate::Expr<language::Compability::evaluate::SomeCharacter>>(
                  &objectDetails->init()->u))
            if (language::Compability::semantics::MaybeSubscriptIntExpr expr =
                    charExpr->LEN())
              return {language::Compability::evaluate::AsGenericExpr(std::move(*expr))};
    return std::nullopt;
  }

  VT box;
}; // namespace language::Compability::lower

} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_BOXANALYZER_H
