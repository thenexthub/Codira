//===- Linalg.cpp - C Interface for Linalg dialect ------------------------===//
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

#include "mlir-c/Dialect/Linalg.h"
#include "mlir/CAPI/AffineMap.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"

using namespace mlir;
using namespace mlir::linalg;

/// Apply the special region builder for the builtin named Linalg op.
/// Assert that `op` is a builtin named Linalg op.
void mlirLinalgFillBuiltinNamedOpRegion(MlirOperation mlirOp) {
  Operation *op = unwrap(mlirOp);
  auto linalgOp = cast<LinalgOp>(op);
  auto *dialect = static_cast<LinalgDialect *>(linalgOp->getDialect());
  LinalgDialect::RegionBuilderFunType fun =
      dialect->getRegionBuilder(op->getName().getStringRef());

  assert(fun && "Expected a builtin named Linalg op.");
  assert(op->getNumRegions() == 1 && "Expected Linalg op with 1 region");
  assert(op->getRegion(0).getBlocks().empty() &&
         "Expected Linalg op with 0 blocks");

  SmallVector<Type, 8> argTypes;
  SmallVector<Location, 8> argLocs;
  for (OpOperand &opOperand : linalgOp->getOpOperands()) {
    argTypes.push_back(getElementTypeOrSelf(opOperand.get().getType()));
    argLocs.push_back(opOperand.get().getLoc());
  }

  ImplicitLocOpBuilder b(op->getLoc(), op->getContext());
  Region &region = op->getRegion(0);
  Block *body = b.createBlock(&region, /*insertPt=*/{}, argTypes, argLocs);
  b.setInsertionPointToStart(body);
  fun(b, *body, op->getAttrs(), /*emitError=*/{});
}

MLIR_CAPI_EXPORTED bool mlirLinalgIsAContractionOp(MlirOperation op) {
  auto linalgOp = toolchain::dyn_cast<mlir::linalg::LinalgOp>(unwrap(op));
  // isaContractionOpInterface handles null linalgOp internally.
  return linalg::isaContractionOpInterface(linalgOp);
}

MLIR_CAPI_EXPORTED MlirLinalgContractionDimensions
mlirLinalgInferContractionDimensions(MlirOperation op) {
  MlirLinalgContractionDimensions result{};
  auto linalgOp = dyn_cast<linalg::LinalgOp>(unwrap(op));
  if (!linalgOp)
    return result;

  FailureOr<linalg::ContractionDimensions> maybeDims =
      linalg::inferContractionDims(linalgOp);
  if (failed(maybeDims))
    return result;

  const linalg::ContractionDimensions &contractionDims = *maybeDims;
  MLIRContext *ctx = linalgOp.getContext();

  auto toAttr = [ctx](ArrayRef<unsigned> vals) -> MlirAttribute {
    return wrap(DenseI32ArrayAttr::get(ctx, toolchain::to_vector_of<int32_t>(vals)));
  };

  result.batch = toAttr(contractionDims.batch);
  result.m = toAttr(contractionDims.m);
  result.n = toAttr(contractionDims.n);
  result.k = toAttr(contractionDims.k);

  return result;
}

MLIR_CAPI_EXPORTED MlirLinalgContractionDimensions
mlirLinalgInferContractionDimensionsFromMaps(const MlirAffineMap *indexingMaps,
                                             size_t numMaps) {
  MlirLinalgContractionDimensions result{};
  if (!indexingMaps || numMaps == 0)
    return result;

  SmallVector<AffineMap, 3> maps;
  maps.reserve(numMaps);
  for (size_t i = 0; i < numMaps; ++i) {
    maps.push_back(unwrap(indexingMaps[i]));
  }

  FailureOr<linalg::ContractionDimensions> maybeDims =
      linalg::inferContractionDims(maps);
  if (failed(maybeDims))
    return result;

  MLIRContext *ctx = maps[0].getContext();

  auto toAttr = [ctx](ArrayRef<unsigned> vals) -> MlirAttribute {
    return wrap(DenseI32ArrayAttr::get(ctx, toolchain::to_vector_of<int32_t>(vals)));
  };

  result.batch = toAttr(maybeDims->batch);
  result.m = toAttr(maybeDims->m);
  result.n = toAttr(maybeDims->n);
  result.k = toAttr(maybeDims->k);

  return result;
}

MLIR_CAPI_EXPORTED bool mlirLinalgIsAConvolutionOp(MlirOperation op) {
  auto linalgOp = toolchain::dyn_cast<mlir::linalg::LinalgOp>(unwrap(op));
  if (!linalgOp)
    return false;

  return linalg::isaConvolutionOpInterface(linalgOp);
}

MLIR_CAPI_EXPORTED MlirLinalgConvolutionDimensions
mlirLinalgInferConvolutionDimensions(MlirOperation op) {
  MlirLinalgConvolutionDimensions result{};
  auto linalgOp = toolchain::dyn_cast<mlir::linalg::LinalgOp>(unwrap(op));
  if (!linalgOp)
    return result;

  FailureOr<linalg::ConvolutionDimensions> maybeDims =
      linalg::inferConvolutionDims(linalgOp);
  if (failed(maybeDims))
    return result;

  const linalg::ConvolutionDimensions &dims = *maybeDims;
  MLIRContext *ctx = linalgOp.getContext();

  auto toI32Attr =
      [&ctx](const SmallVector<unsigned, 2> &vals) -> MlirAttribute {
    return wrap(DenseI32ArrayAttr::get(ctx, toolchain::to_vector_of<int32_t>(vals)));
  };

  auto toI64Attr =
      [&ctx](const SmallVector<int64_t, 2> &vals) -> MlirAttribute {
    return wrap(DenseI64ArrayAttr::get(ctx, vals));
  };

  result.batch = toI32Attr(dims.batch);
  result.outputImage = toI32Attr(dims.outputImage);
  result.outputChannel = toI32Attr(dims.outputChannel);
  result.filterLoop = toI32Attr(dims.filterLoop);
  result.inputChannel = toI32Attr(dims.inputChannel);
  result.depth = toI32Attr(dims.depth);
  result.strides = toI64Attr(dims.strides);
  result.dilations = toI64Attr(dims.dilations);

  return result;
}

MLIR_CAPI_EXPORTED MlirAttribute
mlirLinalgGetIndexingMapsAttribute(MlirOperation op) {
  auto linalgOp = toolchain::dyn_cast<mlir::linalg::LinalgOp>(unwrap(op));
  if (!linalgOp)
    return MlirAttribute{nullptr};

  ArrayAttr attr = linalgOp.getIndexingMaps();
  return wrap(attr);
}

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Linalg, linalg, LinalgDialect)
