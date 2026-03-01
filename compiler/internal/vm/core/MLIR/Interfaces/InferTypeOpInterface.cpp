//===- InferTypeOpInterface.cpp - Infer Type Interfaces ---------*- C++ -*-===//
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
// This file contains the definitions of the infer op interfaces defined in
// `InferTypeOpInterface.td`.
//
//===----------------------------------------------------------------------===//

#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Matchers.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/InterleavedRange.h"

using namespace mlir;

namespace mlir {
#include "mlir/Interfaces/InferTypeOpInterface.cpp.inc"
} // namespace mlir

LogicalResult
mlir::reifyResultShapes(OpBuilder &b, Operation *op,
                        ReifiedRankedShapedTypeDims &reifiedReturnShapes) {
  auto reifiableOp = dyn_cast<ReifyRankedShapedTypeOpInterface>(op);
  if (!reifiableOp)
    return failure();
  LogicalResult status = reifiableOp.reifyResultShapes(b, reifiedReturnShapes);
#ifndef NDEBUG
  if (failed(status))
    return failure();
  // Assert that ReifyRankedShapedTypeOpInterface::reifyResultShapes produced
  // a correct result.
  int64_t resultIdx = 0;
  for (OpResult result : op->getResults()) {
    auto shapedType = dyn_cast<ShapedType>(result.getType());
    if (!shapedType)
      continue;
    if (!shapedType.hasRank()) {
      // Nothing to check for unranked shaped values.
      ++resultIdx;
      continue;
    }
    // Assert one OpFoldResult per dimension.
    assert(shapedType.getRank() ==
               static_cast<int64_t>(reifiedReturnShapes[resultIdx].size()) &&
           "incorrect implementation of ReifyRankedShapedTypeOpInterface");
    ++resultIdx;
  }
  // Assert that every shaped value result was reified.
  assert(resultIdx == static_cast<int64_t>(reifiedReturnShapes.size()) &&
         "incorrect implementation of ReifyRankedShapedTypeOpInterface");
#endif // NDEBUG
  return status;
}

FailureOr<SmallVector<OpFoldResult>>
mlir::reifyShapeOfResult(OpBuilder &b, Operation *op, int resultIndex) {
  auto reifiableOp = dyn_cast<ReifyRankedShapedTypeOpInterface>(op);
  if (!reifiableOp)
    return failure();
  return reifiableOp.reifyShapeOfResult(b, resultIndex);
}

FailureOr<OpFoldResult> mlir::reifyDimOfResult(OpBuilder &b, Operation *op,
                                               int resultIndex, int dim) {
  auto reifiableOp = dyn_cast<ReifyRankedShapedTypeOpInterface>(op);
  if (!reifiableOp)
    return failure();
  return reifiableOp.reifyDimOfResult(b, resultIndex, dim);
}

bool ShapeAdaptor::hasRank() const {
  if (val.isNull())
    return false;
  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).hasRank();
  if (isa<Attribute>(val))
    return true;
  return cast<ShapedTypeComponents *>(val)->hasRank();
}

Type ShapeAdaptor::getElementType() const {
  if (val.isNull())
    return nullptr;
  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).getElementType();
  if (isa<Attribute>(val))
    return nullptr;
  return cast<ShapedTypeComponents *>(val)->getElementType();
}

void ShapeAdaptor::getDims(SmallVectorImpl<int64_t> &res) const {
  assert(hasRank());
  if (auto t = toolchain::dyn_cast_if_present<Type>(val)) {
    ArrayRef<int64_t> vals = cast<ShapedType>(t).getShape();
    res.assign(vals.begin(), vals.end());
  } else if (auto attr = toolchain::dyn_cast_if_present<Attribute>(val)) {
    auto dattr = cast<DenseIntElementsAttr>(attr);
    res.clear();
    res.reserve(dattr.size());
    for (auto it : dattr.getValues<APInt>())
      res.push_back(it.getSExtValue());
  } else {
    auto vals = cast<ShapedTypeComponents *>(val)->getDims();
    res.assign(vals.begin(), vals.end());
  }
}

void ShapeAdaptor::getDims(ShapedTypeComponents &res) const {
  assert(hasRank());
  res.ranked = true;
  getDims(res.dims);
}

int64_t ShapeAdaptor::getDimSize(int index) const {
  assert(hasRank());
  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).getDimSize(index);
  if (auto attr = toolchain::dyn_cast_if_present<Attribute>(val))
    return cast<DenseIntElementsAttr>(attr)
        .getValues<APInt>()[index]
        .getSExtValue();
  auto *stc = cast<ShapedTypeComponents *>(val);
  return stc->getDims()[index];
}

