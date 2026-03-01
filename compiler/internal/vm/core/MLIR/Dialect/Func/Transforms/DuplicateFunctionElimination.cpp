//===- DuplicateFunctionElimination.cpp - Duplicate function elimination --===//
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

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/Passes.h"

namespace mlir {
namespace func {
#define GEN_PASS_DEF_DUPLICATEFUNCTIONELIMINATIONPASS
#include "mlir/Dialect/Func/Transforms/Passes.h.inc"
} // namespace func

namespace {

// Define a notion of function equivalence that allows for reuse. Ignore the
// symbol name for this purpose.
struct DuplicateFuncOpEquivalenceInfo
    : public toolchain::DenseMapInfo<func::FuncOp> {

  static unsigned getHashValue(const func::FuncOp cFunc) {
    if (!cFunc) {
      return DenseMapInfo<func::FuncOp>::getHashValue(cFunc);
    }

    // Aggregate attributes, ignoring the symbol name.
    toolchain::hash_code hash = {};
    func::FuncOp func = const_cast<func::FuncOp &>(cFunc);
    StringAttr symNameAttrName = func.getSymNameAttrName();
    for (NamedAttribute namedAttr : cFunc->getAttrs()) {
      StringAttr attrName = namedAttr.getName();
      if (attrName == symNameAttrName)
        continue;
      hash = toolchain::hash_combine(hash, namedAttr);
    }

    // Also hash the func body.
    func.getBody().walk([&](Operation *op) {
      hash = toolchain::hash_combine(
          hash, OperationEquivalence::computeHash(
                    op, /*hashOperands=*/OperationEquivalence::ignoreHashValue,
                    /*hashResults=*/OperationEquivalence::ignoreHashValue,
                    OperationEquivalence::IgnoreLocations));
    });

    return hash;
  }

  static bool isEqual(func::FuncOp lhs, func::FuncOp rhs) {
    if (lhs == rhs)
      return true;
    if (lhs == getTombstoneKey() || lhs == getEmptyKey() ||
        rhs == getTombstoneKey() || rhs == getEmptyKey())
      return false;

    if (lhs.isDeclaration() || rhs.isDeclaration())
      return false;

    // Check discardable attributes equivalence
    if (lhs->getDiscardableAttrDictionary() !=
        rhs->getDiscardableAttrDictionary())
      return false;

    // Check properties equivalence, ignoring the symbol name.
    // Make a copy, so that we can erase the symbol name and perform the
    // comparison.
    auto pLhs = lhs.getProperties();
    auto pRhs = rhs.getProperties();
    pLhs.sym_name = nullptr;
    pRhs.sym_name = nullptr;
    if (pLhs != pRhs)
      return false;

    // Compare inner workings.
    return OperationEquivalence::isRegionEquivalentTo(
        &lhs.getBody(), &rhs.getBody(), OperationEquivalence::IgnoreLocations);
  }
};

struct DuplicateFunctionEliminationPass
    : public func::impl::DuplicateFunctionEliminationPassBase<
          DuplicateFunctionEliminationPass> {

  using DuplicateFunctionEliminationPassBase<
      DuplicateFunctionEliminationPass>::DuplicateFunctionEliminationPassBase;

  void runOnOperation() override {
    auto module = getOperation();

    // Find unique representant per equivalent func ops.
    DenseSet<func::FuncOp, DuplicateFuncOpEquivalenceInfo> uniqueFuncOps;
    DenseMap<StringAttr, func::FuncOp> getRepresentant;
    DenseSet<func::FuncOp> toBeErased;
    module.walk([&](func::FuncOp f) {
      auto [repr, inserted] = uniqueFuncOps.insert(f);
      getRepresentant[f.getSymNameAttr()] = *repr;
      if (!inserted) {
        toBeErased.insert(f);
      }
    });

    // Update all symbol uses to reference unique func op
    // representants and erase redundant func ops.
    SymbolTableCollection symbolTable;
    SymbolUserMap userMap(symbolTable, module);
    for (auto it : toBeErased) {
      StringAttr oldSymbol = it.getSymNameAttr();
      StringAttr newSymbol = getRepresentant[oldSymbol].getSymNameAttr();
      userMap.replaceAllUsesWith(it, newSymbol);
      it.erase();
    }
  }
};

} // namespace
} // namespace mlir
