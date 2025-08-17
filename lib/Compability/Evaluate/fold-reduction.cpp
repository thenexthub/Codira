//===-- lib/Evaluate/fold-reduction.cpp -----------------------------------===//
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

#include "fold-reduction.h"

namespace language::Compability::evaluate {
bool CheckReductionDIM(std::optional<int> &dim, FoldingContext &context,
    ActualArguments &arg, std::optional<int> dimIndex, int rank) {
  if (!dimIndex || static_cast<std::size_t>(*dimIndex) >= arg.size() ||
      !arg[*dimIndex]) {
    dim.reset();
    return true; // no DIM= argument
  }
  if (auto *dimConst{
          Folder<SubscriptInteger>{context}.Folding(arg[*dimIndex])}) {
    if (auto dimScalar{dimConst->GetScalarValue()}) {
      auto dimVal{dimScalar->ToInt64()};
      if (dimVal >= 1 && dimVal <= rank) {
        dim = dimVal;
        return true; // DIM= exists and is a valid constant
      } else {
        context.messages().Say(
            "DIM=%jd is not valid for an array of rank %d"_err_en_US,
            static_cast<std::intmax_t>(dimVal), rank);
      }
    }
  }
  return false; // DIM= bad or not scalar constant
}

Constant<LogicalResult> *GetReductionMASK(
    std::optional<ActualArgument> &maskArg, const ConstantSubscripts &shape,
    FoldingContext &context) {
  Constant<LogicalResult> *mask{
      Folder<LogicalResult>{context}.Folding(maskArg)};
  if (mask &&
      !CheckConformance(context.messages(), AsShape(shape),
          AsShape(mask->shape()), CheckConformanceFlags::RightScalarExpandable,
          "ARRAY=", "MASK=")
           .value_or(false)) {
    mask = nullptr;
  }
  return mask;
}
} // namespace language::Compability::evaluate
