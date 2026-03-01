//===- DebugImporter.h - LLVM to MLIR Debug conversion -------*- C++ -*----===//
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
// This file implements the translation between LLVMIR debug information and
// the corresponding MLIR representation.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_TARGET_LLVMIR_DEBUGIMPORTER_H_
#define MLIR_LIB_TARGET_LLVMIR_DEBUGIMPORTER_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/CyclicReplacerCache.h"
#include "vm/core/ADT/MapVector.h"
#include "vm/core/IR/DebugInfoMetadata.h"

namespace mlir {
class Operation;

namespace LLVM {
class LLVMFuncOp;

namespace detail {

class DebugImporter {
public:
  DebugImporter(ModuleOp mlirModule, bool dropDICompositeTypeElements);

  /// Translates the given LLVM debug location to an MLIR location.
  Location translateLoc(toolchain::DILocation *loc);

  /// Translates the LLVM DWARF expression metadata to MLIR.
  DIExpressionAttr translateExpression(toolchain::DIExpression *node);

  /// Translates the LLVM DWARF global variable expression metadata to MLIR.
  DIGlobalVariableExpressionAttr
  translateGlobalVariableExpression(toolchain::DIGlobalVariableExpression *node);

  /// Translates the debug information for the given function into a Location.
  /// Returns UnknownLoc if `func` has no debug information attached to it.
  Location translateFuncLocation(toolchain::Function *func);

  /// Translates the given LLVM debug metadata to MLIR.
  DINodeAttr translate(toolchain::DINode *node);

  /// Infers the metadata type and translates it to MLIR.
  template <typename DINodeT>
  auto translate(DINodeT *node) {
    // Infer the MLIR type from the LLVM metadata type.
    using MLIRTypeT = decltype(translateImpl(node));
    return cast_or_null<MLIRTypeT>(
        translate(static_cast<toolchain::DINode *>(node)));
  }

private:
  /// Translates the given LLVM debug metadata to the corresponding attribute.
  DIBasicTypeAttr translateImpl(toolchain::DIBasicType *node);
  DICompileUnitAttr translateImpl(toolchain::DICompileUnit *node);
  DICompositeTypeAttr translateImpl(toolchain::DICompositeType *node);
  DIDerivedTypeAttr translateImpl(toolchain::DIDerivedType *node);
  DIStringTypeAttr translateImpl(toolchain::DIStringType *node);
  DIFileAttr translateImpl(toolchain::DIFile *node);
  DILabelAttr translateImpl(toolchain::DILabel *node);
  DILexicalBlockAttr translateImpl(toolchain::DILexicalBlock *node);
  DILexicalBlockFileAttr translateImpl(toolchain::DILexicalBlockFile *node);
  DIGlobalVariableAttr translateImpl(toolchain::DIGlobalVariable *node);
  DILocalVariableAttr translateImpl(toolchain::DILocalVariable *node);
  DIVariableAttr translateImpl(toolchain::DIVariable *node);
  DIModuleAttr translateImpl(toolchain::DIModule *node);
  DINamespaceAttr translateImpl(toolchain::DINamespace *node);
  DIImportedEntityAttr translateImpl(toolchain::DIImportedEntity *node);
  DIScopeAttr translateImpl(toolchain::DIScope *node);
  DISubprogramAttr translateImpl(toolchain::DISubprogram *node);
  DISubrangeAttr translateImpl(toolchain::DISubrange *node);
  DIGenericSubrangeAttr translateImpl(toolchain::DIGenericSubrange *node);
  DICommonBlockAttr translateImpl(toolchain::DICommonBlock *node);
  DISubroutineTypeAttr translateImpl(toolchain::DISubroutineType *node);
  DITypeAttr translateImpl(toolchain::DIType *node);

  /// Constructs a StringAttr from the MDString if it is non-null. Returns a
  /// null attribute otherwise.
  StringAttr getStringAttrOrNull(toolchain::MDString *stringNode);

  /// Get the DistinctAttr used to represent `node` if one was already created
  /// for it, or create a new one if not.
  DistinctAttr getOrCreateDistinctID(toolchain::DINode *node);

  std::optional<DINodeAttr> createRecSelf(toolchain::DINode *node);

  /// A mapping between distinct LLVM debug metadata nodes and the corresponding
  /// distinct id attribute.
  DenseMap<toolchain::DINode *, DistinctAttr> nodeToDistinctAttr;

  /// A mapping between DINodes that are recursive, and their assigned recId.
  /// This is kept so that repeated occurrences of the same node can reuse the
  /// same ID and be deduplicated.
  DenseMap<toolchain::DINode *, DistinctAttr> nodeToRecId;

  CyclicReplacerCache<toolchain::DINode *, DINodeAttr> cache;

  MLIRContext *context;
  ModuleOp mlirModule;

  /// An option to control if DICompositeTypes should always be imported without
  /// converting their elements. If set, the option avoids the recursive
  /// traversal of composite type debug information, which can be expensive for
  /// adversarial inputs.
  bool dropDICompositeTypeElements;
};

} // namespace detail
} // namespace LLVM
} // namespace mlir

#endif // MLIR_LIB_TARGET_LLVMIR_DEBUGIMPORTER_H_
