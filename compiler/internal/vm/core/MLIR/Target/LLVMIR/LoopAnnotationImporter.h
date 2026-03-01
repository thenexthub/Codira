//===- LoopAnnotationImporter.h ---------------------------------*- C++ -*-===//
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
// This file implements the translation between LLVMIR loop metadata and the
// corresponding MLIR representation.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONIMPORTER_H_
#define MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONIMPORTER_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Target/LLVMIR/ModuleImport.h"

namespace mlir {
namespace LLVM {
namespace detail {

/// A helper class that converts toolchain.loop metadata nodes into corresponding
/// LoopAnnotationAttrs and toolchain.access.group nodes into AccessGroupAttrs.
class LoopAnnotationImporter {
public:
  LoopAnnotationImporter(ModuleImport &moduleImport, OpBuilder &builder)
      : moduleImport(moduleImport), builder(builder) {}
  LoopAnnotationAttr translateLoopAnnotation(const toolchain::MDNode *node,
                                             Location loc);

  /// Converts all LLVM access groups starting from node to MLIR access group
  /// attributes. It stores a mapping from every nested access group node to the
  /// translated attribute. Returns success if all conversions succeed and
  /// failure otherwise.
  LogicalResult translateAccessGroup(const toolchain::MDNode *node, Location loc);

  /// Returns the access group attribute that map to the access group nodes
  /// starting from the access group metadata node. Returns failure, if any of
  /// the attributes cannot be found.
  FailureOr<SmallVector<AccessGroupAttr>>
  lookupAccessGroupAttrs(const toolchain::MDNode *node) const;

  /// The ModuleImport owning this instance.
  ModuleImport &moduleImport;

private:
  /// Returns the LLVM metadata corresponding to a toolchain loop metadata attribute.
  LoopAnnotationAttr lookupLoopMetadata(const toolchain::MDNode *node) const {
    return loopMetadataMapping.lookup(node);
  }

  void mapLoopMetadata(const toolchain::MDNode *metadata, LoopAnnotationAttr attr) {
    auto result = loopMetadataMapping.try_emplace(metadata, attr);
    (void)result;
    assert(result.second &&
           "attempting to map loop options that was already mapped");
  }

  OpBuilder &builder;
  DenseMap<const toolchain::MDNode *, LoopAnnotationAttr> loopMetadataMapping;
  /// Mapping between original LLVM access group metadata nodes and the imported
  /// MLIR access group attributes.
  DenseMap<const toolchain::MDNode *, AccessGroupAttr> accessGroupMapping;
};

} // namespace detail
} // namespace LLVM
} // namespace mlir

#endif // MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONIMPORTER_H_
