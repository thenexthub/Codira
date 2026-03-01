//===- IntegerSetDetail.h - MLIR IntegerSet storage details -----*- C++ -*-===//
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
// This holds implementation details of IntegerSet.
//
//===----------------------------------------------------------------------===//

#ifndef INTEGERSETDETAIL_H_
#define INTEGERSETDETAIL_H_

#include "mlir/IR/AffineExpr.h"
#include "mlir/Support/StorageUniquer.h"
#include "vm/core/ADT/ArrayRef.h"

namespace mlir {
namespace detail {

struct IntegerSetStorage : public StorageUniquer::BaseStorage {
  /// The hash key used for uniquing.
  using KeyTy =
      std::tuple<unsigned, unsigned, ArrayRef<AffineExpr>, ArrayRef<bool>>;

  unsigned dimCount;
  unsigned symbolCount;

  /// Array of affine constraints: a constraint is either an equality
  /// (affine_expr == 0) or an inequality (affine_expr >= 0).
  ArrayRef<AffineExpr> constraints;

  // Bits to check whether a constraint is an equality or an inequality.
  ArrayRef<bool> eqFlags;

  bool operator==(const KeyTy &key) const {
    return std::get<0>(key) == dimCount && std::get<1>(key) == symbolCount &&
           std::get<2>(key) == constraints && std::get<3>(key) == eqFlags;
  }

  static IntegerSetStorage *
  construct(StorageUniquer::StorageAllocator &allocator, const KeyTy &key) {
    auto *res =
        new (allocator.allocate<IntegerSetStorage>()) IntegerSetStorage();
    res->dimCount = std::get<0>(key);
    res->symbolCount = std::get<1>(key);
    res->constraints = allocator.copyInto(std::get<2>(key));
    res->eqFlags = allocator.copyInto(std::get<3>(key));
    return res;
  }
};

} // namespace detail
} // namespace mlir
#endif // INTEGERSETDETAIL_H_
