//===- SinkVectorProducerOps.cpp ------------------------------------------===//
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

#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/Vector/Utils/VectorUtils.h"
#include "mlir/Dialect/X86Vector/Transforms.h"
#include "mlir/Dialect/X86Vector/X86VectorDialect.h"

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/PatternMatch.h"

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::vector;
using namespace mlir::x86vector;

static FailureOr<toolchain::SmallVector<Operation *>>
getSameBlockUsers(Operation *op) {
  toolchain::SmallVector<Operation *> opUsers;
  for (OpResult result : op->getResults()) {
    for (Operation *user : result.getUsers()) {
      // Check prod and users belongs to same block.
      if (op->getBlock() != user->getBlock())
        return failure();
      opUsers.push_back(user);
    }
  }

  return opUsers;
}

// Prevent pathological looping:
// If two/three producers are used by same consumer, will end in looping of
// moving the producers.
// For example:
// %1 = prod1
// %2 = prod2
// %3 = prod3
// %4 = op %1, %2, %3
static bool checkLooping(Operation *op) {
  toolchain::SmallVector<Operation *> operations;
  operations.push_back(op);

  // Retrive the next immediate operation until it is a vector.load or
  // a vector.transfer_read
  Operation *nextOp = op->getNextNode();
  while (nextOp) {
    if (isa<vector::LoadOp>(nextOp) || isa<vector::TransferReadOp>(nextOp)) {
      operations.push_back(op);
    } else {
      break;
    }
    nextOp = nextOp->getNextNode();
  }

  // If all the loads or transfer_reads have same immediate nextOp as its
  // user, then it loops.
  for (Operation *op : operations) {
    FailureOr<toolchain::SmallVector<Operation *>> users = getSameBlockUsers(op);
    if (failed(users))
      return false;

    if (!toolchain::is_contained(*users, nextOp))
      return false;
  }

  return true;
}

/// Sink vector producers forward to reduce live ranges.
/// This pattern applies to ops such as vector.load and vector.transfer_read.
template <typename producerOp>
struct SinkVectorProducerOps final : public OpRewritePattern<producerOp> {
  using OpRewritePattern<producerOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(producerOp op,
                                PatternRewriter &rewriter) const override {

    auto users = getSameBlockUsers(op);
    if (failed(users))
      return failure();

    if (checkLooping(op))
      return failure();

    toolchain::DenseMap<Operation *, toolchain::SmallVector<Operation *>> prodsAllUsers;
    toolchain::DenseMap<Operation *, Operation *> prodsFirstUser;

    toolchain::SmallVector<Operation *> opUsers = *users;
    prodsAllUsers.try_emplace(op, opUsers);

    // Iterate until the last instruction to find the first users of all
    // producers within the block.
    Operation *nextOp = op;

    while ((nextOp = nextOp->getNextNode())) {

      if (isa<vector::LoadOp>(nextOp) || isa<vector::TransferReadOp>(nextOp)) {
        auto nextUsers = getSameBlockUsers(nextOp);

        if (failed(nextUsers))
          continue;
        toolchain::SmallVector<Operation *> nextOpUsers = *nextUsers;
        prodsAllUsers.try_emplace(nextOp, nextOpUsers);
      } else {
        toolchain::SmallVector<Operation *> operations;

        for (auto &entry : prodsAllUsers) {
          toolchain::SmallVector<Operation *> &users = entry.second;

          if (toolchain::is_contained(users, nextOp)) {
            Operation *operation = entry.first;
            operations.push_back(operation);
            prodsFirstUser.try_emplace(operation, nextOp);
          }
        }

        for (Operation *op : operations) {
          prodsAllUsers.erase(op);
        }
      }
    }

    // Move all the loads or transfer_reads before its first use.
    for (auto &entry : prodsFirstUser) {
      Operation *prod = entry.first;
      Operation *consumer = entry.second;

      prod->moveBefore(consumer);
    }

    return success();
  }
};

void x86vector::populateSinkVectorProducerOpsPatterns(
    RewritePatternSet &patterns) {
  patterns.add<SinkVectorProducerOps<vector::TransferReadOp>,
               SinkVectorProducerOps<vector::LoadOp>>(patterns.getContext());
}
