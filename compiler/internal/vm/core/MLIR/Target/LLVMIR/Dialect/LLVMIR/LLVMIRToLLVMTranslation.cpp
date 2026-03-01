//===- LLVMIRToLLVMTranslation.cpp - Translate LLVM IR to LLVM dialect ----===//
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
// This file implements a translation between LLVM IR and the MLIR LLVM dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMIRToLLVMTranslation.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMInterfaces.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/ModuleImport.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/InlineAsm.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/MemoryModelRelaxationAnnotations.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

#include "mlir/Dialect/LLVMIR/LLVMConversionEnumsFromLLVM.inc"

static constexpr StringLiteral vecTypeHintMDName = "vec_type_hint";
static constexpr StringLiteral workGroupSizeHintMDName = "work_group_size_hint";
static constexpr StringLiteral reqdWorkGroupSizeMDName = "reqd_work_group_size";
static constexpr StringLiteral intelReqdSubGroupSizeMDName =
    "intel_reqd_sub_group_size";

/// Returns true if the LLVM IR intrinsic is convertible to an MLIR LLVM dialect
/// intrinsic. Returns false otherwise.
static bool isConvertibleIntrinsic(toolchain::Intrinsic::ID id) {
  static const DenseSet<unsigned> convertibleIntrinsics = {
#include "mlir/Dialect/LLVMIR/LLVMConvertibleLLVMIRIntrinsics.inc"
  };
  return convertibleIntrinsics.contains(id);
}

/// Returns the list of LLVM IR intrinsic identifiers that are convertible to
/// MLIR LLVM dialect intrinsics.
static ArrayRef<unsigned> getSupportedIntrinsicsImpl() {
  static const SmallVector<unsigned> convertibleIntrinsics = {
#include "mlir/Dialect/LLVMIR/LLVMConvertibleLLVMIRIntrinsics.inc"
  };
  return convertibleIntrinsics;
}

/// Converts the LLVM intrinsic to an MLIR LLVM dialect operation if a
/// conversion exits. Returns failure otherwise.
static LogicalResult convertIntrinsicImpl(OpBuilder &odsBuilder,
                                          toolchain::CallInst *inst,
                                          LLVM::ModuleImport &moduleImport) {
  toolchain::Intrinsic::ID intrinsicID = inst->getIntrinsicID();

  // Check if the intrinsic is convertible to an MLIR dialect counterpart and
  // copy the arguments to an an LLVM operands array reference for conversion.
  if (isConvertibleIntrinsic(intrinsicID)) {
    SmallVector<toolchain::Value *> args(inst->args());
    ArrayRef<toolchain::Value *> llvmOperands(args);

    SmallVector<toolchain::OperandBundleUse> llvmOpBundles;
    llvmOpBundles.reserve(inst->getNumOperandBundles());
    for (unsigned i = 0; i < inst->getNumOperandBundles(); ++i)
      llvmOpBundles.push_back(inst->getOperandBundleAt(i));

#include "mlir/Dialect/LLVMIR/LLVMIntrinsicFromLLVMIRConversions.inc"
  }

  return failure();
}

/// Returns the list of LLVM IR metadata kinds that are convertible to MLIR LLVM
/// dialect attributes.
static SmallVector<unsigned>
getSupportedMetadataImpl(toolchain::LLVMContext &llvmContext) {
  SmallVector<unsigned> convertibleMetadata = {
      toolchain::LLVMContext::MD_prof,
      toolchain::LLVMContext::MD_tbaa,
      toolchain::LLVMContext::MD_access_group,
      toolchain::LLVMContext::MD_loop,
      toolchain::LLVMContext::MD_noalias,
      toolchain::LLVMContext::MD_alias_scope,
      toolchain::LLVMContext::MD_dereferenceable,
      toolchain::LLVMContext::MD_dereferenceable_or_null,
      toolchain::LLVMContext::MD_mmra,
      llvmContext.getMDKindID(vecTypeHintMDName),
      llvmContext.getMDKindID(workGroupSizeHintMDName),
      llvmContext.getMDKindID(reqdWorkGroupSizeMDName),
      llvmContext.getMDKindID(intelReqdSubGroupSizeMDName)};
  return convertibleMetadata;
}

