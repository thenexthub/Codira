//===- DataFlowFramework.cpp - A generic framework for data-flow analysis -===//
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

#include "mlir/Analysis/DataFlowFramework.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/IR/Value.h"
#include "vm/core/ADT/ScopeExit.h"
#include "vm/core/ADT/iterator.h"
#include "vm/core/Config/abi-breaking.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/DebugLog.h"
#include "vm/core/Support/raw_ostream.h"

#define DEBUG_TYPE "dataflow"
#if LLVM_ENABLE_ABI_BREAKING_CHECKS
#define DATAFLOW_DEBUG(X) LLVM_DEBUG(X)
#else
#define DATAFLOW_DEBUG(X)
#endif // LLVM_ENABLE_ABI_BREAKING_CHECKS

using namespace mlir;

//===----------------------------------------------------------------------===//
// GenericLatticeAnchor
//===----------------------------------------------------------------------===//

GenericLatticeAnchor::~GenericLatticeAnchor() = default;

//===----------------------------------------------------------------------===//
// AnalysisState
//===----------------------------------------------------------------------===//

AnalysisState::~AnalysisState() = default;

void AnalysisState::addDependency(ProgramPoint *dependent,
                                  DataFlowAnalysis *analysis) {
  auto inserted = dependents.insert({dependent, analysis});
  (void)inserted;
  DATAFLOW_DEBUG({
    if (inserted) {
      LDBG() << "Creating dependency between " << debugName << " of " << anchor
             << "\nand " << debugName << " on " << *dependent;
    }
  });
}

void AnalysisState::dump() const { print(toolchain::errs()); }

//===----------------------------------------------------------------------===//
// ProgramPoint
//===----------------------------------------------------------------------===//

void ProgramPoint::print(raw_ostream &os) const {
  if (isNull()) {
    os << "<NULL POINT>";
    return;
  }
  if (!isBlockStart()) {
    os << "<after operation>:"
       << OpWithFlags(getPrevOp(), OpPrintingFlags().skipRegions());
    return;
  }
  if (!isBlockEnd()) {
    os << "<before operation>:"
       << OpWithFlags(getNextOp(), OpPrintingFlags().skipRegions());
    return;
  }
  os << "<beginning of empty block>";
}

//===----------------------------------------------------------------------===//
// LatticeAnchor
//===----------------------------------------------------------------------===//

void LatticeAnchor::print(raw_ostream &os) const {
  if (isNull()) {
    os << "<NULL POINT>";
    return;
  }
  if (auto *latticeAnchor = toolchain::dyn_cast<GenericLatticeAnchor *>(*this))
    return latticeAnchor->print(os);
  if (auto value = toolchain::dyn_cast<Value>(*this)) {
    return value.print(os, OpPrintingFlags().skipRegions());
  }

  return toolchain::cast<ProgramPoint *>(*this)->print(os);
}

Location LatticeAnchor::getLoc() const {
  if (auto *latticeAnchor = toolchain::dyn_cast<GenericLatticeAnchor *>(*this))
    return latticeAnchor->getLoc();
  if (auto value = toolchain::dyn_cast<Value>(*this))
    return value.getLoc();

  ProgramPoint *pp = toolchain::cast<ProgramPoint *>(*this);
  if (!pp->isBlockStart())
    return pp->getPrevOp()->getLoc();
  return pp->getBlock()->getParent()->getLoc();
}

//===----------------------------------------------------------------------===//
// DataFlowSolver
//===----------------------------------------------------------------------===//

LogicalResult DataFlowSolver::initializeAndRun(Operation *top) {
  // Enable enqueue to the worklist.
  isRunning = true;
  toolchain::scope_exit guard([&]() { isRunning = false; });

  bool isInterprocedural = config.isInterprocedural();
  toolchain::scope_exit restoreInterprocedural(
      [&]() { config.setInterprocedural(isInterprocedural); });
  if (isInterprocedural && !top->hasTrait<OpTrait::SymbolTable>())
    config.setInterprocedural(false);

  // Initialize equivalent lattice anchors.
  for (DataFlowAnalysis &analysis : toolchain::make_pointee_range(childAnalyses)) {
    analysis.initializeEquivalentLatticeAnchor(top);
  }

  // Initialize the analyses.
  for (DataFlowAnalysis &analysis : toolchain::make_pointee_range(childAnalyses)) {
    DATAFLOW_DEBUG(LDBG() << "Priming analysis: " << analysis.debugName);
    if (failed(analysis.initialize(top)))
      return failure();
  }

  // Run the analysis until fixpoint.
  // Iterate until all states are in some initialized state and the worklist
  // is exhausted.
  while (!worklist.empty()) {
    auto [point, analysis] = worklist.front();
    worklist.pop();

    DATAFLOW_DEBUG(LDBG() << "Invoking '" << analysis->debugName
                          << "' on: " << *point);
    if (failed(analysis->visit(point)))
      return failure();
  }

  return success();
}

void DataFlowSolver::propagateIfChanged(AnalysisState *state,
                                        ChangeResult changed) {
  assert(isRunning &&
         "DataFlowSolver is not running, should not use propagateIfChanged");
  if (changed == ChangeResult::Change) {
    DATAFLOW_DEBUG(LDBG() << "Propagating update to " << state->debugName
                          << " of " << state->anchor << "\n"
                          << "Value: " << *state);
    state->onUpdate(this);
  }
}

//===----------------------------------------------------------------------===//
// DataFlowAnalysis
//===----------------------------------------------------------------------===//

DataFlowAnalysis::~DataFlowAnalysis() = default;

DataFlowAnalysis::DataFlowAnalysis(DataFlowSolver &solver) : solver(solver) {}

void DataFlowAnalysis::addDependency(AnalysisState *state,
                                     ProgramPoint *point) {
  state->addDependency(point, this);
}

void DataFlowAnalysis::propagateIfChanged(AnalysisState *state,
                                          ChangeResult changed) {
  solver.propagateIfChanged(state, changed);
}
