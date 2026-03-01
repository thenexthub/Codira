//===- Utils.cpp - GPU transforms utils -----------------------------------===//
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
//
// Implements GPU dialect transforms utils.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/GPU/Utils/GPUUtils.h"
#include "vm/core/Support/ErrorHandling.h"

namespace mlir::gpu {

vector::CombiningKind convertReductionKind(gpu::AllReduceOperation mode) {
  switch (mode) {
#define MAP_CASE(X)                                                            \
  case gpu::AllReduceOperation::X:                                             \
    return vector::CombiningKind::X

    MAP_CASE(ADD);
    MAP_CASE(MUL);
    MAP_CASE(MINUI);
    MAP_CASE(MINSI);
    MAP_CASE(MINNUMF);
    MAP_CASE(MAXSI);
    MAP_CASE(MAXUI);
    MAP_CASE(MAXNUMF);
    MAP_CASE(AND);
    MAP_CASE(OR);
    MAP_CASE(XOR);
    MAP_CASE(MINIMUMF);
    MAP_CASE(MAXIMUMF);

#undef MAP_CASE
  }

  llvm_unreachable("Vector and GPU reduction kinds should match 1:1");
}

} // namespace mlir::gpu
