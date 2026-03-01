//===- TransformsDetail.h - -------------------------------------*- C++ -*-===//
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

#ifndef MLIR_DIALECT_SHARD_TRANSFORMS_TRANSFORMSDETAIL_H
#define MLIR_DIALECT_SHARD_TRANSFORMS_TRANSFORMSDETAIL_H

#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/SymbolTable.h"

namespace mlir {
namespace shard {

template <typename Op>
struct OpRewritePatternWithSymbolTableCollection : OpRewritePattern<Op> {
  template <typename... OpRewritePatternArgs>
  OpRewritePatternWithSymbolTableCollection(
      SymbolTableCollection &symbolTableCollection,
      OpRewritePatternArgs &&...opRewritePatternArgs)
      : OpRewritePattern<Op>(
            std::forward<OpRewritePatternArgs...>(opRewritePatternArgs)...),
        symbolTableCollection(symbolTableCollection) {}

protected:
  SymbolTableCollection &symbolTableCollection;
};

} // namespace shard
} // namespace mlir

#endif // MLIR_DIALECT_SHARD_TRANSFORMS_TRANSFORMSDETAIL_H
