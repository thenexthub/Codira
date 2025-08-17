//===-- Ragged.cpp --------------------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Ragged.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/ragged.h"

using namespace language::Compability::runtime;

void fir::runtime::genRaggedArrayAllocate(mlir::Location loc,
                                          fir::FirOpBuilder &builder,
                                          mlir::Value header, bool asHeaders,
                                          mlir::Value eleSize,
                                          mlir::ValueRange extents) {
  auto i32Ty = builder.getIntegerType(32);
  auto rank = extents.size();
  auto i64Ty = builder.getIntegerType(64);
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(RaggedArrayAllocate)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto i1Ty = builder.getIntegerType(1);
  fir::SequenceType::Shape shape = {
      static_cast<fir::SequenceType::Extent>(rank)};
  auto extentTy = fir::SequenceType::get(shape, i64Ty);
  auto refTy = fir::ReferenceType::get(i64Ty);
  // Position of the bufferPointer in the header struct.
  auto one = builder.createIntegerConstant(loc, i32Ty, 1);
  auto eleTy = fir::unwrapSequenceType(fir::unwrapRefType(header.getType()));
  auto ptrTy =
      builder.getRefType(mlir::cast<mlir::TupleType>(eleTy).getType(1));
  auto ptr = fir::CoordinateOp::create(builder, loc, ptrTy, header, one);
  auto heap = fir::LoadOp::create(builder, loc, ptr);
  auto cmp = builder.genIsNullAddr(loc, heap);
  builder.genIfThen(loc, cmp)
      .genThen([&]() {
        auto asHeadersVal = builder.createIntegerConstant(loc, i1Ty, asHeaders);
        auto rankVal = builder.createIntegerConstant(loc, i64Ty, rank);
        auto buff = fir::AllocMemOp::create(builder, loc, extentTy);
        // Convert all the extents to i64 and pack them in a buffer on the heap.
        for (auto i : toolchain::enumerate(extents)) {
          auto offset = builder.createIntegerConstant(loc, i32Ty, i.index());
          auto addr =
              fir::CoordinateOp::create(builder, loc, refTy, buff, offset);
          auto castVal = builder.createConvert(loc, i64Ty, i.value());
          fir::StoreOp::create(builder, loc, castVal, addr);
        }
        auto args = fir::runtime::createArguments(
            builder, loc, fTy, header, asHeadersVal, rankVal, eleSize, buff);
        fir::CallOp::create(builder, loc, func, args);
      })
      .end();
}

void fir::runtime::genRaggedArrayDeallocate(mlir::Location loc,
                                            fir::FirOpBuilder &builder,
                                            mlir::Value header) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(RaggedArrayDeallocate)>(
      loc, builder);
  auto fTy = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, header);
  fir::CallOp::create(builder, loc, func, args);
}