/// Converts the given profiling metadata `node` to an MLIR profiling attribute
/// and attaches it to the imported operation if the translation succeeds.
/// Returns failure otherwise.
static LogicalResult setProfilingAttr(OpBuilder &builder, toolchain::MDNode *node,
                                      Operation *op,
                                      LLVM::ModuleImport &moduleImport) {
  // Return failure for empty metadata nodes since there is nothing to import.
  if (!node->getNumOperands())
    return failure();

  auto *name = dyn_cast<toolchain::MDString>(node->getOperand(0));
  if (!name)
    return failure();

  // Handle function entry count metadata.
  if (name->getString() == toolchain::MDProfLabels::FunctionEntryCount) {

    // TODO support function entry count metadata with GUID fields.
    if (node->getNumOperands() != 2)
      return failure();

    toolchain::ConstantInt *entryCount =
        toolchain::mdconst::dyn_extract<toolchain::ConstantInt>(node->getOperand(1));
    if (!entryCount)
      return failure();
    if (auto funcOp = dyn_cast<LLVMFuncOp>(op)) {
      funcOp.setFunctionEntryCount(entryCount->getZExtValue());
      return success();
    }
    return op->emitWarning()
           << "expected function_entry_count to be attached to a function";
  }

  if (name->getString() != toolchain::MDProfLabels::BranchWeights)
    return failure();
  // The branch_weights metadata must have at least 2 operands.
  if (node->getNumOperands() < 2)
    return failure();

  ArrayRef<toolchain::MDOperand> branchWeightOperands =
      node->operands().drop_front();
  if (auto *mdString = dyn_cast<toolchain::MDString>(node->getOperand(1))) {
    if (mdString->getString() != toolchain::MDProfLabels::ExpectedBranchWeights)
      return failure();
    // The MLIR WeightedBranchOpInterface does not support the
    // ExpectedBranchWeights field, so it is dropped.
    branchWeightOperands = branchWeightOperands.drop_front();
  }

  // Handle branch weights metadata.
  SmallVector<int32_t> branchWeights;
  branchWeights.reserve(branchWeightOperands.size());
  for (const toolchain::MDOperand &operand : branchWeightOperands) {
    toolchain::ConstantInt *branchWeight =
        toolchain::mdconst::dyn_extract<toolchain::ConstantInt>(operand);
    if (!branchWeight)
      return failure();
    branchWeights.push_back(branchWeight->getZExtValue());
  }

  if (auto iface = dyn_cast<WeightedBranchOpInterface>(op)) {
    // LLVM allows attaching a single weight to call instructions.
    // This is used for carrying the execution count information
    // in PGO modes. MLIR WeightedBranchOpInterface does not allow this,
    // so we drop the metadata in this case.
    // LLVM should probably use the VP form of MD_prof metadata
    // for such cases.
    if (op->getNumSuccessors() != 0)
      iface.setWeights(branchWeights);
    return success();
  }
  return failure();
}

/// Searches for the attribute that maps to the given TBAA metadata `node` and
/// attaches it to the imported operation if the lookup succeeds. Returns
/// failure otherwise.
static LogicalResult setTBAAAttr(const toolchain::MDNode *node, Operation *op,
                                 LLVM::ModuleImport &moduleImport) {
  Attribute tbaaTagSym = moduleImport.lookupTBAAAttr(node);
  if (!tbaaTagSym)
    return failure();

  auto iface = dyn_cast<AliasAnalysisOpInterface>(op);
  if (!iface)
    return failure();

  iface.setTBAATags(ArrayAttr::get(iface.getContext(), tbaaTagSym));
  return success();
}

/// Looks up all the access group attributes that map to the access group nodes
/// starting from the access group metadata `node`, and attaches all of them to
/// the imported operation if the lookups succeed. Returns failure otherwise.
static LogicalResult setAccessGroupsAttr(const toolchain::MDNode *node,
                                         Operation *op,
                                         LLVM::ModuleImport &moduleImport) {
  FailureOr<SmallVector<AccessGroupAttr>> accessGroups =
      moduleImport.lookupAccessGroupAttrs(node);
  if (failed(accessGroups))
    return failure();

  auto iface = dyn_cast<AccessGroupOpInterface>(op);
  if (!iface)
    return failure();

  iface.setAccessGroups(ArrayAttr::get(
      iface.getContext(), toolchain::to_vector_of<Attribute>(*accessGroups)));
  return success();
}

