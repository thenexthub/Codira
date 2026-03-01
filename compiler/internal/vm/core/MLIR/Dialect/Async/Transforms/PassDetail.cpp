//===- PassDetail.cpp - Async Pass class details ----------------*- C++ -*-===//
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

#include "PassDetail.h"
#include "mlir/IR/Builders.h"
#include "mlir/Transforms/RegionUtils.h"

using namespace mlir;

void mlir::async::cloneConstantsIntoTheRegion(Region &region) {
  OpBuilder builder(&region);
  cloneConstantsIntoTheRegion(region, builder);
}

void mlir::async::cloneConstantsIntoTheRegion(Region &region,
                                              OpBuilder &builder) {
  // Values implicitly captured by the region.
  toolchain::SetVector<Value> captures;
  getUsedValuesDefinedAbove(region, region, captures);

  OpBuilder::InsertionGuard guard(builder);
  builder.setInsertionPointToStart(&region.front());

  // Clone ConstantLike operations into the region.
  for (Value capture : captures) {
    Operation *op = capture.getDefiningOp();
    if (!op || !op->hasTrait<OpTrait::ConstantLike>())
      continue;

    Operation *cloned = builder.clone(*op);

    for (auto tuple : toolchain::zip(op->getResults(), cloned->getResults())) {
      Value orig = std::get<0>(tuple);
      Value replacement = std::get<1>(tuple);
      replaceAllUsesInRegionWith(orig, replacement, region);
    }
  }
}
