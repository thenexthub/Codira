//===-- Support.cpp - generate support runtime API calls --------*- C++ -*-===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Support.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/support.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

using namespace language::Compability::runtime;

template <>
constexpr fir::runtime::TypeBuilderFunc
fir::runtime::getModel<language::Compability::runtime::LowerBoundModifier>() {
  return [](mlir::MLIRContext *context) -> mlir::Type {
    return mlir::IntegerType::get(
        context, sizeof(language::Compability::runtime::LowerBoundModifier) * 8);
  };
}

void fir::runtime::genCopyAndUpdateDescriptor(fir::FirOpBuilder &builder,
                                              mlir::Location loc,
                                              mlir::Value to, mlir::Value from,
                                              mlir::Value newDynamicType,
                                              mlir::Value newAttribute,
                                              mlir::Value newLowerBounds) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(CopyAndUpdateDescriptor)>(loc,
                                                                     builder);
  auto fTy = func.getFunctionType();
  auto args =
      fir::runtime::createArguments(builder, loc, fTy, to, from, newDynamicType,
                                    newAttribute, newLowerBounds);
  toolchain::StringRef noCapture = mlir::LLVM::LLVMDialect::getNoCaptureAttrName();
  if (!func.getArgAttr(0, noCapture)) {
    mlir::UnitAttr unitAttr = mlir::UnitAttr::get(func.getContext());
    func.setArgAttr(0, noCapture, unitAttr);
    func.setArgAttr(1, noCapture, unitAttr);
  }
  fir::CallOp::create(builder, loc, func, args);
}

mlir::Value fir::runtime::genIsAssumedSize(fir::FirOpBuilder &builder,
                                           mlir::Location loc,
                                           mlir::Value box) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(IsAssumedSize)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, box);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}
