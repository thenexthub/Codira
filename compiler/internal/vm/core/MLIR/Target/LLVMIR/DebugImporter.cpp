//===- DebugImporter.cpp - LLVM to MLIR Debug conversion ------------------===//
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

#include "DebugImporter.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Location.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Metadata.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

DebugImporter::DebugImporter(ModuleOp mlirModule,
                             bool dropDICompositeTypeElements)
    : cache([&](toolchain::DINode *node) { return createRecSelf(node); }),
      context(mlirModule.getContext()), mlirModule(mlirModule),
      dropDICompositeTypeElements(dropDICompositeTypeElements) {}

Location DebugImporter::translateFuncLocation(toolchain::Function *func) {
  toolchain::DISubprogram *subprogram = func->getSubprogram();
  if (!subprogram)
    return UnknownLoc::get(context);

  // Add a fused location to link the subprogram information.
  StringAttr fileName = StringAttr::get(context, subprogram->getFilename());
  return FusedLocWith<DISubprogramAttr>::get(
      {FileLineColLoc::get(fileName, subprogram->getLine(), /*column=*/0)},
      translate(subprogram), context);
}

//===----------------------------------------------------------------------===//
// Attributes
//===----------------------------------------------------------------------===//

DIBasicTypeAttr DebugImporter::translateImpl(toolchain::DIBasicType *node) {
  return DIBasicTypeAttr::get(context, node->getTag(), node->getName(),
                              node->getSizeInBits(), node->getEncoding());
}

DICompileUnitAttr DebugImporter::translateImpl(toolchain::DICompileUnit *node) {
  std::optional<DIEmissionKind> emissionKind =
      symbolizeDIEmissionKind(node->getEmissionKind());
  std::optional<DINameTableKind> nameTableKind = symbolizeDINameTableKind(
      static_cast<
          std::underlying_type_t<toolchain::DICompileUnit::DebugNameTableKind>>(
          node->getNameTableKind()));
  return DICompileUnitAttr::get(
      context, getOrCreateDistinctID(node),
      node->getSourceLanguage().getUnversionedName(),
      translate(node->getFile()), getStringAttrOrNull(node->getRawProducer()),
      node->isOptimized(), emissionKind.value(), nameTableKind.value(),
      getStringAttrOrNull(node->getRawSplitDebugFilename()));
}

DICompositeTypeAttr DebugImporter::translateImpl(toolchain::DICompositeType *node) {
  std::optional<DIFlags> flags = symbolizeDIFlags(node->getFlags());
  SmallVector<DINodeAttr> elements;

  // A vector always requires an element.
  bool isVectorType = flags && bitEnumContainsAll(*flags, DIFlags::Vector);
  if (isVectorType || !dropDICompositeTypeElements) {
    for (toolchain::DINode *element : node->getElements()) {
      assert(element && "expected a non-null element type");
      elements.push_back(translate(element));
    }
  }
  // Drop the elements parameter if any of the elements are invalid.
  if (toolchain::is_contained(elements, nullptr))
    elements.clear();
  DITypeAttr baseType = translate(node->getBaseType());
  // Arrays require a base type, otherwise the debug metadata is considered to
  // be malformed.
  if (node->getTag() == toolchain::dwarf::DW_TAG_array_type && !baseType)
    return nullptr;
  return DICompositeTypeAttr::get(
      context, node->getTag(), getStringAttrOrNull(node->getRawName()),
      translate(node->getFile()), node->getLine(), translate(node->getScope()),
      baseType, flags.value_or(DIFlags::Zero), node->getSizeInBits(),
      node->getAlignInBits(), translateExpression(node->getDataLocationExp()),
      translateExpression(node->getRankExp()),
      translateExpression(node->getAllocatedExp()),
      translateExpression(node->getAssociatedExp()), elements);
}

DIDerivedTypeAttr DebugImporter::translateImpl(toolchain::DIDerivedType *node) {
  // Return nullptr if the base type is invalid.
  DITypeAttr baseType = translate(node->getBaseType());
  if (node->getBaseType() && !baseType)
    return nullptr;
  DINodeAttr extraData =
      translate(dyn_cast_or_null<toolchain::DINode>(node->getExtraData()));
  return DIDerivedTypeAttr::get(
      context, node->getTag(), getStringAttrOrNull(node->getRawName()),
      baseType, node->getSizeInBits(), node->getAlignInBits(),
      node->getOffsetInBits(), node->getDWARFAddressSpace(), extraData);
}

