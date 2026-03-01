//===- MapRef.cpp - A dim2lvl/lvl2dim map reference wrapper ---------------===//
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

#include "mlir/ExecutionEngine/SparseTensor/MapRef.h"
#include "mlir/Dialect/SparseTensor/IR/Enums.h"

mlir::sparse_tensor::MapRef::MapRef(uint64_t d, uint64_t l, const uint64_t *d2l,
                                    const uint64_t *l2d)
    : dimRank(d), lvlRank(l), dim2lvl(d2l), lvl2dim(l2d),
      isPermutation(isPermutationMap()) {
  if (isPermutation) {
    for (uint64_t l = 0; l < lvlRank; l++)
      assert(lvl2dim[dim2lvl[l]] == l);
  }
}

bool mlir::sparse_tensor::MapRef::isPermutationMap() const {
  if (dimRank != lvlRank)
    return false;
  std::vector<bool> seen(dimRank, false);
  for (uint64_t l = 0; l < lvlRank; l++) {
    const uint64_t d = dim2lvl[l];
    if (d >= dimRank || seen[d])
      return false;
    seen[d] = true;
  }
  return true;
}

bool mlir::sparse_tensor::MapRef::isFloor(uint64_t l, uint64_t &i,
                                          uint64_t &c) const {
  if (isEncodedFloor(dim2lvl[l])) {
    i = decodeIndex(dim2lvl[l]);
    c = decodeConst(dim2lvl[l]);
    return true;
  }
  return false;
}

bool mlir::sparse_tensor::MapRef::isMod(uint64_t l, uint64_t &i,
                                        uint64_t &c) const {
  if (isEncodedMod(dim2lvl[l])) {
    i = decodeIndex(dim2lvl[l]);
    c = decodeConst(dim2lvl[l]);
    return true;
  }
  return false;
}

bool mlir::sparse_tensor::MapRef::isMul(uint64_t d, uint64_t &i, uint64_t &c,
                                        uint64_t &ii) const {
  if (isEncodedMul(lvl2dim[d])) {
    i = decodeIndex(lvl2dim[d]);
    c = decodeMulc(lvl2dim[d]);
    ii = decodeMuli(lvl2dim[d]);
    return true;
  }
  return false;
}
