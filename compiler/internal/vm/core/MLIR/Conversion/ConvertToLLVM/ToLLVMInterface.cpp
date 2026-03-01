//===- ToLLVMInterface.cpp - MLIR LLVM Conversion -------------------------===//
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

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/Operation.h"

using namespace mlir;

void mlir::populateConversionTargetFromOperation(
    Operation *root, ConversionTarget &target, LLVMTypeConverter &typeConverter,
    RewritePatternSet &patterns) {
  DenseSet<Dialect *> dialects;
  root->walk([&](Operation *op) {
    Dialect *dialect = op->getDialect();
    if (!dialects.insert(dialect).second)
      return;
    // First time we encounter this dialect: if it implements the interface,
    // let's populate patterns !
    auto *iface = dyn_cast<ConvertToLLVMPatternInterface>(dialect);
    if (!iface)
      return;
    iface->populateConvertToLLVMConversionPatterns(target, typeConverter,
                                                   patterns);
  });
}

void mlir::populateOpConvertToLLVMConversionPatterns(
    Operation *op, ConversionTarget &target, LLVMTypeConverter &typeConverter,
    RewritePatternSet &patterns) {
  auto iface = dyn_cast<ConvertToLLVMOpInterface>(op);
  if (!iface)
    iface = op->getParentOfType<ConvertToLLVMOpInterface>();
  if (!iface)
    return;
  SmallVector<ConvertToLLVMAttrInterface, 12> attrs;
  iface.getConvertToLLVMConversionAttrs(attrs);
  for (ConvertToLLVMAttrInterface attr : attrs)
    attr.populateConvertToLLVMConversionPatterns(target, typeConverter,
                                                 patterns);
}

#include "mlir/Conversion/ConvertToLLVM/ToLLVMAttrInterface.cpp.inc"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMOpInterface.cpp.inc"
