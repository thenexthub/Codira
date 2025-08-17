//===-- Optimizer/Dialect/FIROps.h - FIR operations -------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIROPS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIROPS_H

#include "language/Compability/Optimizer/Dialect/CUF/Attributes/CUFAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Dialect/FirAliasTagOpInterface.h"
#include "language/Compability/Optimizer/Dialect/FortranVariableInterface.h"
#include "language/Compability/Optimizer/Dialect/SafeTempArrayCopyAttrInterface.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace fir {

class FirEndOp;
class DoLoopOp;
class RealAttr;

void buildCmpCOp(mlir::OpBuilder &builder, mlir::OperationState &result,
                 mlir::arith::CmpFPredicate predicate, mlir::Value lhs,
                 mlir::Value rhs);
unsigned getCaseArgumentOffset(toolchain::ArrayRef<mlir::Attribute> cases,
                               unsigned dest);
DoLoopOp getForInductionVarOwner(mlir::Value val);
mlir::ParseResult isValidCaseAttr(mlir::Attribute attr);
mlir::ParseResult parseCmpcOp(mlir::OpAsmParser &parser,
                              mlir::OperationState &result);
mlir::ParseResult parseSelector(mlir::OpAsmParser &parser,
                                mlir::OperationState &result,
                                mlir::OpAsmParser::UnresolvedOperand &selector,
                                mlir::Type &type);
bool useStrictVolatileVerification();

static constexpr toolchain::StringRef getNormalizedLowerBoundAttrName() {
  return "normalized.lb";
}

/// Model operations which affect global debugging information
struct DebuggingResource
    : public mlir::SideEffects::Resource::Base<DebuggingResource> {
  mlir::StringRef getName() final { return "DebuggingResource"; }
};

/// Model operations which read from/write to volatile memory
struct VolatileMemoryResource
    : public mlir::SideEffects::Resource::Base<VolatileMemoryResource> {
  mlir::StringRef getName() final { return "VolatileMemoryResource"; }
};

class CoordinateIndicesAdaptor;
using IntOrValue = toolchain::PointerUnion<mlir::IntegerAttr, mlir::Value>;

} // namespace fir

#define GET_OP_CLASSES
#include "language/Compability/Optimizer/Dialect/FIROps.h.inc"

namespace fir {
class CoordinateIndicesAdaptor {
public:
  using value_type = IntOrValue;

  CoordinateIndicesAdaptor(mlir::DenseI32ArrayAttr fieldIndices,
                           mlir::ValueRange values)
      : fieldIndices(fieldIndices), values(values) {}

  value_type operator[](size_t index) const {
    assert(index < size() && "index out of bounds");
    return *std::next(begin(), index);
  }

  size_t size() const {
    return fieldIndices ? fieldIndices.size() : values.size();
  }

  bool empty() const {
    return values.empty() && (!fieldIndices || fieldIndices.empty());
  }

  class iterator
      : public toolchain::iterator_facade_base<iterator, std::forward_iterator_tag,
                                          value_type, std::ptrdiff_t,
                                          value_type *, value_type> {
  public:
    iterator(const CoordinateIndicesAdaptor *base,
             std::optional<toolchain::ArrayRef<int32_t>::iterator> fieldIter,
             toolchain::detail::IterOfRange<const mlir::ValueRange> valuesIter)
        : base(base), fieldIter(fieldIter), valuesIter(valuesIter) {}

    value_type operator*() const {
      if (fieldIter && **fieldIter != fir::CoordinateOp::kDynamicIndex) {
        return mlir::IntegerAttr::get(base->fieldIndices.getElementType(),
                                      **fieldIter);
      }
      return *valuesIter;
    }

    iterator &operator++() {
      if (fieldIter) {
        if (**fieldIter == fir::CoordinateOp::kDynamicIndex)
          valuesIter++;
        (*fieldIter)++;
      } else {
        valuesIter++;
      }
      return *this;
    }

    bool operator==(const iterator &rhs) const {
      return base == rhs.base && fieldIter == rhs.fieldIter &&
             valuesIter == rhs.valuesIter;
    }

  private:
    const CoordinateIndicesAdaptor *base;
    std::optional<toolchain::ArrayRef<int32_t>::const_iterator> fieldIter;
    toolchain::detail::IterOfRange<const mlir::ValueRange> valuesIter;
  };

  iterator begin() const {
    std::optional<toolchain::ArrayRef<int32_t>::const_iterator> fieldIter;
    if (fieldIndices)
      fieldIter = fieldIndices.asArrayRef().begin();
    return iterator(this, fieldIter, values.begin());
  }

  iterator end() const {
    std::optional<toolchain::ArrayRef<int32_t>::const_iterator> fieldIter;
    if (fieldIndices)
      fieldIter = fieldIndices.asArrayRef().end();
    return iterator(this, fieldIter, values.end());
  }

private:
  mlir::DenseI32ArrayAttr fieldIndices;
  mlir::ValueRange values;
};

struct LocalitySpecifierOperands {
  toolchain::SmallVector<::mlir::Value> privateVars;
  toolchain::SmallVector<::mlir::Attribute> privateSyms;
};
} // namespace fir

#endif // FORTRAN_OPTIMIZER_DIALECT_FIROPS_H
