//===- ArrayConstructor.cpp - array constructor runtime API calls ---------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/ArrayConstructor.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/array-constructor-consts.h"

using namespace language::Compability::runtime;

namespace fir::runtime {
template <>
constexpr TypeBuilderFunc
getModel<language::Compability::runtime::ArrayConstructorVector &>() {
  return getModel<void *>();
}
} // namespace fir::runtime

mlir::Value fir::runtime::genInitArrayConstructorVector(
    mlir::Location loc, fir::FirOpBuilder &builder, mlir::Value toBox,
    mlir::Value useValueLengthParameters) {
  // Allocate storage for the runtime cookie for the array constructor vector.
  // Use pessimistic values for size and alignment that are valid for all
  // supported targets. Whether the actual ArrayConstructorVector object fits
  // into the available MaxArrayConstructorVectorSizeInBytes is verified when
  // building clang-rt.
  std::size_t arrayVectorStructBitSize =
      MaxArrayConstructorVectorSizeInBytes * 8;
  std::size_t alignLike = MaxArrayConstructorVectorAlignInBytes * 8;
  fir::SequenceType::Extent numElem =
      (arrayVectorStructBitSize + alignLike - 1) / alignLike;
  mlir::Type intType = builder.getIntegerType(alignLike);
  mlir::Type seqType = fir::SequenceType::get({numElem}, intType);
  mlir::Value cookie =
      builder.createTemporary(loc, seqType, ".rt.arrayctor.vector");

  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(InitArrayConstructorVector)>(
          loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  cookie = builder.createConvert(loc, funcType.getInput(0), cookie);
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, funcType.getInput(4));
  auto args = fir::runtime::createArguments(builder, loc, funcType, cookie,
                                            toBox, useValueLengthParameters,
                                            sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
  return cookie;
}

void fir::runtime::genPushArrayConstructorValue(
    mlir::Location loc, fir::FirOpBuilder &builder,
    mlir::Value arrayConstructorVector, mlir::Value fromBox) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PushArrayConstructorValue)>(loc,
                                                                       builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType,
                                            arrayConstructorVector, fromBox);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genPushArrayConstructorSimpleScalar(
    mlir::Location loc, fir::FirOpBuilder &builder,
    mlir::Value arrayConstructorVector, mlir::Value fromAddress) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PushArrayConstructorSimpleScalar)>(
          loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(
      builder, loc, funcType, arrayConstructorVector, fromAddress);
  fir::CallOp::create(builder, loc, func, args);
}
