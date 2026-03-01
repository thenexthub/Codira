
//===- DLTITransformOps.cpp - Implementation of DLTI transform ops --------===//
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

#include "mlir/Dialect/DLTI/TransformOps/DLTITransformOps.h"

#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/Dialect/Transform/Utils/Utils.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

using namespace mlir;
using namespace mlir::transform;

#define DEBUG_TYPE "dlti-transforms"

//===----------------------------------------------------------------------===//
// QueryOp
//===----------------------------------------------------------------------===//

void transform::QueryOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getTargetMutable(), effects);
  producesHandle(getOperation()->getOpResults(), effects);
  onlyReadsPayload(effects);
}

DiagnosedSilenceableFailure transform::QueryOp::applyToOne(
    transform::TransformRewriter &rewriter, Operation *target,
    transform::ApplyToEachResultList &results, TransformState &state) {
  SmallVector<DataLayoutEntryKey> keys;
  for (Attribute key : getKeys()) {
    if (auto strKey = dyn_cast<StringAttr>(key))
      keys.push_back(strKey);
    else if (auto typeKey = dyn_cast<TypeAttr>(key))
      keys.push_back(typeKey.getValue());
    else
      return emitDefiniteFailure("'transform.dlti.query' keys of wrong type: "
                                 "only StringAttr and TypeAttr are allowed");
  }

  FailureOr<Attribute> result = dlti::query(target, keys, /*emitError=*/true);

  if (failed(result))
    return emitSilenceableFailure(getLoc(),
                                  "'transform.dlti.query' op failed to apply");

  results.push_back(*result);
  return DiagnosedSilenceableFailure::success();
}

//===----------------------------------------------------------------------===//
// Transform op registration
//===----------------------------------------------------------------------===//

namespace {
class DLTITransformDialectExtension
    : public transform::TransformDialectExtension<
          DLTITransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(DLTITransformDialectExtension)

  using Base::Base;

  void init() {
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/DLTI/TransformOps/DLTITransformOps.cpp.inc"
        >();
  }
};
} // namespace

#define GET_OP_CLASSES
#include "mlir/Dialect/DLTI/TransformOps/DLTITransformOps.cpp.inc"

void mlir::dlti::registerTransformDialectExtension(DialectRegistry &registry) {
  registry.addExtensions<DLTITransformDialectExtension>();
}
