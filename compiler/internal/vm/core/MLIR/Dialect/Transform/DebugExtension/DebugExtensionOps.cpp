//===- DebugExtensionOps.cpp - Debug extension for the Transform dialect --===//
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

#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.h"

#include "mlir/Dialect/Transform/IR/TransformTypes.h"
#include "vm/core/Support/InterleavedRange.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.cpp.inc"

DiagnosedSilenceableFailure
transform::EmitRemarkAtOp::apply(transform::TransformRewriter &rewriter,
                                 transform::TransformResults &results,
                                 transform::TransformState &state) {
  if (isa<TransformHandleTypeInterface>(getAt().getType())) {
    auto payload = state.getPayloadOps(getAt());
    for (Operation *op : payload)
      op->emitRemark() << getMessage();
    return DiagnosedSilenceableFailure::success();
  }

  assert(isa<transform::TransformValueHandleTypeInterface>(getAt().getType()) &&
         "unhandled kind of transform type");

  auto describeValue = [](Diagnostic &os, Value value) {
    os << "value handle points to ";
    if (auto arg = toolchain::dyn_cast<BlockArgument>(value)) {
      os << "a block argument #" << arg.getArgNumber() << " in block #"
         << arg.getOwner()->computeBlockNumber() << " in region #"
         << arg.getOwner()->getParent()->getRegionNumber();
    } else {
      os << "an op result #" << toolchain::cast<OpResult>(value).getResultNumber();
    }
  };

  for (Value value : state.getPayloadValues(getAt())) {
    InFlightDiagnostic diag = ::emitRemark(value.getLoc()) << getMessage();
    describeValue(diag.attachNote(), value);
  }

  return DiagnosedSilenceableFailure::success();
}

DiagnosedSilenceableFailure
transform::EmitParamAsRemarkOp::apply(transform::TransformRewriter &rewriter,
                                      transform::TransformResults &results,
                                      transform::TransformState &state) {
  std::string str;
  toolchain::raw_string_ostream os(str);
  if (getMessage())
    os << *getMessage() << " ";
  os << toolchain::interleaved(state.getParams(getParam()));
  if (!getAnchor()) {
    emitRemark() << str;
    return DiagnosedSilenceableFailure::success();
  }
  for (Operation *payload : state.getPayloadOps(getAnchor()))
    ::mlir::emitRemark(payload->getLoc()) << str;
  return DiagnosedSilenceableFailure::success();
}
