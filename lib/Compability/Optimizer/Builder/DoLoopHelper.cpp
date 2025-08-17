//===-- DoLoopHelper.cpp --------------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/DoLoopHelper.h"

//===----------------------------------------------------------------------===//
// DoLoopHelper implementation
//===----------------------------------------------------------------------===//

fir::DoLoopOp
fir::factory::DoLoopHelper::createLoop(mlir::Value lb, mlir::Value ub,
                                       mlir::Value step,
                                       const BodyGenerator &bodyGenerator) {
  auto lbi = builder.convertToIndexType(loc, lb);
  auto ubi = builder.convertToIndexType(loc, ub);
  assert(step && "step must be an actual Value");
  auto inc = builder.convertToIndexType(loc, step);
  auto loop = fir::DoLoopOp::create(builder, loc, lbi, ubi, inc);
  auto insertPt = builder.saveInsertionPoint();
  builder.setInsertionPointToStart(loop.getBody());
  auto index = loop.getInductionVar();
  bodyGenerator(builder, index);
  builder.restoreInsertionPoint(insertPt);
  return loop;
}

fir::DoLoopOp
fir::factory::DoLoopHelper::createLoop(mlir::Value lb, mlir::Value ub,
                                       const BodyGenerator &bodyGenerator) {
  return createLoop(
      lb, ub, builder.createIntegerConstant(loc, builder.getIndexType(), 1),
      bodyGenerator);
}

fir::DoLoopOp
fir::factory::DoLoopHelper::createLoop(mlir::Value count,
                                       const BodyGenerator &bodyGenerator) {
  auto indexType = builder.getIndexType();
  auto zero = builder.createIntegerConstant(loc, indexType, 0);
  auto one = builder.createIntegerConstant(loc, count.getType(), 1);
  auto up = mlir::arith::SubIOp::create(builder, loc, count, one);
  return createLoop(zero, up, one, bodyGenerator);
}
