//===- DebugTranslation.h - MLIR to LLVM Debug conversion -------*- C++ -*-===//
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
// This file implements the translation between an MLIR debug information and
// the corresponding LLVMIR representation.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_LIB_TARGET_LLVMIR_DEBUGTRANSLATION_H_
#define MLIR_LIB_TARGET_LLVMIR_DEBUGTRANSLATION_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Location.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/IR/DIBuilder.h"

namespace mlir {
class Operation;

namespace LLVM {
class LLVMFuncOp;

namespace detail {
class DebugTranslation {
public:
  DebugTranslation(Operation *module, toolchain::Module &llvmModule);

  /// Adds the necessary module flags to the module, if not yet present.
  void addModuleFlagsIfNotPresent();

  /// Translate the given location to an toolchain debug location.
  toolchain::DILocation *translateLoc(Location loc, toolchain::DILocalScope *scope);

  /// Translates the given DWARF expression metadata to to LLVM.
  toolchain::DIExpression *translateExpression(LLVM::DIExpressionAttr attr);

  /// Translates the given DWARF global variable expression to LLVM.
  toolchain::DIGlobalVariableExpression *
  translateGlobalVariableExpression(LLVM::DIGlobalVariableExpressionAttr attr);

  /// Translate the debug information for the given function.
  void translate(LLVMFuncOp func, toolchain::Function &llvmFunc);

  /// Translate the given LLVM debug metadata to LLVM.
  toolchain::DINode *translate(DINodeAttr attr);

  /// Translate the given derived LLVM debug metadata to LLVM.
  template <typename DIAttrT>
  auto translate(DIAttrT attr) {
    // Infer the LLVM type from the attribute type.
    using LLVMTypeT = std::remove_pointer_t<decltype(translateImpl(attr))>;
    return cast_or_null<LLVMTypeT>(translate(DINodeAttr(attr)));
  }

private:
  /// Translate the given location to an toolchain debug location with the given
  /// scope and inlinedAt parameters.
  toolchain::DILocation *translateLoc(Location loc, toolchain::DILocalScope *scope,
                                 toolchain::DILocation *inlinedAt);

  /// Create an toolchain debug file for the given file path.
  toolchain::DIFile *translateFile(StringRef fileName);

  /// Translate the given attribute to the corresponding toolchain debug metadata.
  toolchain::DIType *translateImpl(DINullTypeAttr attr);
  toolchain::DIBasicType *translateImpl(DIBasicTypeAttr attr);
  toolchain::DICompileUnit *translateImpl(DICompileUnitAttr attr);
  toolchain::DICompositeType *translateImpl(DICompositeTypeAttr attr);
  toolchain::DIDerivedType *translateImpl(DIDerivedTypeAttr attr);
  toolchain::DIStringType *translateImpl(DIStringTypeAttr attr);
  toolchain::DIFile *translateImpl(DIFileAttr attr);
  toolchain::DIImportedEntity *translateImpl(DIImportedEntityAttr attr);
  toolchain::DILabel *translateImpl(DILabelAttr attr);
  toolchain::DILexicalBlock *translateImpl(DILexicalBlockAttr attr);
  toolchain::DILexicalBlockFile *translateImpl(DILexicalBlockFileAttr attr);
  toolchain::DILocalScope *translateImpl(DILocalScopeAttr attr);
  toolchain::DILocalVariable *translateImpl(DILocalVariableAttr attr);
  toolchain::DIGlobalVariable *translateImpl(DIGlobalVariableAttr attr);
  toolchain::DIVariable *translateImpl(DIVariableAttr attr);
  toolchain::DIModule *translateImpl(DIModuleAttr attr);
  toolchain::DINamespace *translateImpl(DINamespaceAttr attr);
  toolchain::DIScope *translateImpl(DIScopeAttr attr);
  toolchain::DISubprogram *translateImpl(DISubprogramAttr attr);
  toolchain::DIGenericSubrange *translateImpl(DIGenericSubrangeAttr attr);
  toolchain::DISubrange *translateImpl(DISubrangeAttr attr);
  toolchain::DICommonBlock *translateImpl(DICommonBlockAttr attr);
  toolchain::DISubroutineType *translateImpl(DISubroutineTypeAttr attr);
  toolchain::DIType *translateImpl(DITypeAttr attr);

  /// Attributes that support self recursion need to implement an additional
  /// method to hook into `translateRecursive`.
  /// - `<temp toolchain type> translateTemporaryImpl(<mlir type>)`:
  ///   Create a temporary translation of the DI attr without recursively
  ///   translating any nested DI attrs.
  toolchain::DINode *translateRecursive(DIRecursiveTypeAttrInterface attr);

  /// Translate the given attribute to a temporary toolchain debug metadata of the
  /// corresponding type.
  toolchain::TempDICompositeType translateTemporaryImpl(DICompositeTypeAttr attr);
  toolchain::TempDISubprogram translateTemporaryImpl(DISubprogramAttr attr);

  /// Constructs a string metadata node from the string attribute. Returns
  /// nullptr if `stringAttr` is null or contains and empty string.
  toolchain::MDString *getMDStringOrNull(StringAttr stringAttr);

  /// Constructs a tuple metadata node from the `elements`. Returns nullptr if
  /// `elements` is empty.
  toolchain::MDTuple *getMDTupleOrNull(ArrayRef<DINodeAttr> elements);

  /// Constructs a DIExpression metadata node from the DIExpressionAttr. Returns
  /// nullptr if `DIExpressionAttr` is null.
  toolchain::DIExpression *getExpressionAttrOrNull(DIExpressionAttr attr);

  /// A mapping between mlir location+scope and the corresponding toolchain debug
  /// metadata.
  DenseMap<std::tuple<Location, toolchain::DILocalScope *, const toolchain::DILocation *>,
           toolchain::DILocation *>
      locationToLoc;

  /// A mapping between debug attribute and the corresponding toolchain debug
  /// metadata.
  DenseMap<Attribute, toolchain::DINode *> attrToNode;

  /// A mapping between recursive ID and the translated DINode.
  toolchain::MapVector<DistinctAttr, toolchain::DINode *> recursiveNodeMap;

  /// A mapping between a distinct ID and the translated LLVM metadata node.
  /// This helps identify attrs that should translate into the same LLVM debug
  /// node.
  DenseMap<DistinctAttr, toolchain::DINode *> distinctAttrToNode;

  /// A mapping between filename and toolchain debug file.
  /// TODO: Change this to DenseMap<Identifier, ...> when we can
  /// access the Identifier filename in FileLineColLoc.
  toolchain::StringMap<toolchain::DIFile *> fileMap;

  /// A string containing the current working directory of the compiler.
  SmallString<256> currentWorkingDir;

  /// Flag indicating if debug information should be emitted.
  bool debugEmissionIsEnabled;

  /// Debug information fields.
  toolchain::Module &llvmModule;
  toolchain::LLVMContext &llvmCtx;
};

} // namespace detail
} // namespace LLVM
} // namespace mlir

#endif // MLIR_LIB_TARGET_LLVMIR_DEBUGTRANSLATION_H_