DIStringTypeAttr DebugImporter::translateImpl(toolchain::DIStringType *node) {
  return DIStringTypeAttr::get(
      context, node->getTag(), getStringAttrOrNull(node->getRawName()),
      node->getSizeInBits(), node->getAlignInBits(),
      translate(node->getStringLength()),
      translateExpression(node->getStringLengthExp()),
      translateExpression(node->getStringLocationExp()), node->getEncoding());
}

DIFileAttr DebugImporter::translateImpl(toolchain::DIFile *node) {
  return DIFileAttr::get(context, node->getFilename(), node->getDirectory());
}

DILabelAttr DebugImporter::translateImpl(toolchain::DILabel *node) {
  // Return nullptr if the scope or type is a cyclic dependency.
  DIScopeAttr scope = translate(node->getScope());
  if (node->getScope() && !scope)
    return nullptr;
  return DILabelAttr::get(context, scope,
                          getStringAttrOrNull(node->getRawName()),
                          translate(node->getFile()), node->getLine());
}

DILexicalBlockAttr DebugImporter::translateImpl(toolchain::DILexicalBlock *node) {
  // Return nullptr if the scope or type is a cyclic dependency.
  DIScopeAttr scope = translate(node->getScope());
  if (node->getScope() && !scope)
    return nullptr;
  return DILexicalBlockAttr::get(context, scope, translate(node->getFile()),
                                 node->getLine(), node->getColumn());
}

DILexicalBlockFileAttr
DebugImporter::translateImpl(toolchain::DILexicalBlockFile *node) {
  // Return nullptr if the scope or type is a cyclic dependency.
  DIScopeAttr scope = translate(node->getScope());
  if (node->getScope() && !scope)
    return nullptr;
  return DILexicalBlockFileAttr::get(context, scope, translate(node->getFile()),
                                     node->getDiscriminator());
}

DIGlobalVariableAttr
DebugImporter::translateImpl(toolchain::DIGlobalVariable *node) {
  // Names of DIGlobalVariables can be empty. MLIR models them as null, instead
  // of empty strings, so this special handling is necessary.
  auto convertToStringAttr = [&](StringRef name) -> StringAttr {
    if (name.empty())
      return {};
    return StringAttr::get(context, node->getName());
  };
  return DIGlobalVariableAttr::get(
      context, translate(node->getScope()),
      convertToStringAttr(node->getName()),
      convertToStringAttr(node->getLinkageName()), translate(node->getFile()),
      node->getLine(), translate(node->getType()), node->isLocalToUnit(),
      node->isDefinition(), node->getAlignInBits());
}

DILocalVariableAttr DebugImporter::translateImpl(toolchain::DILocalVariable *node) {
  // Return nullptr if the scope or type is a cyclic dependency.
  DIScopeAttr scope = translate(node->getScope());
  if (node->getScope() && !scope)
    return nullptr;
  return DILocalVariableAttr::get(
      context, scope, getStringAttrOrNull(node->getRawName()),
      translate(node->getFile()), node->getLine(), node->getArg(),
      node->getAlignInBits(), translate(node->getType()),
      symbolizeDIFlags(node->getFlags()).value_or(DIFlags::Zero));
}

DIVariableAttr DebugImporter::translateImpl(toolchain::DIVariable *node) {
  return cast<DIVariableAttr>(translate(static_cast<toolchain::DINode *>(node)));
}

DIScopeAttr DebugImporter::translateImpl(toolchain::DIScope *node) {
  return cast<DIScopeAttr>(translate(static_cast<toolchain::DINode *>(node)));
}

DIModuleAttr DebugImporter::translateImpl(toolchain::DIModule *node) {
  return DIModuleAttr::get(
      context, translate(node->getFile()), translate(node->getScope()),
      getStringAttrOrNull(node->getRawName()),
      getStringAttrOrNull(node->getRawConfigurationMacros()),
      getStringAttrOrNull(node->getRawIncludePath()),
      getStringAttrOrNull(node->getRawAPINotesFile()), node->getLineNo(),
      node->getIsDecl());
}

