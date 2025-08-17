//===-- Optimizer/Dialect/FIRAttr.h -- FIR attributes -----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIRATTR_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIRATTR_H

#include "mlir/IR/BuiltinAttributes.h"

namespace mlir {
class DialectAsmParser;
class DialectAsmPrinter;
} // namespace mlir

namespace fir {

class FIROpsDialect;

namespace detail {
struct RealAttributeStorage;
struct TypeAttributeStorage;
} // namespace detail

using KindTy = unsigned;

class ExactTypeAttr
    : public mlir::Attribute::AttrBase<ExactTypeAttr, mlir::Attribute,
                                       detail::TypeAttributeStorage> {
public:
  using Base::Base;
  using ValueType = mlir::Type;

  static constexpr toolchain::StringLiteral name = "fir.type_is";
  static constexpr toolchain::StringRef getAttrName() { return "type_is"; }
  static ExactTypeAttr get(mlir::Type value);

  mlir::Type getType() const;
};

class SubclassAttr
    : public mlir::Attribute::AttrBase<SubclassAttr, mlir::Attribute,
                                       detail::TypeAttributeStorage> {
public:
  using Base::Base;
  using ValueType = mlir::Type;

  static constexpr toolchain::StringLiteral name = "fir.class_is";
  static constexpr toolchain::StringRef getAttrName() { return "class_is"; }
  static SubclassAttr get(mlir::Type value);

  mlir::Type getType() const;
};

/// Attribute which can be applied to a fir.allocmem operation, specifying that
/// the allocation may not be moved to the heap by passes
class MustBeHeapAttr : public mlir::BoolAttr {
public:
  using BoolAttr::BoolAttr;

  static constexpr toolchain::StringLiteral name = "fir.must_be_heap";
  static constexpr toolchain::StringRef getAttrName() { return "fir.must_be_heap"; }
};

// Attributes for building SELECT CASE multiway branches

/// A closed interval (including the bound values) is an interval with both an
/// upper and lower bound as given as ssa-values.
/// A case selector of `CASE (n:m)` corresponds to any value from `n` to `m` and
/// is encoded as `#fir.interval, %n, %m`.
class ClosedIntervalAttr
    : public mlir::Attribute::AttrBase<ClosedIntervalAttr, mlir::Attribute,
                                       mlir::AttributeStorage> {
public:
  using Base::Base;

  static constexpr toolchain::StringLiteral name = "fir.interval";
  static constexpr toolchain::StringRef getAttrName() { return "interval"; }
  static ClosedIntervalAttr get(mlir::MLIRContext *ctxt);
};

/// An upper bound is an open interval (including the bound value) as given as
/// an ssa-value.
/// A case selector of `CASE (:m)` corresponds to any value up to and including
/// `m` and is encoded as `#fir.upper, %m`.
class UpperBoundAttr
    : public mlir::Attribute::AttrBase<UpperBoundAttr, mlir::Attribute,
                                       mlir::AttributeStorage> {
public:
  using Base::Base;

  static constexpr toolchain::StringLiteral name = "fir.upper";
  static constexpr toolchain::StringRef getAttrName() { return "upper"; }
  static UpperBoundAttr get(mlir::MLIRContext *ctxt);
};

/// A lower bound is an open interval (including the bound value) as given as
/// an ssa-value.
/// A case selector of `CASE (n:)` corresponds to any value down to and
/// including `n` and is encoded as `#fir.lower, %n`.
class LowerBoundAttr
    : public mlir::Attribute::AttrBase<LowerBoundAttr, mlir::Attribute,
                                       mlir::AttributeStorage> {
public:
  using Base::Base;

  static constexpr toolchain::StringLiteral name = "fir.lower";
  static constexpr toolchain::StringRef getAttrName() { return "lower"; }
  static LowerBoundAttr get(mlir::MLIRContext *ctxt);
};

/// A pointer interval is a closed interval as given as an ssa-value. The
/// interval contains exactly one value.
/// A case selector of `CASE (p)` corresponds to exactly the value `p` and is
/// encoded as `#fir.point, %p`.
class PointIntervalAttr
    : public mlir::Attribute::AttrBase<PointIntervalAttr, mlir::Attribute,
                                       mlir::AttributeStorage> {
public:
  using Base::Base;

  static constexpr toolchain::StringLiteral name = "fir.point";
  static constexpr toolchain::StringRef getAttrName() { return "point"; }
  static PointIntervalAttr get(mlir::MLIRContext *ctxt);
};

/// A real attribute is used to workaround MLIR's default parsing of a real
/// constant.
/// `#fir.real<10, 3.14>` is used to introduce a real constant of value `3.14`
/// with a kind of `10`.
class RealAttr
    : public mlir::Attribute::AttrBase<RealAttr, mlir::Attribute,
                                       detail::RealAttributeStorage> {
public:
  using Base::Base;
  using ValueType = std::pair<int, toolchain::APFloat>;

  static constexpr toolchain::StringLiteral name = "fir.real";
  static constexpr toolchain::StringRef getAttrName() { return "real"; }
  static RealAttr get(mlir::MLIRContext *ctxt, const ValueType &key);

  KindTy getFKind() const;
  toolchain::APFloat getValue() const;
};

mlir::Attribute parseFirAttribute(FIROpsDialect *dialect,
                                  mlir::DialectAsmParser &parser,
                                  mlir::Type type);

void printFirAttribute(FIROpsDialect *dialect, mlir::Attribute attr,
                       mlir::DialectAsmPrinter &p);

} // namespace fir

#include "language/Compability/Optimizer/Dialect/FIREnumAttr.h.inc"

#define GET_ATTRDEF_CLASSES
#include "language/Compability/Optimizer/Dialect/FIRAttr.h.inc"

#include "language/Compability/Optimizer/Dialect/SafeTempArrayCopyAttrInterface.h"

#endif // FORTRAN_OPTIMIZER_DIALECT_FIRATTR_H