/// Converts the given dereferenceable metadata node to a dereferenceable
/// attribute, and attaches it to the imported operation if the translation
/// succeeds. Returns failure if the LLVM IR metadata node is ill-formed.
static LogicalResult setDereferenceableAttr(const toolchain::MDNode *node,
                                            unsigned kindID, Operation *op,
                                            LLVM::ModuleImport &moduleImport) {
  auto dereferenceable =
      moduleImport.translateDereferenceableAttr(node, kindID);
  if (failed(dereferenceable))
    return failure();

  auto iface = dyn_cast<DereferenceableOpInterface>(op);
  if (!iface)
    return failure();

  iface.setDereferenceable(*dereferenceable);
  return success();
}

/// Convert the given MMRA metadata (either an MMRA tag or an array of them)
/// into corresponding MLIR attributes and set them on the given operation as a
/// discardable `toolchain.mmra` attribute.
static LogicalResult setMmraAttr(toolchain::MDNode *node, Operation *op,
                                 LLVM::ModuleImport &moduleImport) {
  if (!node)
    return success();

  // We don't use the LLVM wrappers here becasue we care about the order
  // of the metadata for deterministic roundtripping.
  MLIRContext *ctx = op->getContext();
  auto toAttribute = [&](toolchain::MDNode *tag) -> Attribute {
    return LLVM::MMRATagAttr::get(
        ctx, cast<toolchain::MDString>(tag->getOperand(0))->getString(),
        cast<toolchain::MDString>(tag->getOperand(1))->getString());
  };
  Attribute mlirMmra;
  if (toolchain::MMRAMetadata::isTagMD(node)) {
    mlirMmra = toAttribute(node);
  } else {
    SmallVector<Attribute> tags;
    for (const toolchain::MDOperand &operand : node->operands()) {
      auto *tagNode = dyn_cast<toolchain::MDNode>(operand.get());
      if (!tagNode || !toolchain::MMRAMetadata::isTagMD(tagNode))
        return failure();
      tags.push_back(toAttribute(tagNode));
    }
    mlirMmra = ArrayAttr::get(ctx, tags);
  }
  op->setAttr(LLVMDialect::getMmraAttrName(), mlirMmra);
  return success();
}

/// Converts the given loop metadata node to an MLIR loop annotation attribute
/// and attaches it to the imported operation if the translation succeeds.
/// Returns failure otherwise.
static LogicalResult setLoopAttr(const toolchain::MDNode *node, Operation *op,
                                 LLVM::ModuleImport &moduleImport) {
  LoopAnnotationAttr attr =
      moduleImport.translateLoopAnnotationAttr(node, op->getLoc());
  if (!attr)
    return failure();

  return TypeSwitch<Operation *, LogicalResult>(op)
      .Case<LLVM::BrOp, LLVM::CondBrOp>([&](auto branchOp) {
        branchOp.setLoopAnnotationAttr(attr);
        return success();
      })
      .Default(failure());
}

/// Looks up all the alias scope attributes that map to the alias scope nodes
/// starting from the alias scope metadata `node`, and attaches all of them to
/// the imported operation if the lookups succeed. Returns failure otherwise.
static LogicalResult setAliasScopesAttr(const toolchain::MDNode *node, Operation *op,
                                        LLVM::ModuleImport &moduleImport) {
  FailureOr<SmallVector<AliasScopeAttr>> aliasScopes =
      moduleImport.lookupAliasScopeAttrs(node);
  if (failed(aliasScopes))
    return failure();

  auto iface = dyn_cast<AliasAnalysisOpInterface>(op);
  if (!iface)
    return failure();

  iface.setAliasScopes(ArrayAttr::get(
      iface.getContext(), toolchain::to_vector_of<Attribute>(*aliasScopes)));
  return success();
}

/// Looks up all the alias scope attributes that map to the alias scope nodes
/// starting from the noalias metadata `node`, and attaches all of them to the
/// imported operation if the lookups succeed. Returns failure otherwise.
static LogicalResult setNoaliasScopesAttr(const toolchain::MDNode *node,
                                          Operation *op,
                                          LLVM::ModuleImport &moduleImport) {
  FailureOr<SmallVector<AliasScopeAttr>> noAliasScopes =
      moduleImport.lookupAliasScopeAttrs(node);
  if (failed(noAliasScopes))
    return failure();

  auto iface = dyn_cast<AliasAnalysisOpInterface>(op);
  if (!iface)
    return failure();

  iface.setNoAliasScopes(ArrayAttr::get(
      iface.getContext(), toolchain::to_vector_of<Attribute>(*noAliasScopes)));
  return success();
}