DINamespaceAttr DebugImporter::translateImpl(toolchain::DINamespace *node) {
  return DINamespaceAttr::get(context, getStringAttrOrNull(node->getRawName()),
                              translate(node->getScope()),
                              node->getExportSymbols());
}

DIImportedEntityAttr
DebugImporter::translateImpl(toolchain::DIImportedEntity *node) {
  SmallVector<DINodeAttr> elements;
  for (toolchain::DINode *element : node->getElements()) {
    assert(element && "expected a non-null element type");
    elements.push_back(translate(element));
  }

  return DIImportedEntityAttr::get(
      context, node->getTag(), translate(node->getScope()),
      translate(node->getEntity()), translate(node->getFile()), node->getLine(),
      getStringAttrOrNull(node->getRawName()), elements);
}

DISubprogramAttr DebugImporter::translateImpl(toolchain::DISubprogram *node) {
  // Only definitions require a distinct identifier.
  mlir::DistinctAttr id;
  if (node->isDistinct())
    id = getOrCreateDistinctID(node);

  // Return nullptr if the scope or type is invalid.
  DIScopeAttr scope = translate(node->getScope());
  if (node->getScope() && !scope)
    return nullptr;
  std::optional<DISubprogramFlags> subprogramFlags =
      symbolizeDISubprogramFlags(node->getSubprogram()->getSPFlags());
  assert(subprogramFlags && "expected valid subprogram flags");
  DISubroutineTypeAttr type = translate(node->getType());
  if (node->getType() && !type)
    return nullptr;

  // Convert the retained nodes but drop all of them if one of them is invalid.
  SmallVector<DINodeAttr> retainedNodes;
  for (toolchain::DINode *retainedNode : node->getRetainedNodes())
    retainedNodes.push_back(translate(retainedNode));
  if (toolchain::is_contained(retainedNodes, nullptr))
    retainedNodes.clear();

  SmallVector<DINodeAttr> annotations;
  // We currently only support `string` values for annotations on the MLIR side.
  // Theoretically we could support other primitives, but LLVM is not using
  // other types in practice.
  if (toolchain::DINodeArray rawAnns = node->getAnnotations(); rawAnns) {
    for (size_t i = 0, e = rawAnns->getNumOperands(); i < e; ++i) {
      const toolchain::MDTuple *tuple = cast<toolchain::MDTuple>(rawAnns->getOperand(i));
      if (tuple->getNumOperands() != 2)
        continue;
      const toolchain::MDString *name = cast<toolchain::MDString>(tuple->getOperand(0));
      const toolchain::MDString *value =
          dyn_cast<toolchain::MDString>(tuple->getOperand(1));
      if (name && value) {
        annotations.push_back(DIAnnotationAttr::get(
            context, StringAttr::get(context, name->getString()),
            StringAttr::get(context, value->getString())));
      }
    }
  }

  return DISubprogramAttr::get(context, id, translate(node->getUnit()), scope,
                               getStringAttrOrNull(node->getRawName()),
                               getStringAttrOrNull(node->getRawLinkageName()),
                               translate(node->getFile()), node->getLine(),
                               node->getScopeLine(), *subprogramFlags, type,
                               retainedNodes, annotations);
}

DISubrangeAttr DebugImporter::translateImpl(toolchain::DISubrange *node) {
  auto getAttrOrNull = [&](toolchain::DISubrange::BoundType data) -> Attribute {
    if (data.isNull())
      return nullptr;
    if (auto *constInt = dyn_cast<toolchain::ConstantInt *>(data))
      return IntegerAttr::get(IntegerType::get(context, 64),
                              constInt->getSExtValue());
    if (auto *expr = dyn_cast<toolchain::DIExpression *>(data))
      return translateExpression(expr);
    if (auto *var = dyn_cast<toolchain::DIVariable *>(data)) {
      if (auto *local = dyn_cast<toolchain::DILocalVariable>(var))
        return translate(local);
      if (auto *global = dyn_cast<toolchain::DIGlobalVariable>(var))
        return translate(global);
      return nullptr;
    }
    return nullptr;
  };
  Attribute count = getAttrOrNull(node->getCount());
  Attribute upperBound = getAttrOrNull(node->getUpperBound());
  // Either count or the upper bound needs to be present. Otherwise, the
  // metadata is invalid. The conversion might fail due to unsupported DI nodes.
  if (!count && !upperBound)
    return {};
  return DISubrangeAttr::get(context, count,
                             getAttrOrNull(node->getLowerBound()), upperBound,
                             getAttrOrNull(node->getStride()));
}

