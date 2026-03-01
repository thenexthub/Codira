//===- SymbolPrivatize.cpp - Pass to mark symbols private -----------------===//
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
// This file implements an pass that marks all symbols as private unless
// excluded.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/SymbolTable.h"

namespace mlir {
#define GEN_PASS_DEF_SYMBOLPRIVATIZE
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct SymbolPrivatize : public impl::SymbolPrivatizeBase<SymbolPrivatize> {
  explicit SymbolPrivatize(ArrayRef<std::string> excludeSymbols);
  LogicalResult initialize(MLIRContext *context) override;
  void runOnOperation() override;

  /// Symbols whose visibility won't be changed.
  DenseSet<StringAttr> excludedSymbols;
};
} // namespace

SymbolPrivatize::SymbolPrivatize(toolchain::ArrayRef<std::string> excludeSymbols) {
  exclude = excludeSymbols;
}

LogicalResult SymbolPrivatize::initialize(MLIRContext *context) {
  for (const std::string &symbol : exclude)
    excludedSymbols.insert(StringAttr::get(context, symbol));
  return success();
}

void SymbolPrivatize::runOnOperation() {
  for (Region &region : getOperation()->getRegions()) {
    for (Block &block : region) {
      for (Operation &op : block) {
        auto symbol = dyn_cast<SymbolOpInterface>(op);
        if (!symbol)
          continue;
        if (!excludedSymbols.contains(symbol.getNameAttr()))
          symbol.setVisibility(SymbolTable::Visibility::Private);
      }
    }
  }
}

std::unique_ptr<Pass>
mlir::createSymbolPrivatizePass(ArrayRef<std::string> exclude) {
  return std::make_unique<SymbolPrivatize>(exclude);
}
