//===- AffineMapDetail.h - MLIR Affine Map details Class --------*- C++ -*-===//
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
// This holds implementation details of AffineMap.
//
//===----------------------------------------------------------------------===//

#ifndef AFFINEMAPDETAIL_H_
#define AFFINEMAPDETAIL_H_

#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/Support/StorageUniquer.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/Support/TrailingObjects.h"

namespace mlir {
namespace detail {

struct AffineMapStorage final
    : public StorageUniquer::BaseStorage,
      private toolchain::TrailingObjects<AffineMapStorage, AffineExpr> {
  friend toolchain::TrailingObjects<AffineMapStorage, AffineExpr>;

  /// The hash key used for uniquing.
  using KeyTy = std::tuple<unsigned, unsigned, ArrayRef<AffineExpr>>;

  unsigned numDims;
  unsigned numSymbols;
  unsigned numResults;

  MLIRContext *context;

  /// The affine expressions for this (multi-dimensional) map.
  ArrayRef<AffineExpr> results() const {
    return getTrailingObjects(numResults);
  }

  bool operator==(const KeyTy &key) const {
    return std::get<0>(key) == numDims && std::get<1>(key) == numSymbols &&
           std::get<2>(key) == results();
  }

  // Constructs an AffineMapStorage from a key. The context must be set by the
  // caller.
  static AffineMapStorage *
  construct(StorageUniquer::StorageAllocator &allocator, const KeyTy &key) {
    auto results = std::get<2>(key);
    auto byteSize =
        AffineMapStorage::totalSizeToAlloc<AffineExpr>(results.size());
    auto *rawMem = allocator.allocate(byteSize, alignof(AffineMapStorage));
    auto *res = new (rawMem) AffineMapStorage();
    res->numDims = std::get<0>(key);
    res->numSymbols = std::get<1>(key);
    res->numResults = results.size();
    toolchain::uninitialized_copy(results, res->getTrailingObjects());
    return res;
  }
};

} // namespace detail
} // namespace mlir

#endif // AFFINEMAPDETAIL_H_
