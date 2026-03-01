//===- Unit.cpp - Support for manipulating IR Unit ------------------------===//
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

#include "mlir/IR/Unit.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/IR/Region.h"
#include "vm/core/Support/raw_ostream.h"
#include <iterator>

using namespace mlir;

static void printOp(toolchain::raw_ostream &os, Operation *op,
                    OpPrintingFlags &flags) {
  if (!op) {
    os << "<Operation:nullptr>";
    return;
  }
  op->print(os, flags);
}

static void printRegion(toolchain::raw_ostream &os, Region *region,
                        OpPrintingFlags &flags) {
  if (!region) {
    os << "<Region:nullptr>";
    return;
  }
  os << "Region #" << region->getRegionNumber() << " for op ";
  printOp(os, region->getParentOp(), flags);
}

static void printBlock(toolchain::raw_ostream &os, Block *block,
                       OpPrintingFlags &flags) {
  Region *region = block->getParent();
  os << "Block #" << block->computeBlockNumber() << " for ";
  bool shouldSkipRegions = flags.shouldSkipRegions();
  printRegion(os, region, flags.skipRegions());
  if (!shouldSkipRegions)
    block->print(os);
}

void mlir::IRUnit::print(toolchain::raw_ostream &os, OpPrintingFlags flags) const {
  if (auto *op = toolchain::dyn_cast_if_present<Operation *>(*this))
    return printOp(os, op, flags);
  if (auto *region = toolchain::dyn_cast_if_present<Region *>(*this))
    return printRegion(os, region, flags);
  if (auto *block = toolchain::dyn_cast_if_present<Block *>(*this))
    return printBlock(os, block, flags);
  llvm_unreachable("unknown IRUnit");
}

toolchain::raw_ostream &mlir::operator<<(toolchain::raw_ostream &os, const IRUnit &unit) {
  unit.print(os);
  return os;
}
