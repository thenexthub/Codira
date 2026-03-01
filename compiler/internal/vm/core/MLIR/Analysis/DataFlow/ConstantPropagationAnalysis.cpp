//===- ConstantPropagationAnalysis.cpp - Constant propagation analysis ----===//
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

#include "mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/DebugLog.h"
#include <cassert>

#define DEBUG_TYPE "constant-propagation"

using namespace mlir;
using namespace mlir::dataflow;

//===----------------------------------------------------------------------===//
// ConstantValue
//===----------------------------------------------------------------------===//

void ConstantValue::print(raw_ostream &os) const {
  if (isUninitialized()) {
    os << "<UNINITIALIZED>";
    return;
  }
  if (getConstantValue() == nullptr) {
    os << "<UNKNOWN>";
    return;
  }
  return getConstantValue().print(os);
}

//===----------------------------------------------------------------------===//
// SparseConstantPropagation
//===----------------------------------------------------------------------===//

LogicalResult SparseConstantPropagation::visitOperation(
    Operation *op, ArrayRef<const Lattice<ConstantValue> *> operands,
    ArrayRef<Lattice<ConstantValue> *> results) {
  LDBG() << "SCP: Visiting operation: " << *op;

  // Don't try to simulate the results of a region operation as we can't
  // guarantee that folding will be out-of-place. We don't allow in-place
  // folds as the desire here is for simulated execution, and not general
  // folding.
  if (op->getNumRegions()) {
    setAllToEntryStates(results);
    return success();
  }

  SmallVector<Attribute, 8> constantOperands;
  constantOperands.reserve(op->getNumOperands());
  for (auto *operandLattice : operands) {
    if (operandLattice->getValue().isUninitialized())
      return success();
    constantOperands.push_back(operandLattice->getValue().getConstantValue());
  }

  // Save the original operands and attributes just in case the operation
  // folds in-place. The constant passed in may not correspond to the real
  // runtime value, so in-place updates are not allowed.
  SmallVector<Value, 8> originalOperands(op->getOperands());
  DictionaryAttr originalAttrs = op->getAttrDictionary();

  // Simulate the result of folding this operation to a constant. If folding
  // fails or was an in-place fold, mark the results as overdefined.
  SmallVector<OpFoldResult, 8> foldResults;
  foldResults.reserve(op->getNumResults());
  if (failed(op->fold(constantOperands, foldResults))) {
    setAllToEntryStates(results);
    return success();
  }

  // If the folding was in-place, mark the results as overdefined and reset
  // the operation. We don't allow in-place folds as the desire here is for
  // simulated execution, and not general folding.
  if (foldResults.empty()) {
    op->setOperands(originalOperands);
    op->setAttrs(originalAttrs);
    setAllToEntryStates(results);
    return success();
  }

  // Merge the fold results into the lattice for this operation.
  assert(foldResults.size() == op->getNumResults() && "invalid result size");
  for (const auto it : toolchain::zip(results, foldResults)) {
    Lattice<ConstantValue> *lattice = std::get<0>(it);

    // Merge in the result of the fold, either a constant or a value.
    OpFoldResult foldResult = std::get<1>(it);
    if (Attribute attr = toolchain::dyn_cast_if_present<Attribute>(foldResult)) {
      LDBG() << "Folded to constant: " << attr;
      propagateIfChanged(lattice,
                         lattice->join(ConstantValue(attr, op->getDialect())));
    } else {
      LDBG() << "Folded to value: " << cast<Value>(foldResult);
      AbstractSparseForwardDataFlowAnalysis::join(
          lattice, *getLatticeElement(cast<Value>(foldResult)));
    }
  }
  return success();
}

void SparseConstantPropagation::setToEntryState(
    Lattice<ConstantValue> *lattice) {
  propagateIfChanged(lattice,
                     lattice->join(ConstantValue::getUnknownConstant()));
}
