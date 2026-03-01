//===- InferIntRangeInterfaceImpls.cpp - Integer range impls for affine --===//
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

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Interfaces/InferIntRangeInterface.h"
#include "mlir/Interfaces/Utils/InferIntRangeCommon.h"

using namespace mlir;
using namespace mlir::affine;
using namespace mlir::intrange;

//===----------------------------------------------------------------------===//
// AffineApplyOp
//===----------------------------------------------------------------------===//

void AffineApplyOp::inferResultRanges(ArrayRef<ConstantIntRanges> argRanges,
                                      SetIntRangeFn setResultRange) {
  AffineMap map = getAffineMap();

  // Split operand ranges into dimensions and symbols.
  unsigned numDims = map.getNumDims();
  ArrayRef<ConstantIntRanges> dimRanges = argRanges.take_front(numDims);
  ArrayRef<ConstantIntRanges> symbolRanges = argRanges.drop_front(numDims);

  // Affine maps should have exactly one result for affine.apply.
  assert(map.getNumResults() == 1 && "affine.apply must have single result");

  // Infer the range for the affine expression.
  ConstantIntRanges resultRange =
      inferAffineExpr(map.getResult(0), dimRanges, symbolRanges);

  setResultRange(getResult(), resultRange);
}
