//===- StructBuilder.cpp - Helper for building LLVM structs  --------------===//
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

#include "mlir/Conversion/LLVMCommon/StructBuilder.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/Builders.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// StructBuilder implementation
//===----------------------------------------------------------------------===//

StructBuilder::StructBuilder(Value v) : value(v), structType(v.getType()) {
  assert(value != nullptr && "value cannot be null");
  assert(LLVM::isCompatibleType(structType) && "expected toolchain type");
}

Value StructBuilder::extractPtr(OpBuilder &builder, Location loc,
                                unsigned pos) const {
  return LLVM::ExtractValueOp::create(builder, loc, value, pos);
}

void StructBuilder::setPtr(OpBuilder &builder, Location loc, unsigned pos,
                           Value ptr) {
  value = LLVM::InsertValueOp::create(builder, loc, value, ptr, pos);
}
