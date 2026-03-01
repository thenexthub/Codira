//===- PreloadLibraryPass.cpp - Pass to preload a transform library -------===//
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

#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/Transforms/Passes.h"
#include "mlir/Dialect/Transform/Transforms/TransformInterpreterUtils.h"

using namespace mlir;

namespace mlir {
namespace transform {
#define GEN_PASS_DEF_PRELOADLIBRARYPASS
#include "mlir/Dialect/Transform/Transforms/Passes.h.inc"
} // namespace transform
} // namespace mlir

namespace {
class PreloadLibraryPass
    : public transform::impl::PreloadLibraryPassBase<PreloadLibraryPass> {
public:
  using Base::Base;

  void runOnOperation() override {
    OwningOpRef<ModuleOp> mergedParsedLibraries;
    if (failed(transform::detail::assembleTransformLibraryFromPaths(
            &getContext(), transformLibraryPaths, mergedParsedLibraries)))
      return signalPassFailure();
    // TODO: investigate using a resource blob if some ownership mode allows it.
    auto *dialect =
        getContext().getOrLoadDialect<transform::TransformDialect>();
    if (failed(
            dialect->loadIntoLibraryModule(std::move(mergedParsedLibraries))))
      signalPassFailure();
  }
};
} // namespace
