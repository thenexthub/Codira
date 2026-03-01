//===--- MatchersInternal.cpp----------------------------------------------===//
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

#include "mlir/Query/Matcher/MatchersInternal.h"

namespace mlir::query::matcher {

namespace internal {

bool allOfVariadicOperator(Operation *op, SetVector<Operation *> *matchedOps,
                           ArrayRef<DynMatcher> innerMatchers) {
  return toolchain::all_of(innerMatchers, [&](const DynMatcher &matcher) {
    if (matchedOps)
      return matcher.match(op, *matchedOps);
    return matcher.match(op);
  });
}
bool anyOfVariadicOperator(Operation *op, SetVector<Operation *> *matchedOps,
                           ArrayRef<DynMatcher> innerMatchers) {
  return toolchain::any_of(innerMatchers, [&](const DynMatcher &matcher) {
    if (matchedOps)
      return matcher.match(op, *matchedOps);
    return matcher.match(op);
  });
}
} // namespace internal
} // namespace mlir::query::matcher
