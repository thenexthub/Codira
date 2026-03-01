//===- RegionKindInterface.cpp - Region Kind Interfaces ---------*- C++ -*-===//
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
// This file contains the definitions of the region kind interfaces defined in
// `RegionKindInterface.td`.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/RegionKindInterface.h"

using namespace mlir;

#include "mlir/IR/RegionKindInterface.cpp.inc"

bool mlir::mayHaveSSADominance(Region &region) {
  auto regionKindOp = dyn_cast<RegionKindInterface>(region.getParentOp());
  if (!regionKindOp)
    return true;
  return regionKindOp.hasSSADominance(region.getRegionNumber());
}

bool mlir::mayBeGraphRegion(Region &region) {
  if (!region.getParentOp()->isRegistered())
    return true;
  auto regionKindOp = dyn_cast<RegionKindInterface>(region.getParentOp());
  if (!regionKindOp)
    return false;
  return !regionKindOp.hasSSADominance(region.getRegionNumber());
}
