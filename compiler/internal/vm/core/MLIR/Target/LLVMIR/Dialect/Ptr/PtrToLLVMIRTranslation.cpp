//===- PtrToLLVMIRTranslation.cpp - Translate `ptr` to LLVM IR ------------===//
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
// This file implements a translation between the MLIR `ptr` dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/Ptr/PtrToLLVMIRTranslation.h"
#include "mlir/Dialect/Ptr/IR/PtrOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"

using namespace mlir;
using namespace mlir::ptr;

namespace {

/// Converts ptr::AtomicOrdering to toolchain::AtomicOrdering
static toolchain::AtomicOrdering
translateAtomicOrdering(ptr::AtomicOrdering ordering) {
  switch (ordering) {
  case ptr::AtomicOrdering::not_atomic:
    return toolchain::AtomicOrdering::NotAtomic;
  case ptr::AtomicOrdering::unordered:
    return toolchain::AtomicOrdering::Unordered;
  case ptr::AtomicOrdering::monotonic:
    return toolchain::AtomicOrdering::Monotonic;
  case ptr::AtomicOrdering::acquire:
    return toolchain::AtomicOrdering::Acquire;
  case ptr::AtomicOrdering::release:
    return toolchain::AtomicOrdering::Release;
  case ptr::AtomicOrdering::acq_rel:
    return toolchain::AtomicOrdering::AcquireRelease;
  case ptr::AtomicOrdering::seq_cst:
    return toolchain::AtomicOrdering::SequentiallyConsistent;
  }
  llvm_unreachable("Unknown atomic ordering");
}

/// Translate ptr.ptr_add operation to LLVM IR.
static LogicalResult
translatePtrAddOp(PtrAddOp ptrAddOp, toolchain::IRBuilderBase &builder,
                  LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *basePtr = moduleTranslation.lookupValue(ptrAddOp.getBase());
  toolchain::Value *offset = moduleTranslation.lookupValue(ptrAddOp.getOffset());

  if (!basePtr || !offset)
    return ptrAddOp.emitError("Failed to lookup operands");

  // Create the GEP flags
  toolchain::GEPNoWrapFlags gepFlags;
  switch (ptrAddOp.getFlags()) {
  case ptr::PtrAddFlags::none:
    break;
  case ptr::PtrAddFlags::nusw:
    gepFlags = toolchain::GEPNoWrapFlags::noUnsignedSignedWrap();
    break;
  case ptr::PtrAddFlags::nuw:
    gepFlags = toolchain::GEPNoWrapFlags::noUnsignedWrap();
    break;
  case ptr::PtrAddFlags::inbounds:
    gepFlags = toolchain::GEPNoWrapFlags::inBounds();
    break;
  }

  // Create GEP instruction for pointer arithmetic
  toolchain::Value *gep =
      builder.CreateGEP(builder.getInt8Ty(), basePtr, {offset}, "", gepFlags);

  moduleTranslation.mapValue(ptrAddOp.getResult(), gep);
  return success();
}

/// Translate ptr.load operation to LLVM IR.
static LogicalResult
translateLoadOp(LoadOp loadOp, toolchain::IRBuilderBase &builder,
                LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *ptr = moduleTranslation.lookupValue(loadOp.getPtr());
  if (!ptr)
    return loadOp.emitError("Failed to lookup pointer operand");

  // Translate result type to LLVM type
  toolchain::Type *resultType =
      moduleTranslation.convertType(loadOp.getValue().getType());
  if (!resultType)
    return loadOp.emitError("Failed to translate result type");

  // Create the load instruction.
  toolchain::MaybeAlign alignment(loadOp.getAlignment().value_or(0));
  toolchain::LoadInst *loadInst = builder.CreateAlignedLoad(
      resultType, ptr, alignment, loadOp.getVolatile_());

  // Set op flags and metadata.
  loadInst->setAtomic(translateAtomicOrdering(loadOp.getOrdering()));
  // Set sync scope if specified
  if (loadOp.getSyncscope().has_value()) {
    toolchain::LLVMContext &ctx = builder.getContext();
    toolchain::SyncScope::ID syncScope =
        ctx.getOrInsertSyncScopeID(loadOp.getSyncscope().value());
    loadInst->setSyncScopeID(syncScope);
  }

  // Set metadata for nontemporal, invariant, and invariant_group
  if (loadOp.getNontemporal()) {
    toolchain::MDNode *nontemporalMD =
        toolchain::MDNode::get(builder.getContext(),
                          toolchain::ConstantAsMetadata::get(builder.getInt32(1)));
    loadInst->setMetadata(toolchain::LLVMContext::MD_nontemporal, nontemporalMD);
  }

  if (loadOp.getInvariant()) {
    toolchain::MDNode *invariantMD = toolchain::MDNode::get(builder.getContext(), {});
    loadInst->setMetadata(toolchain::LLVMContext::MD_invariant_load, invariantMD);
  }

  if (loadOp.getInvariantGroup()) {
    toolchain::MDNode *invariantGroupMD =
        toolchain::MDNode::get(builder.getContext(), {});
    loadInst->setMetadata(toolchain::LLVMContext::MD_invariant_group,
                          invariantGroupMD);
  }

  moduleTranslation.mapValue(loadOp.getResult(), loadInst);
  return success();
}

/// Translate ptr.store operation to LLVM IR.
static LogicalResult
translateStoreOp(StoreOp storeOp, toolchain::IRBuilderBase &builder,
                 LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *value = moduleTranslation.lookupValue(storeOp.getValue());
  toolchain::Value *ptr = moduleTranslation.lookupValue(storeOp.getPtr());

  if (!value || !ptr)
    return storeOp.emitError("Failed to lookup operands");

  // Create the store instruction.
  toolchain::MaybeAlign alignment(storeOp.getAlignment().value_or(0));
  toolchain::StoreInst *storeInst =
      builder.CreateAlignedStore(value, ptr, alignment, storeOp.getVolatile_());

  // Set op flags and metadata.
  storeInst->setAtomic(translateAtomicOrdering(storeOp.getOrdering()));
  // Set sync scope if specified
  if (storeOp.getSyncscope().has_value()) {
    toolchain::LLVMContext &ctx = builder.getContext();
    toolchain::SyncScope::ID syncScope =
        ctx.getOrInsertSyncScopeID(storeOp.getSyncscope().value());
    storeInst->setSyncScopeID(syncScope);
  }

  // Set metadata for nontemporal and invariant_group
  if (storeOp.getNontemporal()) {
    toolchain::MDNode *nontemporalMD =
        toolchain::MDNode::get(builder.getContext(),
                          toolchain::ConstantAsMetadata::get(builder.getInt32(1)));
    storeInst->setMetadata(toolchain::LLVMContext::MD_nontemporal, nontemporalMD);
  }

  if (storeOp.getInvariantGroup()) {
    toolchain::MDNode *invariantGroupMD =
        toolchain::MDNode::get(builder.getContext(), {});
    storeInst->setMetadata(toolchain::LLVMContext::MD_invariant_group,
                           invariantGroupMD);
  }

  return success();
}

/// Translate ptr.type_offset operation to LLVM IR.
static LogicalResult
translateTypeOffsetOp(TypeOffsetOp typeOffsetOp, toolchain::IRBuilderBase &builder,
                      LLVM::ModuleTranslation &moduleTranslation) {
  // Translate the element type to LLVM type
  toolchain::Type *elementType =
      moduleTranslation.convertType(typeOffsetOp.getElementType());
  if (!elementType)
    return typeOffsetOp.emitError("Failed to translate the element type");

  // Translate result type
  toolchain::Type *resultType =
      moduleTranslation.convertType(typeOffsetOp.getResult().getType());
  if (!resultType)
    return typeOffsetOp.emitError("Failed to translate the result type");

  // Use GEP with null pointer to compute type size/offset.
  toolchain::Value *nullPtr = toolchain::Constant::getNullValue(builder.getPtrTy(0));
  toolchain::Value *offsetPtr =
      builder.CreateGEP(elementType, nullPtr, {builder.getInt32(1)});
  toolchain::Value *offset = builder.CreatePtrToInt(offsetPtr, resultType);

  moduleTranslation.mapValue(typeOffsetOp.getResult(), offset);
  return success();
}

/// Translate ptr.gather operation to LLVM IR.
static LogicalResult
translateGatherOp(GatherOp gatherOp, toolchain::IRBuilderBase &builder,
                  LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *ptrs = moduleTranslation.lookupValue(gatherOp.getPtrs());
  toolchain::Value *mask = moduleTranslation.lookupValue(gatherOp.getMask());
  toolchain::Value *passthrough =
      moduleTranslation.lookupValue(gatherOp.getPassthrough());

  if (!ptrs || !mask || !passthrough)
    return gatherOp.emitError("Failed to lookup operands");

  // Translate result type to LLVM type.
  toolchain::Type *resultType =
      moduleTranslation.convertType(gatherOp.getResult().getType());
  if (!resultType)
    return gatherOp.emitError("Failed to translate result type");

  // Get the alignment.
  toolchain::MaybeAlign alignment(gatherOp.getAlignment().value_or(0));

  // Create the masked gather intrinsic call.
  toolchain::Value *result = builder.CreateMaskedGather(
      resultType, ptrs, alignment.valueOrOne(), mask, passthrough);

  moduleTranslation.mapValue(gatherOp.getResult(), result);
  return success();
}

/// Translate ptr.masked_load operation to LLVM IR.
static LogicalResult
translateMaskedLoadOp(MaskedLoadOp maskedLoadOp, toolchain::IRBuilderBase &builder,
                      LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *ptr = moduleTranslation.lookupValue(maskedLoadOp.getPtr());
  toolchain::Value *mask = moduleTranslation.lookupValue(maskedLoadOp.getMask());
  toolchain::Value *passthrough =
      moduleTranslation.lookupValue(maskedLoadOp.getPassthrough());

  if (!ptr || !mask || !passthrough)
    return maskedLoadOp.emitError("Failed to lookup operands");

  // Translate result type to LLVM type.
  toolchain::Type *resultType =
      moduleTranslation.convertType(maskedLoadOp.getResult().getType());
  if (!resultType)
    return maskedLoadOp.emitError("Failed to translate result type");

  // Get the alignment.
  toolchain::MaybeAlign alignment(maskedLoadOp.getAlignment().value_or(0));

  // Create the masked load intrinsic call.
  toolchain::Value *result = builder.CreateMaskedLoad(
      resultType, ptr, alignment.valueOrOne(), mask, passthrough);

  moduleTranslation.mapValue(maskedLoadOp.getResult(), result);
  return success();
}

/// Translate ptr.masked_store operation to LLVM IR.
static LogicalResult
translateMaskedStoreOp(MaskedStoreOp maskedStoreOp,
                       toolchain::IRBuilderBase &builder,
                       LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *value = moduleTranslation.lookupValue(maskedStoreOp.getValue());
  toolchain::Value *ptr = moduleTranslation.lookupValue(maskedStoreOp.getPtr());
  toolchain::Value *mask = moduleTranslation.lookupValue(maskedStoreOp.getMask());

  if (!value || !ptr || !mask)
    return maskedStoreOp.emitError("Failed to lookup operands");

  // Get the alignment.
  toolchain::MaybeAlign alignment(maskedStoreOp.getAlignment().value_or(0));

  // Create the masked store intrinsic call.
  builder.CreateMaskedStore(value, ptr, alignment.valueOrOne(), mask);
  return success();
}

/// Translate ptr.scatter operation to LLVM IR.
static LogicalResult
translateScatterOp(ScatterOp scatterOp, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *value = moduleTranslation.lookupValue(scatterOp.getValue());
  toolchain::Value *ptrs = moduleTranslation.lookupValue(scatterOp.getPtrs());
  toolchain::Value *mask = moduleTranslation.lookupValue(scatterOp.getMask());

  if (!value || !ptrs || !mask)
    return scatterOp.emitError("Failed to lookup operands");

  // Get the alignment.
  toolchain::MaybeAlign alignment(scatterOp.getAlignment().value_or(0));

  // Create the masked scatter intrinsic call.
  builder.CreateMaskedScatter(value, ptrs, alignment.valueOrOne(), mask);
  return success();
}

/// Translate ptr.constant operation to LLVM IR.
static LogicalResult
translateConstantOp(ConstantOp constantOp, toolchain::IRBuilderBase &builder,
                    LLVM::ModuleTranslation &moduleTranslation) {
  // Translate result type to LLVM type
  toolchain::PointerType *resultType = dyn_cast_or_null<toolchain::PointerType>(
      moduleTranslation.convertType(constantOp.getResult().getType()));
  if (!resultType)
    return constantOp.emitError("Expected a valid pointer type");

  toolchain::Value *result = nullptr;

  TypedAttr value = constantOp.getValue();
  if (auto nullAttr = dyn_cast<ptr::NullAttr>(value)) {
    // Create a null pointer constant
    result = toolchain::ConstantPointerNull::get(resultType);
  } else if (auto addressAttr = dyn_cast<ptr::AddressAttr>(value)) {
    // Create an integer constant and translate it to pointer
    toolchain::APInt addressValue = addressAttr.getValue();

    // Determine the integer type width based on the target's pointer size
    toolchain::DataLayout dataLayout =
        moduleTranslation.getLLVMModule()->getDataLayout();
    unsigned pointerSizeInBits =
        dataLayout.getPointerSizeInBits(resultType->getAddressSpace());

    // Extend or truncate the address value to match pointer size if needed
    if (addressValue.getBitWidth() != pointerSizeInBits) {
      if (addressValue.getBitWidth() > pointerSizeInBits) {
        constantOp.emitWarning()
            << "Truncating address value to fit pointer size";
      }
      addressValue = addressValue.getBitWidth() < pointerSizeInBits
                         ? addressValue.zext(pointerSizeInBits)
                         : addressValue.trunc(pointerSizeInBits);
    }

    // Create integer constant and translate to pointer
    toolchain::Type *intType = builder.getIntNTy(pointerSizeInBits);
    toolchain::Value *intValue = toolchain::ConstantInt::get(intType, addressValue);
    result = builder.CreateIntToPtr(intValue, resultType);
  } else {
    return constantOp.emitError("Unsupported constant attribute type");
  }

  moduleTranslation.mapValue(constantOp.getResult(), result);
  return success();
}

/// Translate ptr.ptr_diff operation operation to LLVM IR.
static LogicalResult
translatePtrDiffOp(PtrDiffOp ptrDiffOp, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Value *lhs = moduleTranslation.lookupValue(ptrDiffOp.getLhs());
  toolchain::Value *rhs = moduleTranslation.lookupValue(ptrDiffOp.getRhs());

  if (!lhs || !rhs)
    return ptrDiffOp.emitError("Failed to lookup operands");

  // Translate result type to LLVM type
  toolchain::Type *resultType =
      moduleTranslation.convertType(ptrDiffOp.getResult().getType());
  if (!resultType)
    return ptrDiffOp.emitError("Failed to translate result type");

  PtrDiffFlags flags = ptrDiffOp.getFlags();

  // Convert both pointers to integers using ptrtoaddr, and compute the
  // difference: lhs - rhs
  toolchain::Value *llLhs = builder.CreatePtrToAddr(lhs);
  toolchain::Value *llRhs = builder.CreatePtrToAddr(rhs);
  toolchain::Value *result = builder.CreateSub(
      llLhs, llRhs, /*Name=*/"",
      /*HasNUW=*/(flags & PtrDiffFlags::nuw) == PtrDiffFlags::nuw,
      /*HasNSW=*/(flags & PtrDiffFlags::nsw) == PtrDiffFlags::nsw);

  // Convert the difference to the expected result type by truncating or
  // extending.
  if (result->getType() != resultType)
    result = builder.CreateIntCast(result, resultType, /*isSigned=*/true);

  moduleTranslation.mapValue(ptrDiffOp.getResult(), result);
  return success();
}

/// Implementation of the dialect interface that translates operations belonging
/// to the `ptr` dialect to LLVM IR.
class PtrDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {

