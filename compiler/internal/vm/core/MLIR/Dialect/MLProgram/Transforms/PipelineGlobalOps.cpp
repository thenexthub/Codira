//===- PipelineGlobalOpsPass.cpp - Pipeline Global Ops Pass ---------------===//
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

#include "mlir/Dialect/MLProgram/Transforms/Passes.h"

#include "mlir/Dialect/MLProgram/IR/MLProgram.h"
#include "mlir/IR/BuiltinOps.h"

namespace mlir {
namespace ml_program {
#define GEN_PASS_DEF_MLPROGRAMPIPELINEGLOBALSPASS
#include "mlir/Dialect/MLProgram/Transforms/Passes.h.inc"

namespace {

class MLProgramPipelineGlobals
    : public impl::MLProgramPipelineGlobalsPassBase<MLProgramPipelineGlobals> {
public:
  void runOnOperation() override;

private:
  LogicalResult buildGlobalMap(ModuleOp op);

  void processBlock(Block &block, toolchain::DenseSet<SymbolRefAttr> &symbolLoad,
                    toolchain::DenseSet<SymbolRefAttr> &symbolStore);

  toolchain::DenseMap<SymbolRefAttr, toolchain::DenseSet<SymbolRefAttr>> loadSymbolsMap;
  toolchain::DenseMap<SymbolRefAttr, toolchain::DenseSet<SymbolRefAttr>> storeSymbolsMap;
};

// Traverses upwards searchign for the operation mapped by the symbol.
static Operation *getFromSymbol(Operation *baseOp, SymbolRefAttr symbol) {
  for (auto *op = baseOp; op; op = op->getParentOp()) {
    auto *lookup = SymbolTable::lookupNearestSymbolFrom(op, symbol);
    if (lookup)
      return lookup;
  }
  return nullptr;
}

// Builds map from a symbol to MLProgram global symbols loaded or stored
// during processing.
LogicalResult MLProgramPipelineGlobals::buildGlobalMap(ModuleOp module) {
  toolchain::DenseMap<SymbolRefAttr, Operation *> callableMap;
  auto res = module->walk([&](Operation *op) {
    if (auto caller = mlir::dyn_cast<CallOpInterface>(op)) {
      auto callable = caller.getCallableForCallee();
      // For now we do not know how to handle Value based tracing, so fail.
      if (mlir::isa<Value>(callable)) {
        return WalkResult::interrupt();
      }

      auto symbol = mlir::dyn_cast<SymbolRefAttr>(callable);
      auto *func = getFromSymbol(op, symbol);
      callableMap[symbol] = func;
    }
    return WalkResult::advance();
  });

  if (res.wasInterrupted()) {
    return failure();
  }

  // First grab all symbols loaded or stored by each function. This
  // will not handle calls initially.
  toolchain::DenseMap<SymbolRefAttr, toolchain::DenseSet<SymbolRefAttr>> opLoadSymbols;
  toolchain::DenseMap<SymbolRefAttr, toolchain::DenseSet<SymbolRefAttr>> opStoreSymbols;
  for (auto callable : callableMap) {
    toolchain::DenseSet<SymbolRefAttr> loadSymbols;
    toolchain::DenseSet<SymbolRefAttr> storeSymbols;

    callable.getSecond()->walk(
        [&](GlobalLoadOp op) { loadSymbols.insert(op.getGlobal()); });

    callable.getSecond()->walk(
        [&](GlobalStoreOp op) { storeSymbols.insert(op.getGlobal()); });

    opLoadSymbols[callable.getFirst()] = std::move(loadSymbols);
    opStoreSymbols[callable.getFirst()] = std::move(storeSymbols);
  }

  // For each callable function we find each global loaded/stored within the
  // function or a nested called function. This includes recursion checking to
  // avoid infinitely recursing.
  for (auto callable : callableMap) {
    SymbolRefAttr thisSymbol = toolchain::dyn_cast<SymbolRefAttr>(callable.first);
    toolchain::SmallVector<SymbolRefAttr> work = {thisSymbol};
    toolchain::DenseSet<SymbolRefAttr> visited = {thisSymbol};
    toolchain::DenseSet<SymbolRefAttr> loadSymbols;
    toolchain::DenseSet<SymbolRefAttr> storeSymbols;

    for (size_t i = 0; i < work.size(); ++i) {
      callableMap[work[i]]->walk([&](CallOpInterface call) {
        auto symbol = dyn_cast<SymbolRefAttr>(call.getCallableForCallee());
        if (visited.insert(symbol).second)
          work.push_back(symbol);
      });

      loadSymbols.insert_range(opLoadSymbols[work[i]]);

      storeSymbols.insert_range(opStoreSymbols[work[i]]);
    }

    loadSymbolsMap[thisSymbol] = std::move(loadSymbols);
    storeSymbolsMap[thisSymbol] = std::move(storeSymbols);
  }

  return success();
}

// Process each operation in the block deleting unneeded loads / stores,
// recursing on subblocks and checking function calls.
void MLProgramPipelineGlobals::processBlock(
    Block &block, toolchain::DenseSet<SymbolRefAttr> &symbolLoad,
    toolchain::DenseSet<SymbolRefAttr> &symbolStore) {

  toolchain::DenseMap<SymbolRefAttr, Value> previousLoads;
  toolchain::DenseMap<SymbolRefAttr, Operation *> previousStores;
  toolchain::SmallVector<Operation *> toDelete;
  for (auto &op : block) {
    // If this is a global load, remap to a previous value if known
    // and delete this load. Remember that this value is the currently
    // known load.
    if (auto load = mlir::dyn_cast<GlobalLoadOp>(op)) {
      auto ref = load.getGlobal();
      symbolLoad.insert(ref);
      if (previousLoads.contains(ref)) {
        toDelete.push_back(&op);
        load.getResult().replaceAllUsesWith(previousLoads[ref]);
      } else {
        previousLoads[ref] = load.getResult();
      }
      continue;
    }

    // Delete a previous store if it exists and is not needed, update
    // the most recent known value for this global ref.
    if (auto store = mlir::dyn_cast<GlobalStoreOp>(op)) {
      auto ref = store.getGlobal();
      symbolStore.insert(ref);
      auto it = previousStores.find(ref);
      if (it != previousStores.end()) {
        toDelete.push_back(it->getSecond());
      }

      previousLoads[ref] = store.getValue();
      previousStores[ref] = &op;
      continue;
    }

    // If a function is called, clear known values for loads/stores used by
    // the function or its sub-functions.
    if (auto call = mlir::dyn_cast<CallOpInterface>(op)) {
      auto loadSymbols =
          loadSymbolsMap[dyn_cast<SymbolRefAttr>(call.getCallableForCallee())];
      auto storeSymbols =
          storeSymbolsMap[dyn_cast<SymbolRefAttr>(call.getCallableForCallee())];

      for (auto sym : loadSymbols) {
        previousStores.erase(sym);
      }

      for (auto sym : storeSymbols) {
        previousLoads.erase(sym);
        previousStores.erase(sym);
      }
      continue;
    }

    // If the op has sub-regions, recurse inside. We make no guarantees whether
    // the recursion occurs.
    toolchain::DenseSet<SymbolRefAttr> opSymbolLoad;
    toolchain::DenseSet<SymbolRefAttr> opSymbolStore;
    for (auto &region : op.getRegions()) {
      for (auto &block : region) {
        processBlock(block, opSymbolLoad, opSymbolStore);
      }
    }

    // Update current state from the subblock.
    for (auto change : opSymbolLoad) {
      symbolLoad.insert(change);
      previousStores.erase(change);
    }

    for (auto change : opSymbolStore) {
      symbolStore.insert(change);
      previousLoads.erase(change);
      previousStores.erase(change);
    }
  }

  for (auto *op : toDelete) {
    op->erase();
  }
}

void MLProgramPipelineGlobals::runOnOperation() {
  auto targetOp = getOperation();
  if (failed(buildGlobalMap(targetOp))) {
    return;
  }

  for (auto &funcOp : *targetOp.getBody()) {
    for (auto &region : funcOp.getRegions()) {
      for (auto &block : region.getBlocks()) {
        toolchain::DenseSet<SymbolRefAttr> symbolsLoaded;
        toolchain::DenseSet<SymbolRefAttr> symbolsStored;
        processBlock(block, symbolsLoaded, symbolsStored);
      }
    }
  }
}

} // namespace

} // namespace ml_program
} // namespace mlir
