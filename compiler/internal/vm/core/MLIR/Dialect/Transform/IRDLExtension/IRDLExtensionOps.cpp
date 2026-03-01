//===- IRDLExtensionOps.cpp - IRDL extension for the Transform dialect ----===//
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

#include "mlir/Dialect/Transform/IRDLExtension/IRDLExtensionOps.h"
#include "mlir/Dialect/IRDL/IR/IRDL.h"
#include "mlir/Dialect/IRDL/IRDLVerifiers.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/ExtensibleDialect.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "vm/core/ADT/STLExtras.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/IRDLExtension/IRDLExtensionOps.cpp.inc"

namespace mlir::transform {

DiagnosedSilenceableFailure
IRDLCollectMatchingOp::apply(TransformRewriter &rewriter,
                             TransformResults &results, TransformState &state) {
  auto dialect = cast<irdl::DialectOp>(getBody().front().front());
  Block &body = dialect.getBody().front();
  irdl::OperationOp operation = *body.getOps<irdl::OperationOp>().begin();
  auto verifier = irdl::createVerifier(
      operation,
      DenseMap<irdl::TypeOp, std::unique_ptr<DynamicTypeDefinition>>(),
      DenseMap<irdl::AttributeOp, std::unique_ptr<DynamicAttrDefinition>>());

  auto handlerID = getContext()->getDiagEngine().registerHandler(
      [](Diagnostic &) { return success(); });
  SmallVector<Operation *> matched;
  for (Operation *payload : state.getPayloadOps(getRoot())) {
    payload->walk([&](Operation *target) {
      if (succeeded(verifier(target))) {
        matched.push_back(target);
      }
    });
  }
  getContext()->getDiagEngine().eraseHandler(handlerID);
  results.set(cast<OpResult>(getMatched()), matched);
  return DiagnosedSilenceableFailure::success();
}

void IRDLCollectMatchingOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getRootMutable(), effects);
  producesHandle(getOperation()->getOpResults(), effects);
  onlyReadsPayload(effects);
}

LogicalResult IRDLCollectMatchingOp::verify() {
  Block &bodyBlock = getBody().front();
  if (!toolchain::hasSingleElement(bodyBlock))
    return emitOpError() << "expects a single operation in the body";

  auto dialect = dyn_cast<irdl::DialectOp>(bodyBlock.front());
  if (!dialect) {
    return emitOpError() << "expects the body operation to be "
                         << irdl::DialectOp::getOperationName();
  }

  // TODO: relax this by taking a symbol name of the operation to match, note
  // that symbol name is also the name of the operation and we may want to
  // divert from that to have constraints on-the-fly using IRDL.
  auto irdlOperations = dialect.getOps<irdl::OperationOp>();
  if (!toolchain::hasSingleElement(irdlOperations))
    return emitOpError() << "expects IRDL to contain exactly one operation";

  if (!dialect.getOps<irdl::TypeOp>().empty() ||
      !dialect.getOps<irdl::AttributeOp>().empty()) {
    return emitOpError() << "IRDL types and attributes are not yet supported";
  }

  return success();
}

} // namespace mlir::transform
