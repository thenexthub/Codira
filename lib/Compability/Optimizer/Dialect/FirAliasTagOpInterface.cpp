//===-- FirAliasTagOpInterface.cpp ----------------------------------===//
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

#include "language/Compability/Optimizer/Dialect/FirAliasTagOpInterface.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

#include "language/Compability/Optimizer/Dialect/FirAliasTagOpInterface.cpp.inc"

toolchain::LogicalResult
fir::detail::verifyFirAliasTagOpInterface(mlir::Operation *op) {
  auto iface = mlir::cast<FirAliasTagOpInterface>(op);

  mlir::ArrayAttr tags = iface.getTBAATagsOrNull();
  if (!tags)
    return toolchain::success();

  for (mlir::Attribute iter : tags)
    if (!mlir::isa<mlir::LLVM::TBAATagAttr>(iter))
      return op->emitOpError("expected op to return array of ")
             << mlir::LLVM::TBAATagAttr::getMnemonic() << " attributes";
  return toolchain::success();
}
