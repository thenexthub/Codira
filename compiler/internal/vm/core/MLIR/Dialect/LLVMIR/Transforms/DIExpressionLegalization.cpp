//===- DIExpressionLegalization.cpp - DIExpression Legalization Patterns --===//
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

#include "mlir/Dialect/LLVMIR/Transforms/DIExpressionLegalization.h"

#include "vm/core/BinaryFormat/Dwarf.h"

using namespace mlir;
using namespace LLVM;

//===----------------------------------------------------------------------===//
// MergeFragments
//===----------------------------------------------------------------------===//

MergeFragments::OpIterT MergeFragments::match(OpIterRange operators) const {
  OpIterT it = operators.begin();
  if (it == operators.end() ||
      it->getOpcode() != toolchain::dwarf::DW_OP_LLVM_fragment)
    return operators.begin();

  ++it;
  if (it == operators.end() ||
      it->getOpcode() != toolchain::dwarf::DW_OP_LLVM_fragment)
    return operators.begin();

  return ++it;
}

SmallVector<MergeFragments::OperatorT>
MergeFragments::replace(OpIterRange operators) const {
  OpIterT it = operators.begin();
  OperatorT first = *(it++);
  OperatorT second = *it;
  // Add offsets & select the size of the earlier operator (the one closer to
  // the IR value).
  uint64_t offset = first.getArguments()[0] + second.getArguments()[0];
  uint64_t size = first.getArguments()[1];
  OperatorT newOp = OperatorT::get(
      first.getContext(), toolchain::dwarf::DW_OP_LLVM_fragment, {offset, size});
  return SmallVector<OperatorT>{newOp};
}

//===----------------------------------------------------------------------===//
// Runner
//===----------------------------------------------------------------------===//

void mlir::LLVM::legalizeDIExpressionsRecursively(Operation *op) {
  LLVM::DIExpressionRewriter rewriter;
  rewriter.addPattern(std::make_unique<MergeFragments>());

  AttrTypeReplacer replacer;
  replacer.addReplacement([&rewriter](LLVM::DIExpressionAttr expr) {
    return rewriter.simplify(expr);
  });
  replacer.recursivelyReplaceElementsIn(op);
}
