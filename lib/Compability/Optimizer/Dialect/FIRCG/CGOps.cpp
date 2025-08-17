//===-- CGOps.cpp -- FIR codegen operations -------------------------------===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRCG/CGOps.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"

/// FIR codegen dialect constructor.
fir::FIRCodeGenDialect::FIRCodeGenDialect(mlir::MLIRContext *ctx)
    : mlir::Dialect("fircg", ctx, mlir::TypeID::get<FIRCodeGenDialect>()) {
  addOperations<
#define GET_OP_LIST
#include "language/Compability/Optimizer/Dialect/FIRCG/CGOps.cpp.inc"
      >();
}

// anchor the class vtable to this compilation unit
fir::FIRCodeGenDialect::~FIRCodeGenDialect() {
  // do nothing
}

#define GET_OP_CLASSES
#include "language/Compability/Optimizer/Dialect/FIRCG/CGOps.cpp.inc"

unsigned fir::cg::XEmboxOp::getOutRank() {
  if (getSlice().empty())
    return getRank();
  auto outRank = fir::SliceOp::getOutputRank(getSlice());
  assert(outRank >= 1);
  return outRank;
}

unsigned fir::cg::XReboxOp::getOutRank() {
  if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(
          fir::dyn_cast_ptrOrBoxEleTy(getType())))
    return seqTy.getDimension();
  return 0;
}

unsigned fir::cg::XReboxOp::getRank() {
  if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(
          fir::dyn_cast_ptrOrBoxEleTy(getBox().getType())))
    return seqTy.getDimension();
  return 0;
}

unsigned fir::cg::XArrayCoorOp::getRank() {
  auto memrefTy = getMemref().getType();
  if (mlir::isa<fir::BaseBoxType>(memrefTy))
    if (auto seqty = mlir::dyn_cast<fir::SequenceType>(
            fir::dyn_cast_ptrOrBoxEleTy(memrefTy)))
      return seqty.getDimension();
  return getShape().size();
}
