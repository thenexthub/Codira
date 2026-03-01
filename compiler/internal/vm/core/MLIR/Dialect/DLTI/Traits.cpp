//===- Traits.cpp - Traits for MLIR DLTI dialect --------------------------===//
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

#include "mlir/Dialect/DLTI/Traits.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

using namespace mlir;

LogicalResult mlir::impl::verifyHasDefaultDLTIDataLayoutTrait(Operation *op) {
  // TODO: consider having trait inheritance so that HasDefaultDLTIDataLayout
  // trait can inherit DataLayoutOpInterface::Trait and enforce the validity of
  // the assertion below.
  assert(
      isa<DataLayoutOpInterface>(op) &&
      "HasDefaultDLTIDataLayout trait unexpectedly attached to an op that does "
      "not implement DataLayoutOpInterface");
  return success();
}

DataLayoutSpecInterface mlir::impl::getDataLayoutSpec(Operation *op) {
  return op->getAttrOfType<DataLayoutSpecInterface>(
      DLTIDialect::kDataLayoutAttrName);
}

TargetSystemSpecInterface mlir::impl::getTargetSystemSpec(Operation *op) {
  return op->getAttrOfType<TargetSystemSpecAttr>(
      DLTIDialect::kTargetSystemDescAttrName);
}