/// Extracts an integer from the provided metadata `md` if possible. Returns
/// nullopt otherwise.
static std::optional<int32_t> parseIntegerMD(toolchain::Metadata *md) {
  auto *constant = dyn_cast_if_present<toolchain::ConstantAsMetadata>(md);
  if (!constant)
    return {};

  auto *intConstant = dyn_cast<toolchain::ConstantInt>(constant->getValue());
  if (!intConstant)
    return {};

  return intConstant->getValue().getSExtValue();
}

/// Converts the provided metadata node `node` to an LLVM dialect
/// VecTypeHintAttr if possible.
static VecTypeHintAttr convertVecTypeHint(Builder builder, toolchain::MDNode *node,
                                          ModuleImport &moduleImport) {
  if (!node || node->getNumOperands() != 2)
    return {};

  auto *hintMD = dyn_cast<toolchain::ValueAsMetadata>(node->getOperand(0).get());
  if (!hintMD)
    return {};
  TypeAttr hint = TypeAttr::get(moduleImport.convertType(hintMD->getType()));

  std::optional<int32_t> optIsSigned =
      parseIntegerMD(node->getOperand(1).get());
  if (!optIsSigned)
    return {};
  bool isSigned = *optIsSigned != 0;

  return builder.getAttr<VecTypeHintAttr>(hint, isSigned);
}

/// Converts the provided metadata node `node` to an MLIR DenseI32ArrayAttr if
/// possible.
static DenseI32ArrayAttr convertDenseI32Array(Builder builder,
                                              toolchain::MDNode *node) {
  if (!node)
    return {};
  SmallVector<int32_t> vals;
  for (const toolchain::MDOperand &op : node->operands()) {
    std::optional<int32_t> mdValue = parseIntegerMD(op.get());
    if (!mdValue)
      return {};
    vals.push_back(*mdValue);
  }
  return builder.getDenseI32ArrayAttr(vals);
}

/// Convert an `MDNode` to an MLIR `IntegerAttr` if possible.
static IntegerAttr convertIntegerMD(Builder builder, toolchain::MDNode *node) {
  if (!node || node->getNumOperands() != 1)
    return {};
  std::optional<int32_t> val = parseIntegerMD(node->getOperand(0));
  if (!val)
    return {};
  return builder.getI32IntegerAttr(*val);
}

static LogicalResult setVecTypeHintAttr(Builder &builder, toolchain::MDNode *node,
                                        Operation *op,
                                        LLVM::ModuleImport &moduleImport) {
  auto funcOp = dyn_cast<LLVM::LLVMFuncOp>(op);
  if (!funcOp)
    return failure();

  VecTypeHintAttr attr = convertVecTypeHint(builder, node, moduleImport);
  if (!attr)
    return failure();

  funcOp.setVecTypeHintAttr(attr);
  return success();
}

static LogicalResult
setWorkGroupSizeHintAttr(Builder &builder, toolchain::MDNode *node, Operation *op) {
  auto funcOp = dyn_cast<LLVM::LLVMFuncOp>(op);
  if (!funcOp)
    return failure();

  DenseI32ArrayAttr attr = convertDenseI32Array(builder, node);
  if (!attr)
    return failure();

  funcOp.setWorkGroupSizeHintAttr(attr);
  return success();
}

static LogicalResult
setReqdWorkGroupSizeAttr(Builder &builder, toolchain::MDNode *node, Operation *op) {
  auto funcOp = dyn_cast<LLVM::LLVMFuncOp>(op);
  if (!funcOp)
    return failure();

  DenseI32ArrayAttr attr = convertDenseI32Array(builder, node);
  if (!attr)
    return failure();

  funcOp.setReqdWorkGroupSizeAttr(attr);
  return success();
}