    return toolchain::TypeSwitch<Operation *, LogicalResult>(op)
        .Case([&](ConstantOp constantOp) {
          return translateConstantOp(constantOp, builder, moduleTranslation);
        })
        .Case([&](PtrAddOp ptrAddOp) {
          return translatePtrAddOp(ptrAddOp, builder, moduleTranslation);
        })
        .Case([&](PtrDiffOp ptrDiffOp) {
          return translatePtrDiffOp(ptrDiffOp, builder, moduleTranslation);
        })
        .Case([&](LoadOp loadOp) {
          return translateLoadOp(loadOp, builder, moduleTranslation);
        })
        .Case([&](StoreOp storeOp) {
          return translateStoreOp(storeOp, builder, moduleTranslation);
        })
        .Case([&](TypeOffsetOp typeOffsetOp) {
          return translateTypeOffsetOp(typeOffsetOp, builder,
                                       moduleTranslation);
        })
        .Case<GatherOp>([&](GatherOp gatherOp) {
          return translateGatherOp(gatherOp, builder, moduleTranslation);
        })
        .Case<MaskedLoadOp>([&](MaskedLoadOp maskedLoadOp) {
          return translateMaskedLoadOp(maskedLoadOp, builder,
                                       moduleTranslation);
        })
        .Case<MaskedStoreOp>([&](MaskedStoreOp maskedStoreOp) {
          return translateMaskedStoreOp(maskedStoreOp, builder,
                                        moduleTranslation);
        })
        .Case<ScatterOp>([&](ScatterOp scatterOp) {
          return translateScatterOp(scatterOp, builder, moduleTranslation);
        })
        .Default([&](Operation *op) {
          return op->emitError("Translation for operation '")
                 << op->getName() << "' is not implemented.";
        });
  }

  /// Attaches module-level metadata for functions marked as kernels.
  LogicalResult
  amendOperation(Operation *op, ArrayRef<toolchain::Instruction *> instructions,
                 NamedAttribute attribute,
                 LLVM::ModuleTranslation &moduleTranslation) const final {
    // No special amendments needed for ptr dialect operations
    return success();
  }
};
} // namespace

void mlir::registerPtrDialectTranslation(DialectRegistry &registry) {
  registry.insert<ptr::PtrDialect>();
  registry.addExtension(+[](MLIRContext *ctx, ptr::PtrDialect *dialect) {
    dialect->addInterfaces<PtrDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerPtrDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerPtrDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
