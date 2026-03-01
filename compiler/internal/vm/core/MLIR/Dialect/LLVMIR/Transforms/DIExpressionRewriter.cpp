//===- DIExpressionRewriter.cpp - Rewriter for DIExpression operators -----===//
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

#include "mlir/Dialect/LLVMIR/Transforms/DIExpressionRewriter.h"
#include "vm/core/Support/DebugLog.h"

using namespace mlir;
using namespace LLVM;

#define DEBUG_TYPE "toolchain-di-expression-simplifier"

//===----------------------------------------------------------------------===//
// DIExpressionRewriter
//===----------------------------------------------------------------------===//

void DIExpressionRewriter::addPattern(
    std::unique_ptr<ExprRewritePattern> pattern) {
  patterns.emplace_back(std::move(pattern));
}

DIExpressionAttr
DIExpressionRewriter::simplify(DIExpressionAttr expr,
                               std::optional<uint64_t> maxNumRewrites) const {
  ArrayRef<OperatorT> operators = expr.getOperations();

  // `inputs` contains the unprocessed postfix of operators.
  // `result` contains the already finalized prefix of operators.
  // Invariant: concat(result, inputs) is equivalent to `operators` after some
  // application of the rewrite patterns.
  // Using a deque for inputs so that we have efficient front insertion and
  // removal. Random access is not necessary for patterns.
  std::deque<OperatorT> inputs(operators.begin(), operators.end());
  SmallVector<OperatorT> result;

  uint64_t numRewrites = 0;
  while (!inputs.empty() &&
         (!maxNumRewrites || numRewrites < *maxNumRewrites)) {
    bool foundMatch = false;
    for (const std::unique_ptr<ExprRewritePattern> &pattern : patterns) {
      ExprRewritePattern::OpIterT matchEnd = pattern->match(inputs);
      if (matchEnd == inputs.begin())
        continue;

      foundMatch = true;
      SmallVector<OperatorT> replacement =
          pattern->replace(toolchain::make_range(inputs.cbegin(), matchEnd));
      inputs.erase(inputs.begin(), matchEnd);
      inputs.insert(inputs.begin(), replacement.begin(), replacement.end());
      ++numRewrites;
      break;
    }

    if (!foundMatch) {
      // If no match, pass along the current operator.
      result.push_back(inputs.front());
      inputs.pop_front();
    }
  }

  if (maxNumRewrites && numRewrites >= *maxNumRewrites) {
    LDBG() << "LLVMDIExpressionSimplifier exceeded max num rewrites ("
           << maxNumRewrites << ")";
    // Skip rewriting the rest.
    result.append(inputs.begin(), inputs.end());
  }

  return LLVM::DIExpressionAttr::get(expr.getContext(), result);
}
