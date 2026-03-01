//===- DataLayoutAnalysis.cpp ---------------------------------------------===//
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

#include "mlir/Analysis/DataLayoutAnalysis.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/Support/LLVM.h"
#include <memory>

using namespace mlir;

DataLayoutAnalysis::DataLayoutAnalysis(Operation *root)
    : defaultLayout(std::make_unique<DataLayout>(DataLayoutOpInterface())) {
  // Construct a DataLayout if possible from the op.
  auto computeLayout = [this](Operation *op) {
    if (auto iface = dyn_cast<DataLayoutOpInterface>(op))
      layouts[op] = std::make_unique<DataLayout>(iface);
    if (auto module = dyn_cast<ModuleOp>(op))
      layouts[op] = std::make_unique<DataLayout>(module);
  };

  // Compute layouts for both ancestors and descendants.
  root->walk(computeLayout);
  for (Operation *ancestor = root->getParentOp(); ancestor != nullptr;
       ancestor = ancestor->getParentOp()) {
    computeLayout(ancestor);
  }
}

const DataLayout &DataLayoutAnalysis::getAbove(Operation *operation) const {
  for (Operation *ancestor = operation->getParentOp(); ancestor != nullptr;
       ancestor = ancestor->getParentOp()) {
    auto it = layouts.find(ancestor);
    if (it != layouts.end())
      return *it->getSecond();
  }

  // Fallback to the default layout.
  return *defaultLayout;
}

const DataLayout &DataLayoutAnalysis::getAtOrAbove(Operation *operation) const {
  auto it = layouts.find(operation);
  if (it != layouts.end())
    return *it->getSecond();
  return getAbove(operation);
}
