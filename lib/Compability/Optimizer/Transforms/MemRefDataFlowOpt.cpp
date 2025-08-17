//===- MemRefDataFlowOpt.cpp - Memory DataFlow Optimization pass ----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/Operation.h"
#include "mlir/Transforms/Passes.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallVector.h"
#include <optional>

namespace fir {
#define GEN_PASS_DEF_MEMREFDATAFLOWOPT
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "fir-memref-dataflow-opt"

using namespace mlir;

namespace {

template <typename OpT>
static std::vector<OpT> getSpecificUsers(mlir::Value v) {
  std::vector<OpT> ops;
  for (mlir::Operation *user : v.getUsers())
    if (auto op = dyn_cast<OpT>(user))
      ops.push_back(op);
  return ops;
}

/// This is based on MLIR's MemRefDataFlowOpt which is specialized on AffineRead
/// and AffineWrite interface
template <typename ReadOp, typename WriteOp>
class LoadStoreForwarding {
public:
  LoadStoreForwarding(mlir::DominanceInfo *di) : domInfo(di) {}

  // FIXME: This algorithm has a bug. It ignores escaping references between a
  // store and a load.
  std::optional<WriteOp> findStoreToForward(ReadOp loadOp,
                                            std::vector<WriteOp> &&storeOps) {
    toolchain::SmallVector<WriteOp> candidateSet;

    for (auto storeOp : storeOps)
      if (domInfo->dominates(storeOp, loadOp))
        candidateSet.push_back(storeOp);

    if (candidateSet.empty())
      return {};

    std::optional<WriteOp> nearestStore;
    for (auto candidate : candidateSet) {
      auto nearerThan = [&](WriteOp otherStore) {
        if (candidate == otherStore)
          return false;
        bool rv = domInfo->properlyDominates(candidate, otherStore);
        if (rv) {
          LLVM_DEBUG(toolchain::dbgs()
                     << "candidate " << candidate << " is not the nearest to "
                     << loadOp << " because " << otherStore << " is closer\n");
        }
        return rv;
      };
      if (!toolchain::any_of(candidateSet, nearerThan)) {
        nearestStore = mlir::cast<WriteOp>(candidate);
        break;
      }
    }
    if (!nearestStore) {
      LLVM_DEBUG(
          toolchain::dbgs()
          << "load " << loadOp << " has " << candidateSet.size()
          << " store candidates, but this algorithm can't find a best.\n");
    }
    return nearestStore;
  }

  std::optional<ReadOp> findReadForWrite(WriteOp storeOp,
                                         std::vector<ReadOp> &&loadOps) {
    for (auto &loadOp : loadOps) {
      if (domInfo->dominates(storeOp, loadOp))
        return loadOp;
    }
    return {};
  }

private:
  mlir::DominanceInfo *domInfo;
};

class MemDataFlowOpt : public fir::impl::MemRefDataFlowOptBase<MemDataFlowOpt> {
public:
  void runOnOperation() override {
    mlir::func::FuncOp f = getOperation();

    auto *domInfo = &getAnalysis<mlir::DominanceInfo>();
    LoadStoreForwarding<fir::LoadOp, fir::StoreOp> lsf(domInfo);
    f.walk([&](fir::LoadOp loadOp) {
      auto maybeStore = lsf.findStoreToForward(
          loadOp, getSpecificUsers<fir::StoreOp>(loadOp.getMemref()));
      if (maybeStore) {
        auto storeOp = *maybeStore;
        LLVM_DEBUG(toolchain::dbgs() << "FlangMemDataFlowOpt: In " << f.getName()
                                << " erasing load " << loadOp
                                << " with value from " << storeOp << '\n');
        loadOp.getResult().replaceAllUsesWith(storeOp.getValue());
        loadOp.erase();
      }
    });
    f.walk([&](fir::AllocaOp alloca) {
      for (auto &storeOp : getSpecificUsers<fir::StoreOp>(alloca.getResult())) {
        if (!lsf.findReadForWrite(
                storeOp, getSpecificUsers<fir::LoadOp>(storeOp.getMemref()))) {
          LLVM_DEBUG(toolchain::dbgs() << "FlangMemDataFlowOpt: In " << f.getName()
                                  << " erasing store " << storeOp << '\n');
          storeOp.erase();
        }
      }
    });
  }
};
} // namespace

std::unique_ptr<mlir::Pass> fir::createMemDataFlowOptPass() {
  return std::make_unique<MemDataFlowOpt>();
}
