//===- LoopAnnotationTranslation.h ------------------------------*- C++ -*-===//
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
// This file implements the translation between an MLIR loop annotations and
// the corresponding LLVMIR metadata representation.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONTRANSLATION_H_
#define MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONTRANSLATION_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

namespace mlir {
namespace LLVM {
namespace detail {

/// A helper class that converts LoopAnnotationAttrs and AccessGroupAttrs into
/// corresponding toolchain::MDNodes.
class LoopAnnotationTranslation {
public:
  LoopAnnotationTranslation(ModuleTranslation &moduleTranslation,
                            toolchain::Module &llvmModule)
      : moduleTranslation(moduleTranslation), llvmModule(llvmModule) {}

  toolchain::MDNode *translateLoopAnnotation(LoopAnnotationAttr attr, Operation *op);

  /// Returns the LLVM metadata corresponding to an mlir LLVM dialect access
  /// group attribute.
  toolchain::MDNode *getAccessGroup(AccessGroupAttr accessGroupAttr);

  /// Returns the LLVM metadata corresponding to the access group attribute
  /// referenced by the AccessGroupOpInterface or null if there are none.
  toolchain::MDNode *getAccessGroups(AccessGroupOpInterface op);

  /// The ModuleTranslation owning this instance.
  ModuleTranslation &moduleTranslation;

private:
  /// Returns the LLVM metadata corresponding to a toolchain loop metadata attribute.
  toolchain::MDNode *lookupLoopMetadata(Attribute options) const {
    return loopMetadataMapping.lookup(options);
  }

  void mapLoopMetadata(Attribute options, toolchain::MDNode *metadata) {
    auto result = loopMetadataMapping.try_emplace(options, metadata);
    (void)result;
    assert(result.second &&
           "attempting to map loop options that was already mapped");
  }

  /// Mapping from an attribute describing loop metadata to its LLVM metadata.
  /// The metadata is attached to Latch block branches with this attribute.
  DenseMap<Attribute, toolchain::MDNode *> loopMetadataMapping;

  /// Mapping from an access group attribute to its LLVM metadata.
  /// This map is populated on module entry and is used to annotate loops (as
  /// identified via their branches) and contained memory accesses.
  DenseMap<AccessGroupAttr, toolchain::MDNode *> accessGroupMetadataMapping;

  toolchain::Module &llvmModule;
};

} // namespace detail
} // namespace LLVM
} // namespace mlir

#endif // MLIR_LIB_TARGET_LLVMIR_LOOPANNOTATIONTRANSLATION_H_