DICommonBlockAttr DebugImporter::translateImpl(toolchain::DICommonBlock *node) {
  return DICommonBlockAttr::get(context, translate(node->getScope()),
                                translate(node->getDecl()),
                                getStringAttrOrNull(node->getRawName()),
                                translate(node->getFile()), node->getLineNo());
}

DIGenericSubrangeAttr
DebugImporter::translateImpl(toolchain::DIGenericSubrange *node) {
  auto getAttrOrNull =
      [&](toolchain::DIGenericSubrange::BoundType data) -> Attribute {
    if (data.isNull())
      return nullptr;
    if (auto *expr = dyn_cast<toolchain::DIExpression *>(data))
      return translateExpression(expr);
    if (auto *var = dyn_cast<toolchain::DIVariable *>(data)) {
      if (auto *local = dyn_cast<toolchain::DILocalVariable>(var))
        return translate(local);
      if (auto *global = dyn_cast<toolchain::DIGlobalVariable>(var))
        return translate(global);
      return nullptr;
    }
    return nullptr;
  };
  Attribute count = getAttrOrNull(node->getCount());
  Attribute upperBound = getAttrOrNull(node->getUpperBound());
  Attribute lowerBound = getAttrOrNull(node->getLowerBound());
  Attribute stride = getAttrOrNull(node->getStride());
  // Either count or the upper bound needs to be present. Otherwise, the
  // metadata is invalid.
  if (!count && !upperBound)
    return {};
  return DIGenericSubrangeAttr::get(context, count, lowerBound, upperBound,
                                    stride);
}

DISubroutineTypeAttr
DebugImporter::translateImpl(toolchain::DISubroutineType *node) {
  SmallVector<DITypeAttr> types;
  for (toolchain::DIType *type : node->getTypeArray()) {
    if (!type) {
      // A nullptr entry may appear at the beginning or the end of the
      // subroutine types list modeling either a void result type or the type of
      // a variadic argument. Translate the nullptr to an explicit
      // DINullTypeAttr since the attribute list cannot contain a nullptr entry.
      types.push_back(DINullTypeAttr::get(context));
      continue;
    }
    types.push_back(translate(type));
  }
  // Return nullptr if any of the types is invalid.
  if (toolchain::is_contained(types, nullptr))
    return nullptr;
  return DISubroutineTypeAttr::get(context, node->getCC(), types);
}

DITypeAttr DebugImporter::translateImpl(toolchain::DIType *node) {
  return cast<DITypeAttr>(translate(static_cast<toolchain::DINode *>(node)));
}

DINodeAttr DebugImporter::translate(toolchain::DINode *node) {
  if (!node)
    return nullptr;

  // Check for a cached instance.
  auto cacheEntry = cache.lookupOrInit(node);
  if (std::optional<DINodeAttr> result = cacheEntry.get())
    return *result;

  // Convert the debug metadata if possible.
  auto translateNode = [this](toolchain::DINode *node) -> DINodeAttr {
    if (auto *casted = dyn_cast<toolchain::DIBasicType>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DICommonBlock>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DICompileUnit>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DICompositeType>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIDerivedType>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIStringType>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIFile>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIGlobalVariable>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIImportedEntity>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DILabel>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DILexicalBlock>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DILexicalBlockFile>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DILocalVariable>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIModule>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DINamespace>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DISubprogram>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DISubrange>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DIGenericSubrange>(node))
      return translateImpl(casted);
    if (auto *casted = dyn_cast<toolchain::DISubroutineType>(node))
      return translateImpl(casted);
    return nullptr;
  };
  if (DINodeAttr attr = translateNode(node)) {
    // If this node was repeated, lookup its recursive ID and assign it to the
    // base result.
    if (cacheEntry.wasRepeated()) {
      DistinctAttr recId = nodeToRecId.lookup(node);
      auto recType = cast<DIRecursiveTypeAttrInterface>(attr);
      attr = cast<DINodeAttr>(recType.withRecId(recId));
    }
    cacheEntry.resolve(attr);
    return attr;
  }
  cacheEntry.resolve(nullptr);
  return nullptr;
}