/// Converts the given intel required subgroup size metadata node to an MLIR
/// attribute and attaches it to the imported operation if the translation
/// succeeds. Returns failure otherwise.
static LogicalResult setIntelReqdSubGroupSizeAttr(Builder &builder,
                                                  toolchain::MDNode *node,
                                                  Operation *op) {
  auto funcOp = dyn_cast<LLVM::LLVMFuncOp>(op);
  if (!funcOp)
    return failure();

  IntegerAttr attr = convertIntegerMD(builder, node);
  if (!attr)
    return failure();

  funcOp.setIntelReqdSubGroupSizeAttr(attr);
  return success();
}

namespace {

/// Implementation of the dialect interface that converts operations belonging
/// to the LLVM dialect to LLVM IR.
class LLVMDialectLLVMIRImportInterface : public LLVMImportDialectInterface {
public:
  using LLVMImportDialectInterface::LLVMImportDialectInterface;

  /// Converts the LLVM intrinsic to an MLIR LLVM dialect operation if a
  /// conversion exits. Returns failure otherwise.
  LogicalResult convertIntrinsic(OpBuilder &builder, toolchain::CallInst *inst,
                                 LLVM::ModuleImport &moduleImport) const final {
    return convertIntrinsicImpl(builder, inst, moduleImport);
  }

  /// Attaches the given LLVM metadata to the imported operation if a conversion
  /// to an LLVM dialect attribute exists and succeeds. Returns failure
  /// otherwise.
  LogicalResult setMetadataAttrs(OpBuilder &builder, unsigned kind,
                                 toolchain::MDNode *node, Operation *op,
                                 LLVM::ModuleImport &moduleImport) const final {
    // Call metadata specific handlers.
    if (kind == toolchain::LLVMContext::MD_prof)
      return setProfilingAttr(builder, node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_tbaa)
      return setTBAAAttr(node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_access_group)
      return setAccessGroupsAttr(node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_loop)
      return setLoopAttr(node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_alias_scope)
      return setAliasScopesAttr(node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_noalias)
      return setNoaliasScopesAttr(node, op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_dereferenceable)
      return setDereferenceableAttr(node, toolchain::LLVMContext::MD_dereferenceable,
                                    op, moduleImport);
    if (kind == toolchain::LLVMContext::MD_dereferenceable_or_null)
      return setDereferenceableAttr(
          node, toolchain::LLVMContext::MD_dereferenceable_or_null, op,
          moduleImport);
    if (kind == toolchain::LLVMContext::MD_mmra)
      return setMmraAttr(node, op, moduleImport);
    toolchain::LLVMContext &context = node->getContext();
    if (kind == context.getMDKindID(vecTypeHintMDName))
      return setVecTypeHintAttr(builder, node, op, moduleImport);
    if (kind == context.getMDKindID(workGroupSizeHintMDName))
      return setWorkGroupSizeHintAttr(builder, node, op);
    if (kind == context.getMDKindID(reqdWorkGroupSizeMDName))
      return setReqdWorkGroupSizeAttr(builder, node, op);
    if (kind == context.getMDKindID(intelReqdSubGroupSizeMDName))
      return setIntelReqdSubGroupSizeAttr(builder, node, op);

    // A handler for a supported metadata kind is missing.
    llvm_unreachable("unknown metadata type");
  }

  /// Returns the list of LLVM IR intrinsic identifiers that are convertible to
  /// MLIR LLVM dialect intrinsics.
  ArrayRef<unsigned> getSupportedIntrinsics() const final {
    return getSupportedIntrinsicsImpl();
  }

  /// Returns the list of LLVM IR metadata kinds that are convertible to MLIR
  /// LLVM dialect attributes.
  SmallVector<unsigned>
  getSupportedMetadata(toolchain::LLVMContext &llvmContext) const final {
    return getSupportedMetadataImpl(llvmContext);
  }
};
} // namespace

void mlir::registerLLVMDialectImport(DialectRegistry &registry) {
  registry.insert<LLVM::LLVMDialect>();
  registry.addExtension(+[](MLIRContext *ctx, LLVM::LLVMDialect *dialect) {
    dialect->addInterfaces<LLVMDialectLLVMIRImportInterface>();
  });
}

void mlir::registerLLVMDialectImport(MLIRContext &context) {
  DialectRegistry registry;
  registerLLVMDialectImport(registry);
  context.appendDialectRegistry(registry);
}
