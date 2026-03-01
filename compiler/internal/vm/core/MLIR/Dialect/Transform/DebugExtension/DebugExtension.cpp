//===- DebugExtension.cpp - Debug extension for the Transform dialect -----===//
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

#include "mlir/Dialect/Transform/DebugExtension/DebugExtension.h"

#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/IR/DialectRegistry.h"

using namespace mlir;

namespace {
/// Debug extension of the Transform dialect. This provides operations for
/// debugging transform dialect scripts.
class DebugExtension
    : public transform::TransformDialectExtension<DebugExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(DebugExtension)

  void init() {
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/Transform/DebugExtension/DebugExtensionOps.cpp.inc"
        >();
  }
};
} // namespace

void mlir::transform::registerDebugExtension(DialectRegistry &dialectRegistry) {
  dialectRegistry.addExtensions<DebugExtension>();
}