int64_t ShapeAdaptor::getRank() const {
  assert(hasRank());
  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).getRank();
  if (auto attr = toolchain::dyn_cast_if_present<Attribute>(val))
    return cast<DenseIntElementsAttr>(attr).size();
  return cast<ShapedTypeComponents *>(val)->getDims().size();
}

bool ShapeAdaptor::hasStaticShape() const {
  if (!hasRank())
    return false;

  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).hasStaticShape();
  if (auto attr = toolchain::dyn_cast_if_present<Attribute>(val)) {
    auto dattr = cast<DenseIntElementsAttr>(attr);
    for (auto index : dattr.getValues<APInt>())
      if (ShapedType::isDynamic(index.getSExtValue()))
        return false;
    return true;
  }
  auto *stc = cast<ShapedTypeComponents *>(val);
  return toolchain::none_of(stc->getDims(), ShapedType::isDynamic);
}

int64_t ShapeAdaptor::getNumElements() const {
  assert(hasStaticShape() && "cannot get element count of dynamic shaped type");

  if (auto t = toolchain::dyn_cast_if_present<Type>(val))
    return cast<ShapedType>(t).getNumElements();

  if (auto attr = toolchain::dyn_cast_if_present<Attribute>(val)) {
    auto dattr = cast<DenseIntElementsAttr>(attr);
    int64_t num = 1;
    for (auto index : dattr.getValues<APInt>()) {
      num *= index.getZExtValue();
      assert(num >= 0 && "integer overflow in element count computation");
    }
    return num;
  }

  auto *stc = cast<ShapedTypeComponents *>(val);
  int64_t num = 1;
  for (int64_t dim : stc->getDims()) {
    num *= dim;
    assert(num >= 0 && "integer overflow in element count computation");
  }
  return num;
}

void ShapeAdaptor::dump() const {
  if (!hasRank()) {
    toolchain::errs() << "<<unranked>>\n";
    return;
  }

  SmallVector<int64_t> dims;
  getDims(dims);
  auto mapped = toolchain::map_range(dims, [](int64_t dim) -> std::string {
    if (ShapedType::isDynamic(dim))
      return "?";
    return toolchain::formatv("{0}", dim).str();
  });
  toolchain::errs() << "rank = " << getRank()
               << " dims = " << toolchain::interleaved_array(mapped, "x") << "\n";
}

ShapeAdaptor ValueShapeRange::getValueAsShape(int index) {
  Value val = operator[](index);
  if (valueToShape)
    if (ShapeAdaptor ret = valueToShape(val))
      return ret;

  DenseIntElementsAttr attr;
  if (!matchPattern(val, m_Constant(&attr)))
    return nullptr;
  if (attr.getType().getRank() != 1)
    return nullptr;
  return attr;
}

ShapeAdaptor ValueShapeRange::getShape(Value val) const {
  if (operandShape)
    if (ShapeAdaptor ret = operandShape(val))
      return ret;
  return val.getType();
}

ShapeAdaptor ValueShapeRange::getShape(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= size())
    return nullptr;
  return getShape(operator[](index));
}

LogicalResult mlir::detail::inferReturnTensorTypes(
    ArrayRef<ShapedTypeComponents> retComponents,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  for (const auto &shapeAndType : retComponents) {
    Type elementTy = shapeAndType.getElementType();
    assert(elementTy && "element type required to construct tensor");

    Attribute attr = shapeAndType.getAttribute();
    if (shapeAndType.hasRank()) {
      inferredReturnTypes.push_back(
          RankedTensorType::get(shapeAndType.getDims(), elementTy, attr));
    } else {
      assert(attr == nullptr && "attribute not supported");
      inferredReturnTypes.push_back(UnrankedTensorType::get(elementTy));
    }
  }
  return success();
}

LogicalResult mlir::detail::verifyInferredResultTypes(Operation *op) {
  SmallVector<Type, 4> inferredReturnTypes(op->getResultTypes());
  auto retTypeFn = cast<InferTypeOpInterface>(op);
  auto result = retTypeFn.refineReturnTypes(
      op->getContext(), op->getLoc(), op->getOperands(),
      op->getRawDictionaryAttrs(), op->getPropertiesStorage(), op->getRegions(),
      inferredReturnTypes);
  if (failed(result))
    op->emitOpError() << "failed to infer returned types";

  return result;
}

void mlir::detail::reportFatalInferReturnTypesError(OperationState &state) {
  std::string buffer;
  toolchain::raw_string_ostream os(buffer);
  os << "Failed to infer result type(s):\n"
     << "\"" << state.name << "\"(...) "
     << state.attributes.getDictionary(state.location.getContext()) << " : ("
     << toolchain::interleaved(toolchain::map_range(
            state.operands, [](Value val) { return val.getType(); }))
     << ") -> ( ??? )";
  emitRemark(state.location, "location of op");
  toolchain::report_fatal_error(toolchain::StringRef(buffer));
}