/// Get the `getRecSelf` constructor for the translated type of `node` if its
/// translated DITypeAttr supports recursion. Otherwise, returns nullptr.
static function_ref<DIRecursiveTypeAttrInterface(DistinctAttr)>
getRecSelfConstructor(toolchain::DINode *node) {
  using CtorType = function_ref<DIRecursiveTypeAttrInterface(DistinctAttr)>;
  return TypeSwitch<toolchain::DINode *, CtorType>(node)
      .Case([&](toolchain::DICompositeType *) {
        return CtorType(DICompositeTypeAttr::getRecSelf);
      })
      .Case([&](toolchain::DISubprogram *) {
        return CtorType(DISubprogramAttr::getRecSelf);
      })
      .Default(CtorType());
}

std::optional<DINodeAttr> DebugImporter::createRecSelf(toolchain::DINode *node) {
  auto recSelfCtor = getRecSelfConstructor(node);
  if (!recSelfCtor)
    return std::nullopt;

  // The original node may have already been assigned a recursive ID from
  // a different self-reference. Use that if possible.
  DistinctAttr recId = nodeToRecId.lookup(node);
  if (!recId) {
    recId = DistinctAttr::create(UnitAttr::get(context));
    nodeToRecId[node] = recId;
  }
  DIRecursiveTypeAttrInterface recSelf = recSelfCtor(recId);
  return cast<DINodeAttr>(recSelf);
}

//===----------------------------------------------------------------------===//
// Locations
//===----------------------------------------------------------------------===//

Location DebugImporter::translateLoc(toolchain::DILocation *loc) {
  if (!loc)
    return UnknownLoc::get(context);

  // Get the file location of the instruction.
  Location result = FileLineColLoc::get(context, loc->getFilename(),
                                        loc->getLine(), loc->getColumn());

  // Add scope information.
  assert(loc->getScope() && "expected non-null scope");
  result = FusedLocWith<DIScopeAttr>::get({result}, translate(loc->getScope()),
                                          context);

  // Add call site information, if available.
  if (toolchain::DILocation *inlinedAt = loc->getInlinedAt())
    result = CallSiteLoc::get(result, translateLoc(inlinedAt));

  return result;
}

DIExpressionAttr DebugImporter::translateExpression(toolchain::DIExpression *node) {
  if (!node)
    return nullptr;

  SmallVector<DIExpressionElemAttr> ops;

  // Begin processing the operations.
  for (const toolchain::DIExpression::ExprOperand &op : node->expr_ops()) {
    SmallVector<uint64_t> operands;
    operands.reserve(op.getNumArgs());
    for (const auto &i : toolchain::seq(op.getNumArgs()))
      operands.push_back(op.getArg(i));
    const auto attr = DIExpressionElemAttr::get(context, op.getOp(), operands);
    ops.push_back(attr);
  }
  return DIExpressionAttr::get(context, ops);
}

DIGlobalVariableExpressionAttr DebugImporter::translateGlobalVariableExpression(
    toolchain::DIGlobalVariableExpression *node) {
  return DIGlobalVariableExpressionAttr::get(
      context, translate(node->getVariable()),
      translateExpression(node->getExpression()));
}

StringAttr DebugImporter::getStringAttrOrNull(toolchain::MDString *stringNode) {
  if (!stringNode)
    return StringAttr();
  return StringAttr::get(context, stringNode->getString());
}

DistinctAttr DebugImporter::getOrCreateDistinctID(toolchain::DINode *node) {
  DistinctAttr &id = nodeToDistinctAttr[node];
  if (!id)
    id = DistinctAttr::create(UnitAttr::get(context));
  return id;
}
