//===- TuneExtension.cpp - Tune extension for the Transform dialect -------===//
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

#include "mlir/Dialect/Transform/TuneExtension/TuneExtension.h"

#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/TuneExtension/TuneExtensionOps.h"
#include "mlir/IR/DialectRegistry.h"

using namespace mlir;

class TuneExtension
    : public transform::TransformDialectExtension<TuneExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TuneExtension)

  void init() {
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/Transform/TuneExtension/TuneExtensionOps.cpp.inc"
        >();
  }
};

void mlir::transform::registerTuneExtension(DialectRegistry &dialectRegistry) {
  dialectRegistry.addExtensions<TuneExtension>();
}
