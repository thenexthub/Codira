//=- VectorEmulateMaskedLoadStore.cpp - Emulate 'vector.maskedload/store' op =//
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
// This file implements target-independent rewrites and utilities to emulate the
// 'vector.maskedload' and 'vector.maskedstore' operation.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Vector/Transforms/LoweringPatterns.h"

using namespace mlir;

namespace {

/// Convert vector.maskedload
///
/// Before:
///
///   vector.maskedload %base[%idx_0, %idx_1], %mask, %pass_thru
///
/// After:
///
///   %ivalue = %pass_thru
///   %m = vector.extract %mask[0]
///   %result0 = scf.if %m {
///     %v = memref.load %base[%idx_0, %idx_1]
///     %combined = vector.insert %v, %ivalue[0]
///     scf.yield %combined
///   } else {
///     scf.yield %ivalue
///   }
///   %m = vector.extract %mask[1]
///   %result1 = scf.if %m {
///     %v = memref.load %base[%idx_0, %idx_1 + 1]
///     %combined = vector.insert %v, %result0[1]
///     scf.yield %combined
///   } else {
///     scf.yield %result0
///   }
///   ...
///
struct VectorMaskedLoadOpConverter final
    : OpRewritePattern<vector::MaskedLoadOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(vector::MaskedLoadOp maskedLoadOp,
                                PatternRewriter &rewriter) const override {
    VectorType maskVType = maskedLoadOp.getMaskVectorType();
    if (maskVType.getShape().size() != 1)
      return rewriter.notifyMatchFailure(
          maskedLoadOp, "expected vector.maskedstore with 1-D mask");

    Location loc = maskedLoadOp.getLoc();
    int64_t maskLength = maskVType.getShape()[0];

    Type indexType = rewriter.getIndexType();
    Value mask = maskedLoadOp.getMask();
    Value base = maskedLoadOp.getBase();
    Value iValue = maskedLoadOp.getPassThru();
    auto indices = toolchain::to_vector_of<Value>(maskedLoadOp.getIndices());
    Value one = arith::ConstantOp::create(rewriter, loc, indexType,
                                          IntegerAttr::get(indexType, 1));
    for (int64_t i = 0; i < maskLength; ++i) {
      auto maskBit = vector::ExtractOp::create(rewriter, loc, mask, i);

      auto ifOp = scf::IfOp::create(
          rewriter, loc, maskBit,
          [&](OpBuilder &builder, Location loc) {
            auto loadedValue = memref::LoadOp::create(
                builder, loc, base, indices, /*nontemporal=*/false,
                toolchain::MaybeAlign(maskedLoadOp.getAlignment().value_or(0)));
            auto combinedValue =
                vector::InsertOp::create(builder, loc, loadedValue, iValue, i);
            scf::YieldOp::create(builder, loc, combinedValue.getResult());
          },
          [&](OpBuilder &builder, Location loc) {
            scf::YieldOp::create(builder, loc, iValue);
          });
      iValue = ifOp.getResult(0);
      if (!indices.empty())
        indices.back() =
            arith::AddIOp::create(rewriter, loc, indices.back(), one);
    }

    rewriter.replaceOp(maskedLoadOp, iValue);

    return success();
  }
};

/// Convert vector.maskedstore
///
/// Before:
///
///   vector.maskedstore %base[%idx_0, %idx_1], %mask, %value
///
/// After:
///
///   %m = vector.extract %mask[0]
///   scf.if %m {
///     %extracted = vector.extract %value[0]
///     memref.store %extracted, %base[%idx_0, %idx_1]
///   }
///   %m = vector.extract %mask[1]
///   scf.if %m {
///     %extracted = vector.extract %value[1]
///     memref.store %extracted, %base[%idx_0, %idx_1 + 1]
///   }
///   ...
///
struct VectorMaskedStoreOpConverter final
    : OpRewritePattern<vector::MaskedStoreOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(vector::MaskedStoreOp maskedStoreOp,
                                PatternRewriter &rewriter) const override {
    VectorType maskVType = maskedStoreOp.getMaskVectorType();
    if (maskVType.getShape().size() != 1)
      return rewriter.notifyMatchFailure(
          maskedStoreOp, "expected vector.maskedstore with 1-D mask");

    Location loc = maskedStoreOp.getLoc();
    int64_t maskLength = maskVType.getShape()[0];

    Type indexType = rewriter.getIndexType();
    Value mask = maskedStoreOp.getMask();
    Value base = maskedStoreOp.getBase();
    Value value = maskedStoreOp.getValueToStore();
    bool nontemporal = false;
    auto indices = toolchain::to_vector_of<Value>(maskedStoreOp.getIndices());
    Value one = arith::ConstantOp::create(rewriter, loc, indexType,
                                          IntegerAttr::get(indexType, 1));
    for (int64_t i = 0; i < maskLength; ++i) {
      auto maskBit = vector::ExtractOp::create(rewriter, loc, mask, i);

      auto ifOp = scf::IfOp::create(rewriter, loc, maskBit, /*else=*/false);
      rewriter.setInsertionPointToStart(&ifOp.getThenRegion().front());
      auto extractedValue = vector::ExtractOp::create(rewriter, loc, value, i);
      memref::StoreOp::create(
          rewriter, loc, extractedValue, base, indices, nontemporal,
          toolchain::MaybeAlign(maskedStoreOp.getAlignment().value_or(0)));

      rewriter.setInsertionPointAfter(ifOp);
      if (!indices.empty())
        indices.back() =
            arith::AddIOp::create(rewriter, loc, indices.back(), one);
    }

    rewriter.eraseOp(maskedStoreOp);

    return success();
  }
};

} // namespace

void mlir::vector::populateVectorMaskedLoadStoreEmulationPatterns(
    RewritePatternSet &patterns, PatternBenefit benefit) {
  patterns.add<VectorMaskedLoadOpConverter, VectorMaskedStoreOpConverter>(
      patterns.getContext(), benefit);
}
