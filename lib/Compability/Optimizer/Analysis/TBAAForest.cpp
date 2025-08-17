//===- TBAAForest.cpp - Per-functon TBAA Trees ----------------------------===//
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

#include "language/Compability/Optimizer/Analysis/TBAAForest.h"
#include <mlir/Dialect/LLVMIR/LLVMAttrs.h>

mlir::LLVM::TBAATagAttr
fir::TBAATree::SubtreeState::getTag(toolchain::StringRef uniqueName) const {
  std::string id = (parentId + "/" + uniqueName).str();
  mlir::LLVM::TBAATypeDescriptorAttr type =
      mlir::LLVM::TBAATypeDescriptorAttr::get(
          context, id, mlir::LLVM::TBAAMemberAttr::get(parent, 0));
  return mlir::LLVM::TBAATagAttr::get(type, type, 0);
  // return tag;
}

mlir::LLVM::TBAATagAttr fir::TBAATree::SubtreeState::getTag() const {
  return mlir::LLVM::TBAATagAttr::get(parent, parent, 0);
}

fir::TBAATree fir::TBAATree::buildTree(mlir::StringAttr func) {
  toolchain::StringRef funcName = func.getValue();
  std::string rootId = ("Flang function root " + funcName).str();
  mlir::MLIRContext *ctx = func.getContext();
  mlir::LLVM::TBAARootAttr funcRoot =
      mlir::LLVM::TBAARootAttr::get(ctx, mlir::StringAttr::get(ctx, rootId));

  static constexpr toolchain::StringRef anyAccessTypeDescId = "any access";
  mlir::LLVM::TBAATypeDescriptorAttr anyAccess =
      mlir::LLVM::TBAATypeDescriptorAttr::get(
          ctx, anyAccessTypeDescId,
          mlir::LLVM::TBAAMemberAttr::get(funcRoot, 0));

  static constexpr toolchain::StringRef anyDataAccessTypeDescId = "any data access";
  mlir::LLVM::TBAATypeDescriptorAttr dataRoot =
      mlir::LLVM::TBAATypeDescriptorAttr::get(
          ctx, anyDataAccessTypeDescId,
          mlir::LLVM::TBAAMemberAttr::get(anyAccess, 0));

  static constexpr toolchain::StringRef boxMemberTypeDescId = "descriptor member";
  mlir::LLVM::TBAATypeDescriptorAttr boxMemberTypeDesc =
      mlir::LLVM::TBAATypeDescriptorAttr::get(
          ctx, boxMemberTypeDescId,
          mlir::LLVM::TBAAMemberAttr::get(anyAccess, 0));

  return TBAATree{anyAccess, dataRoot, boxMemberTypeDesc};
}

fir::TBAATree::TBAATree(mlir::LLVM::TBAATypeDescriptorAttr anyAccess,
                        mlir::LLVM::TBAATypeDescriptorAttr dataRoot,
                        mlir::LLVM::TBAATypeDescriptorAttr boxMemberTypeDesc)
    : targetDataTree(dataRoot.getContext(), "target data", dataRoot),
      globalDataTree(dataRoot.getContext(), "global data",
                     targetDataTree.getRoot()),
      allocatedDataTree(dataRoot.getContext(), "allocated data",
                        targetDataTree.getRoot()),
      dummyArgDataTree(dataRoot.getContext(), "dummy arg data", dataRoot),
      directDataTree(dataRoot.getContext(), "direct data",
                     targetDataTree.getRoot()),
      anyAccessDesc(anyAccess), boxMemberTypeDesc(boxMemberTypeDesc),
      anyDataTypeDesc(dataRoot) {}
