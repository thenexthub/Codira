//===--- GenHeap.cpp - Layout of heap objects and their metadata ----------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements routines for arbitrary Codira-native heap objects,
//  such as layout and reference-counting.
//
//===----------------------------------------------------------------------===//

#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/Path.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/GlobalVariable.h"
#include "toolchain/IR/Intrinsics.h"

#include "language/Basic/Assertions.h"
#include "language/Basic/SourceLoc.h"
#include "language/ABI/MetadataValues.h"
#include "language/AST/ASTContext.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/IRGenOptions.h"
#include "language/SIL/SILModule.h"

#include "ClassTypeInfo.h"
#include "ConstantBuilder.h"
#include "Explosion.h"
#include "GenClass.h"
#include "GenMeta.h"
#include "GenPointerAuth.h"
#include "GenProto.h"
#include "GenType.h"
#include "HeapTypeInfo.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "IndirectTypeInfo.h"
#include "MetadataRequest.h"

#include "GenHeap.h"

using namespace language;
using namespace irgen;

namespace {
#define NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Nativeness) \
  class Nativeness##Name##ReferenceTypeInfo \
      : public IndirectTypeInfo<Nativeness##Name##ReferenceTypeInfo, \
                                FixedTypeInfo> { \
    toolchain::PointerIntPair<toolchain::Type*, 1, bool> ValueTypeAndIsOptional; \
  public: \
    TypeLayoutEntry \
    *buildTypeLayoutEntry(IRGenModule &IGM, \
                          SILType T, \
                          bool useStructLayouts) const override { \
      if (!useStructLayouts) { \
        return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T); \
      } \
      return IGM.typeLayoutCache.getOrCreateScalarEntry( \
          *this, T, ScalarKind::Nativeness##Name##Reference); \
    } \
    Nativeness##Name##ReferenceTypeInfo(toolchain::Type *valueType, \
                                    toolchain::Type *type, \
                                    Size size, Alignment alignment, \
                                    SpareBitVector &&spareBits, \
                                    bool isOptional) \
      : IndirectTypeInfo(type, size, std::move(spareBits), alignment, \
                         IsNotTriviallyDestroyable, IsNotBitwiseTakable, IsCopyable, IsFixedSize, IsABIAccessible), \
        ValueTypeAndIsOptional(valueType, isOptional) {} \
    void initializeWithCopy(IRGenFunction &IGF, Address destAddr, \
                            Address srcAddr, SILType T, \
                            bool isOutlined) const override { \
      IGF.emit##Nativeness##Name##CopyInit(destAddr, srcAddr); \
    } \
    void initializeWithTake(IRGenFunction &IGF, Address destAddr, \
                            Address srcAddr, SILType T, \
                            bool isOutlined, bool zeroizeIfSensitive) const override { \
      IGF.emit##Nativeness##Name##TakeInit(destAddr, srcAddr); \
    } \
    void assignWithCopy(IRGenFunction &IGF, Address destAddr, Address srcAddr, \
                        SILType T, bool isOutlined) const override { \
      IGF.emit##Nativeness##Name##CopyAssign(destAddr, srcAddr); \
    } \
    void assignWithTake(IRGenFunction &IGF, Address destAddr, Address srcAddr, \
                        SILType T, bool isOutlined) const override { \
      IGF.emit##Nativeness##Name##TakeAssign(destAddr, srcAddr); \
    } \
    void destroy(IRGenFunction &IGF, Address addr, SILType T, \
                 bool isOutlined) const override { \
      IGF.emit##Nativeness##Name##Destroy(addr); \
    } \
    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override { \
      auto count = IGM.getReferenceStorageExtraInhabitantCount( \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
      return count - ValueTypeAndIsOptional.getInt(); \
    } \
    APInt getFixedExtraInhabitantValue(IRGenModule &IGM, \
                                       unsigned bits, \
                                       unsigned index) const override { \
      return IGM.getReferenceStorageExtraInhabitantValue(bits, \
                                      index + ValueTypeAndIsOptional.getInt(), \
                                      ReferenceOwnership::Name, \
                                      ReferenceCounting::Nativeness); \
    } \
    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src, \
                                         SILType T, bool isOutlined) \
    const override { \
      return IGF.getReferenceStorageExtraInhabitantIndex(src, \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
    void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index, \
                              Address dest, SILType T, bool isOutlined) \
    const override { \
      return IGF.storeReferenceStorageExtraInhabitant(index, dest, \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
    APInt getFixedExtraInhabitantMask(IRGenModule &IGM) const override { \
      return IGM.getReferenceStorageExtraInhabitantMask( \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
    toolchain::Type *getOptionalIntType() const { \
      return toolchain::IntegerType::get( \
          ValueTypeAndIsOptional.getPointer()->getContext(), \
          getFixedSize().getValueInBits()); \
    } \
  };
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Nativeness) \
  class Nativeness##Name##ReferenceTypeInfo \
    : public SingleScalarTypeInfo<Nativeness##Name##ReferenceTypeInfo, \
                                  LoadableTypeInfo> { \
    toolchain::PointerIntPair<toolchain::Type*, 1, bool> ValueTypeAndIsOptional; \
  public: \
    Nativeness##Name##ReferenceTypeInfo(toolchain::Type *valueType, \
                                              toolchain::Type *type, \
                                              Size size, Alignment alignment, \
                                              SpareBitVector &&spareBits, \
                                              bool isOptional) \
      : SingleScalarTypeInfo(type, size, std::move(spareBits), \
                             alignment, IsNotTriviallyDestroyable, \
                             IsCopyable, \
                             IsFixedSize, IsABIAccessible), \
        ValueTypeAndIsOptional(valueType, isOptional) {} \
    enum { IsScalarTriviallyDestroyable = false }; \
    TypeLayoutEntry \
    *buildTypeLayoutEntry(IRGenModule &IGM, \
                          SILType T, \
                          bool useStructLayouts) const override { \
      if (!useStructLayouts) { \
        return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T); \
      } \
      return IGM.typeLayoutCache.getOrCreateScalarEntry( \
          *this, T, ScalarKind::Nativeness##Name##Reference); \
    } \
    toolchain::Type *getScalarType() const { \
      return ValueTypeAndIsOptional.getPointer(); \
    } \
    Address projectScalar(IRGenFunction &IGF, Address addr) const { \
      return IGF.Builder.CreateElementBitCast(addr, getScalarType()); \
    } \
    void emitScalarRetain(IRGenFunction &IGF, toolchain::Value *value, \
                          Atomicity atomicity) const { \
      IGF.emit##Nativeness##Name##Retain(value, atomicity); \
    } \
    void emitScalarRelease(IRGenFunction &IGF, toolchain::Value *value, \
                           Atomicity atomicity) const { \
      IGF.emit##Nativeness##Name##Release(value, atomicity); \
    } \
    void emitScalarFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const { \
      IGF.emitFixLifetime(value); \
    } \
    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override { \
      auto count = IGM.getReferenceStorageExtraInhabitantCount( \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
      return count - ValueTypeAndIsOptional.getInt(); \
    } \
    APInt getFixedExtraInhabitantValue(IRGenModule &IGM, \
                                       unsigned bits, \
                                       unsigned index) const override { \
      return IGM.getReferenceStorageExtraInhabitantValue(bits, \
                                      index + ValueTypeAndIsOptional.getInt(), \
                                      ReferenceOwnership::Name, \
                                      ReferenceCounting::Nativeness); \
    } \
    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src, \
                                         SILType T, bool isOutlined) \
    const override {     \
      return IGF.getReferenceStorageExtraInhabitantIndex(src, \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
    void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index, \
                              Address dest, SILType T, bool isOutlined) \
    const override { \
      return IGF.storeReferenceStorageExtraInhabitant(index, dest, \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
    APInt getFixedExtraInhabitantMask(IRGenModule &IGM) const override { \
      return IGM.getReferenceStorageExtraInhabitantMask( \
                                               ReferenceOwnership::Name, \
                                               ReferenceCounting::Nativeness); \
    } \
  };

  // The nativeness of a reference storage type is a policy decision.
  // Please also see the related ALWAYS_NATIVE and SOMETIMES_UNKNOWN macros
  // later in this file that expand to the following boilerplate:
  //   TypeConverter::create##Name##StorageType
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Weak, Native)
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Weak, Unknown)
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Unowned, Unknown)
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Unowned, Native)
#undef NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER
#undef ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER

#define UNCHECKED_REF_STORAGE(Name, ...) \
  class Name##ReferenceTypeInfo \
      : public PODSingleScalarTypeInfo<Name##ReferenceTypeInfo, \
                                       LoadableTypeInfo> { \
    bool IsOptional; \
  public: \
    Name##ReferenceTypeInfo(toolchain::Type *type, \
                            const SpareBitVector &spareBits, \
                            Size size, Alignment alignment, bool isOptional) \
      : PODSingleScalarTypeInfo(type, size, spareBits, alignment), \
        IsOptional(isOptional) {} \
    /* Static types have the same spare bits as managed heap objects. */ \
    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override { \
      return getHeapObjectExtraInhabitantCount(IGM) - IsOptional; \
    } \
    APInt getFixedExtraInhabitantValue(IRGenModule &IGM, \
                                       unsigned bits, \
                                       unsigned index) const override { \
      return getHeapObjectFixedExtraInhabitantValue(IGM, bits, \
                                                    index + IsOptional, 0); \
    } \
    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src, \
                                         SILType T, bool isOutlined) \
    const override { \
      return getHeapObjectExtraInhabitantIndex(IGF, src); \
    } \
    void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index, \
                              Address dest, SILType T, bool isOutlined) \
    const override { \
      return storeHeapObjectExtraInhabitant(IGF, index, dest); \
    } \
  };
#include "language/AST/ReferenceStorage.def"
} // end anonymous namespace

/// Produce a constant to place in a metatype's isa field
/// corresponding to the given metadata kind.
static toolchain::ConstantInt *getMetadataKind(IRGenModule &IGM,
                                          MetadataKind kind) {
  return toolchain::ConstantInt::get(IGM.MetadataKindTy, uint32_t(kind));
}

/// Perform the layout required for a heap object.
HeapLayout::HeapLayout(IRGenModule &IGM, LayoutStrategy strategy,
                       ArrayRef<SILType> fieldTypes,
                       ArrayRef<const TypeInfo *> fieldTypeInfos,
                       toolchain::StructType *typeToFill,
                       NecessaryBindings &&bindings, unsigned bindingsIndex)
    : StructLayout(IGM, /*type=*/std::nullopt, LayoutKind::HeapObject, strategy,
                   fieldTypeInfos, typeToFill),
      ElementTypes(fieldTypes.begin(), fieldTypes.end()),
      Bindings(std::move(bindings)), BindingsIndex(bindingsIndex) {
#ifndef NDEBUG
  assert(fieldTypeInfos.size() == fieldTypes.size()
         && "type infos don't match types");
  if (!Bindings.empty()) {
    assert(fieldTypeInfos.size() >= (bindingsIndex + 1) &&
           "no field for bindings");
    auto fixedBindingsField =
        dyn_cast<FixedTypeInfo>(fieldTypeInfos[bindingsIndex]);
    assert(fixedBindingsField
           && "bindings field is not fixed size");
    assert(fixedBindingsField->getFixedSize()
               == Bindings.getBufferSize(IGM)
           && fixedBindingsField->getFixedAlignment()
               == IGM.getPointerAlignment()
           && "bindings field doesn't fit bindings");
  }
#endif
}

static toolchain::Value *calcInitOffset(language::irgen::IRGenFunction &IGF,
                                   unsigned int i,
                                   const language::irgen::HeapLayout &layout) {
  toolchain::Value *offset = nullptr;
  if (i == 0) {
    auto startOffset = layout.getHeaderSize();
    offset = toolchain::ConstantInt::get(IGF.IGM.SizeTy, startOffset.getValue());
    return offset;
  }
  auto &prevElt = layout.getElement(i - 1);
  auto prevType = layout.getElementTypes()[i - 1];
  // Start calculating offsets from the last fixed-offset field.
  Size lastFixedOffset = layout.getElement(i - 1).getByteOffsetDuringLayout();
  if (auto *fixedType = dyn_cast<FixedTypeInfo>(&prevElt.getType())) {
    // If the last fixed-offset field is also fixed-size, we can
    // statically compute the end of the fixed-offset fields.
    auto fixedEnd = lastFixedOffset + fixedType->getFixedSize();
    offset = toolchain::ConstantInt::get(IGF.IGM.SizeTy, fixedEnd.getValue());
  } else {
    // Otherwise, we need to add the dynamic size to the fixed start
    // offset.
    offset = toolchain::ConstantInt::get(IGF.IGM.SizeTy, lastFixedOffset.getValue());
    offset = IGF.Builder.CreateAdd(
        offset, prevElt.getType().getSize(IGF, prevType));
  }
  return offset;
}

HeapNonFixedOffsets::HeapNonFixedOffsets(IRGenFunction &IGF,
                                         const HeapLayout &layout) {
  if (!layout.isFixedLayout()) {
    // Calculate all the non-fixed layouts.
    // TODO: We could be lazier about this.
    toolchain::Value *offset = nullptr;
    toolchain::Value *totalAlign = toolchain::ConstantInt::get(IGF.IGM.SizeTy,
                                         layout.getAlignment().getMaskValue());
    for (unsigned i : indices(layout.getElements())) {
      auto &elt = layout.getElement(i);
      auto eltTy = layout.getElementTypes()[i];
      switch (elt.getKind()) {
      case ElementLayout::Kind::InitialNonFixedSize:
        // Factor the non-fixed-size field's alignment into the total alignment.
        totalAlign = IGF.Builder.CreateOr(totalAlign,
                                    elt.getType().getAlignmentMask(IGF, eltTy));
        TOOLCHAIN_FALLTHROUGH;
      case ElementLayout::Kind::Empty:
      case ElementLayout::Kind::EmptyTailAllocatedCType:
      case ElementLayout::Kind::Fixed:
        // Don't need to dynamically calculate this offset.
        Offsets.push_back(nullptr);
        break;
      
      case ElementLayout::Kind::NonFixed:
        // Start calculating non-fixed offsets from the end of the first fixed
        // field.
        if (!offset) {
          offset = calcInitOffset(IGF, i, layout);
        }
        
        // Round up to alignment to get the offset.
        auto alignMask = elt.getType().getAlignmentMask(IGF, eltTy);
        auto notAlignMask = IGF.Builder.CreateNot(alignMask);
        offset = IGF.Builder.CreateAdd(offset, alignMask);
        offset = IGF.Builder.CreateAnd(offset, notAlignMask);
        
        Offsets.push_back(offset);
        
        // Advance by the field's size to start the next field.
        offset = IGF.Builder.CreateAdd(offset,
                                       elt.getType().getSize(IGF, eltTy));
        totalAlign = IGF.Builder.CreateOr(totalAlign, alignMask);

        break;
      }
    }
    TotalSize = offset;
    TotalAlignMask = totalAlign;
  } else {
    TotalSize = layout.emitSize(IGF.IGM);
    TotalAlignMask = layout.emitAlignMask(IGF.IGM);
  }
}

void irgen::emitDeallocateHeapObject(IRGenFunction &IGF,
                                     toolchain::Value *object,
                                     toolchain::Value *size,
                                     toolchain::Value *alignMask) {
  // FIXME: We should call a fast deallocator for heap objects with
  // known size.
  IGF.Builder.CreateCall(IGF.IGM.getDeallocObjectFunctionPointer(),
                         {object, size, alignMask});
}

void emitDeallocateUninitializedHeapObject(IRGenFunction &IGF,
                                           toolchain::Value *object,
                                           toolchain::Value *size,
                                           toolchain::Value *alignMask) {
  IGF.Builder.CreateCall(IGF.IGM.getDeallocUninitializedObjectFunctionPointer(),
                         {object, size, alignMask});
}

void irgen::emitDeallocateClassInstance(IRGenFunction &IGF,
                                        toolchain::Value *object,
                                        toolchain::Value *size,
                                        toolchain::Value *alignMask) {
  // FIXME: We should call a fast deallocator for heap objects with
  // known size.
  IGF.Builder.CreateCall(IGF.IGM.getDeallocClassInstanceFunctionPointer(),
                         {object, size, alignMask});
}

void irgen::emitDeallocatePartialClassInstance(IRGenFunction &IGF,
                                               toolchain::Value *object,
                                               toolchain::Value *metadata,
                                               toolchain::Value *size,
                                               toolchain::Value *alignMask) {
  // FIXME: We should call a fast deallocator for heap objects with
  // known size.
  IGF.Builder.CreateCall(
      IGF.IGM.getDeallocPartialClassInstanceFunctionPointer(),
      {object, metadata, size, alignMask});
}

/// Create the destructor function for a layout.
/// TODO: give this some reasonable name and possibly linkage.
static toolchain::Function *createDtorFn(IRGenModule &IGM,
                                    const HeapLayout &layout) {
  toolchain::Function *fn =
    toolchain::Function::Create(IGM.DeallocatingDtorTy,
                           toolchain::Function::PrivateLinkage,
                           "objectdestroy", &IGM.Module);
  auto attrs = IGM.constructInitialAttributes();
  IGM.addCodiraSelfAttributes(attrs, 0);
  fn->setAttributes(attrs);
  fn->setCallingConv(IGM.CodiraCC);

  IRGenFunction IGF(IGM, fn);
  if (IGM.DebugInfo)
    IGM.DebugInfo->emitArtificialFunction(IGF, fn);

  Address structAddr = layout.emitCastTo(IGF, &*fn->arg_begin());

  // Bind necessary bindings, if we have them.
  if (layout.hasBindings()) {
    // The type metadata bindings should be at a fixed offset, so we can pass
    // None for NonFixedOffsets. If we didn't, we'd have a chicken-egg problem.
    auto bindingsAddr = layout.getElement(layout.getBindingsIndex())
                            .project(IGF, structAddr, std::nullopt);
    layout.getBindings().restore(IGF, bindingsAddr, MetadataState::Complete);
  }

  // Figure out the non-fixed offsets.
  HeapNonFixedOffsets offsets(IGF, layout);

  // Destroy the fields.
  for (unsigned i : indices(layout.getElements())) {
    auto &field = layout.getElement(i);
    auto fieldTy = layout.getElementTypes()[i];
    if (field.isTriviallyDestroyable())
      continue;

    field.getType().destroy(
        IGF, field.project(IGF, structAddr, offsets), fieldTy,
        true /*Called from metadata constructors: must be outlined*/);
  }

  emitDeallocateHeapObject(IGF, &*fn->arg_begin(), offsets.getSize(),
                           offsets.getAlignMask());
  IGF.Builder.CreateRetVoid();

  return fn;
}

/// Create the size function for a layout.
/// TODO: give this some reasonable name and possibly linkage.
toolchain::Constant *HeapLayout::createSizeFn(IRGenModule &IGM) const {
  toolchain::Function *fn =
    toolchain::Function::Create(IGM.DeallocatingDtorTy,
                           toolchain::Function::PrivateLinkage,
                           "objectsize", &IGM.Module);
  fn->setAttributes(IGM.constructInitialAttributes());

  IRGenFunction IGF(IGM, fn);
  if (IGM.DebugInfo)
    IGM.DebugInfo->emitArtificialFunction(IGF, fn);

  // Ignore the object pointer; we aren't a dynamically-sized array,
  // so it's pointless.

  toolchain::Value *size = emitSize(IGM);
  IGF.Builder.CreateRet(size);

  return fn;
}

static toolchain::Constant *buildPrivateMetadata(IRGenModule &IGM,
                                            const HeapLayout &layout,
                                            toolchain::Constant *dtorFn,
                                            toolchain::Constant *captureDescriptor,
                                            MetadataKind kind) {
  // Build the fields of the private metadata.
  ConstantInitBuilder builder(IGM);

  // In embedded Codira, heap objects have a different, simple(r) layout:
  // superclass pointer + destructor.
  if (IGM.Context.LangOpts.hasFeature(Feature::Embedded)) {
    auto fields = builder.beginStruct();
    fields.addNullPointer(IGM.Int8PtrTy);
    fields.addSignedPointer(dtorFn,
                            IGM.getOptions().PointerAuth.HeapDestructors,
                            PointerAuthEntity::Special::HeapDestructor);

    toolchain::GlobalVariable *var = fields.finishAndCreateGlobal(
        "metadata", IGM.getPointerAlignment(), /*constant*/ true,
        toolchain::GlobalVariable::PrivateLinkage);
    return var;
  }

  auto fields = builder.beginStruct(IGM.FullBoxMetadataStructTy);

  fields.addSignedPointer(dtorFn, IGM.getOptions().PointerAuth.HeapDestructors,
                          PointerAuthEntity::Special::HeapDestructor);
  fields.addNullPointer(IGM.WitnessTablePtrTy);
  {
    auto kindStruct = fields.beginStruct(IGM.TypeMetadataStructTy);
    kindStruct.add(getMetadataKind(IGM, kind));
    kindStruct.finishAndAddTo(fields);
  }

  // Figure out the offset to the first element, which is necessary to be able
  // to polymorphically project as a generic box.
  auto elements = layout.getElements();
  Size offset;
  if (!elements.empty()
      && elements[0].getKind() == ElementLayout::Kind::Fixed)
    offset = elements[0].getByteOffset();
  else
    offset = Size(0);
  fields.addInt32(offset.getValue());

  fields.add(captureDescriptor);

  toolchain::GlobalVariable *var =
    fields.finishAndCreateGlobal("metadata",
                                 IGM.getPointerAlignment(),
                                 /*constant*/ true,
                                 toolchain::GlobalVariable::PrivateLinkage);

  toolchain::Constant *indices[] = {
    toolchain::ConstantInt::get(IGM.Int32Ty, 0),
    toolchain::ConstantInt::get(IGM.Int32Ty, 2)
  };
  return toolchain::ConstantExpr::getInBoundsGetElementPtr(var->getValueType(), var,
                                                      indices);
}

toolchain::Constant *
HeapLayout::getPrivateMetadata(IRGenModule &IGM,
                               toolchain::Constant *captureDescriptor) const {
  if (!privateMetadata)
    privateMetadata = buildPrivateMetadata(IGM, *this, createDtorFn(IGM, *this),
                                           captureDescriptor,
                                           MetadataKind::HeapLocalVariable);
  return privateMetadata;
}

toolchain::Value *IRGenFunction::emitUnmanagedAlloc(const HeapLayout &layout,
                                               const toolchain::Twine &name,
                                           toolchain::Constant *captureDescriptor,
                                           const HeapNonFixedOffsets *offsets) {
  if (layout.isKnownEmpty()
      && layout.isTriviallyDestroyable()) {
    return IGM.RefCountedNull;
  }

  toolchain::Value *metadata = layout.getPrivateMetadata(IGM, captureDescriptor);
  toolchain::Value *size, *alignMask;
  if (offsets) {
    size = offsets->getSize();
    alignMask = offsets->getAlignMask();
  } else {
    size = layout.emitSize(IGM);
    alignMask = layout.emitAlignMask(IGM);
  }

  return emitAllocObjectCall(metadata, size, alignMask, name);
}

namespace {
  class BuiltinNativeObjectTypeInfo
    : public HeapTypeInfo<BuiltinNativeObjectTypeInfo> {
  public:
    BuiltinNativeObjectTypeInfo(toolchain::PointerType *storage,
                                 Size size, SpareBitVector spareBits,
                                 Alignment align)
    : HeapTypeInfo(ReferenceCounting::Native, storage, size, spareBits,
                   align) {}

    /// Builtin.NativeObject uses Codira native reference-counting.
    ReferenceCounting getReferenceCounting() const {
      return ReferenceCounting::Native;
    }
  };
} // end anonymous namespace

const LoadableTypeInfo *TypeConverter::convertBuiltinNativeObject() {
  return new BuiltinNativeObjectTypeInfo(IGM.RefCountedPtrTy,
                                      IGM.getPointerSize(),
                                      IGM.getHeapObjectSpareBits(),
                                      IGM.getPointerAlignment());
}

unsigned IRGenModule::getReferenceStorageExtraInhabitantCount(
                                                ReferenceOwnership ownership,
                                                ReferenceCounting style) const {
  switch (style) {
  case ReferenceCounting::Native:
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
    if (ownership == ReferenceOwnership::Name) \
      break;
#include "language/AST/ReferenceStorage.def"
    if (ObjCInterop)
      break;
    return getHeapObjectExtraInhabitantCount(*this);
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Unknown:
  case ReferenceCounting::Custom:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }

  // The default behavior uses pointer semantics, therefore null is the only
  // extra inhabitant allowed.
  return 1;
}

SpareBitVector IRGenModule::getReferenceStorageSpareBits(
                                                ReferenceOwnership ownership,
                                                ReferenceCounting style) const {
  // We have to be conservative (even with native reference-counting) in order
  // to interoperate with code that might be working more generically with the
  // memory/type.
  switch (style) {
  case ReferenceCounting::Native:
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
    if (ownership == ReferenceOwnership::Name) \
      break;
#include "language/AST/ReferenceStorage.def"
    if (ObjCInterop)
      break;
    return getHeapObjectSpareBits();
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Custom:
  case ReferenceCounting::Unknown:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }

  // The default behavior uses pointer semantics.
  return SpareBitVector::getConstant(getPointerSize().getValueInBits(), false);
}

APInt IRGenModule::getReferenceStorageExtraInhabitantValue(unsigned bits,
                                                unsigned index,
                                                ReferenceOwnership ownership,
                                                ReferenceCounting style) const {
  // We have to be conservative (even with native reference-counting) in order
  // to interoperate with code that might be working more generically with the
  // memory/type.
  switch (style) {
  case ReferenceCounting::Native:
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
    if (ownership == ReferenceOwnership::Name) \
      break;
#include "language/AST/ReferenceStorage.def"
    if (ObjCInterop)
      break;
    return getHeapObjectFixedExtraInhabitantValue(*this, bits, index, 0);
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Custom:
  case ReferenceCounting::Unknown:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }

  // The default behavior allows for only one legal extra inhabitant, therefore
  // this must be the null pattern.
  assert(index == 0);
  return APInt(bits, 0);
}

APInt IRGenModule::getReferenceStorageExtraInhabitantMask(
                                                ReferenceOwnership ownership,
                                                ReferenceCounting style) const {
  switch (style) {
  case ReferenceCounting::Native:
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Custom:
  case ReferenceCounting::Unknown:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }
  return APInt::getAllOnes(getPointerSize().getValueInBits());
}

toolchain::Value *IRGenFunction::getReferenceStorageExtraInhabitantIndex(Address src,
                                                   ReferenceOwnership ownership,
                                                   ReferenceCounting style) {
  switch (style) {
  case ReferenceCounting::Native:
    if (IGM.ObjCInterop)
      break;
    return getHeapObjectExtraInhabitantIndex(*this, src);
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Custom:
  case ReferenceCounting::Unknown:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }

  // The default behavior allows for only one legal extra inhabitant, therefore
  // this must be the null pattern.
  auto PtrTy =
#define CHECKED_REF_STORAGE(Name, ...) \
    ownership == ReferenceOwnership::Name ? IGM.Name##ReferencePtrTy :
#include "language/AST/ReferenceStorage.def"
    nullptr;
  (void)PtrTy;
  assert(src.getAddress()->getType() == PtrTy);
  src = Builder.CreateStructGEP(src, 0, Size(0));
  toolchain::Value *ptr = Builder.CreateLoad(src);
  toolchain::Value *isNull = Builder.CreateIsNull(ptr);
  toolchain::Value *result =
    Builder.CreateSelect(isNull, Builder.getInt32(0),
                         toolchain::ConstantInt::getSigned(IGM.Int32Ty, -1));
  return result;
}

void IRGenFunction::storeReferenceStorageExtraInhabitant(toolchain::Value *index,
                                                   Address dest,
                                                   ReferenceOwnership ownership,
                                                   ReferenceCounting style) {
  switch (style) {
  case ReferenceCounting::Native:
    if (IGM.ObjCInterop)
      break;
    return storeHeapObjectExtraInhabitant(*this, index, dest);
  case ReferenceCounting::Block:
  case ReferenceCounting::ObjC:
  case ReferenceCounting::None:
  case ReferenceCounting::Custom:
  case ReferenceCounting::Unknown:
    break;
  case ReferenceCounting::Bridge:
  case ReferenceCounting::Error:
    toolchain_unreachable("Unsupported reference-counting style");
  }

  // The default behavior allows for only one legal extra inhabitant, therefore
  // this must be the null pattern.
  auto PtrTy =
#define CHECKED_REF_STORAGE(Name, ...) \
    ownership == ReferenceOwnership::Name ? IGM.Name##ReferencePtrTy :
#include "language/AST/ReferenceStorage.def"
    nullptr;
  (void)PtrTy;
  assert(dest.getAddress()->getType() == PtrTy);
  dest = Builder.CreateStructGEP(dest, 0, Size(0));
  toolchain::Value *null = toolchain::ConstantPointerNull::get(IGM.RefCountedPtrTy);
  Builder.CreateStore(null, dest);
}

#define SOMETIMES_UNKNOWN(Name)                                                \
  const TypeInfo *TypeConverter::create##Name##StorageType(                    \
      toolchain::Type *valueType, ReferenceCounting style, bool isOptional) {       \
    auto &&spareBits =                                                         \
        IGM.getReferenceStorageSpareBits(ReferenceOwnership::Name, style);     \
    switch (style) {                                                           \
    case ReferenceCounting::Native:                                            \
      return new Native##Name##ReferenceTypeInfo(                              \
          valueType, IGM.Name##ReferenceStructTy, IGM.getPointerSize(),        \
          IGM.getPointerAlignment(), std::move(spareBits), isOptional);        \
    case ReferenceCounting::ObjC:                                              \
    case ReferenceCounting::None:                                              \
    case ReferenceCounting::Custom:                                            \
    case ReferenceCounting::Block:                                             \
    case ReferenceCounting::Unknown:                                           \
      return new Unknown##Name##ReferenceTypeInfo(                             \
          valueType, IGM.Name##ReferenceStructTy, IGM.getPointerSize(),        \
          IGM.getPointerAlignment(), std::move(spareBits), isOptional);        \
    case ReferenceCounting::Bridge:                                            \
    case ReferenceCounting::Error:                                             \
      toolchain_unreachable("not supported!");                                      \
    }                                                                          \
    toolchain_unreachable("bad reference-counting style");                          \
  }
#define ALWAYS_NATIVE(Name) \
  const TypeInfo * \
  TypeConverter::create##Name##StorageType(toolchain::Type *valueType, \
                                           ReferenceCounting style, \
                                           bool isOptional) { \
    assert(style == ReferenceCounting::Native); \
    auto &&spareBits = IGM.getReferenceStorageSpareBits( \
                                                   ReferenceOwnership::Name, \
                                                   ReferenceCounting::Native); \
    return new Native##Name##ReferenceTypeInfo(valueType, \
                                   IGM.Name##ReferencePtrTy->getElementType(), \
                                   IGM.getPointerSize(), \
                                   IGM.getPointerAlignment(), \
                                   std::move(spareBits), \
                                   isOptional); \
  }

  // Always native versus "sometimes unknown" reference storage type policy:
  SOMETIMES_UNKNOWN(Weak)
  SOMETIMES_UNKNOWN(Unowned)
#undef SOMETIMES_UNKNOWN
#undef ALWAYS_NATIVE

#define CHECKED_REF_STORAGE(Name, ...) \
  static_assert(&TypeConverter::create##Name##StorageType != nullptr, \
    "Missing reference storage type converter helper constructor");
#define UNCHECKED_REF_STORAGE(Name, ...) \
  const TypeInfo * \
  TypeConverter::create##Name##StorageType(toolchain::Type *valueType, \
                                           ReferenceCounting style, \
                                           bool isOptional) { \
    (void)style; /* unused */ \
    return new Name##ReferenceTypeInfo(valueType, \
                                       IGM.getHeapObjectSpareBits(), \
                                       IGM.getPointerSize(), \
                                       IGM.getPointerAlignment(), \
                                       isOptional); \
  }
#include "language/AST/ReferenceStorage.def"

/// Does the given value superficially not require reference-counting?
static bool doesNotRequireRefCounting(toolchain::Value *value) {
  // Constants never require reference-counting.
  return isa<toolchain::ConstantPointerNull>(value);
}

/// Emit a unary call to perform a ref-counting operation.
///
/// \param fn - expected signature 'void (T)' or 'T (T)'
static void emitUnaryRefCountCall(IRGenFunction &IGF,
                                  toolchain::Constant *fn,
                                  toolchain::Value *value) {
  auto fun = cast<toolchain::Function>(fn);
  auto cc = fun->getCallingConv();

  // Instead of casting the input, we cast the function type.
  // This tends to produce less IR, but might be evil.
  auto fnType = cast<toolchain::FunctionType>(fun->getValueType());
  if (value->getType() != fnType->getParamType(0)) {
    auto resultTy = fnType->getReturnType() == IGF.IGM.VoidTy
                        ? IGF.IGM.VoidTy
                        : value->getType();
    fnType = toolchain::FunctionType::get(resultTy, value->getType(), false);
    fn = toolchain::ConstantExpr::getBitCast(fn, fnType->getPointerTo());
  }

  // Emit the call.
  toolchain::CallInst *call = IGF.Builder.CreateCallWithoutDbgLoc(fnType, fn, value);
  if (fun && fun->hasParamAttribute(0, toolchain::Attribute::Returned))
    call->addParamAttr(0, toolchain::Attribute::Returned);
  call->setCallingConv(cc);
  call->setDoesNotThrow();
}

/// Emit a copy-like call to perform a ref-counting operation.
///
/// \param fn - expected signature 'void (T, T)' or 'T (T, T)'
static void emitCopyLikeCall(IRGenFunction &IGF,
                             toolchain::Constant *fn,
                             toolchain::Value *dest,
                             toolchain::Value *src) {
  assert(dest->getType() == src->getType() &&
         "type mismatch in binary refcounting operation");

  auto fun = cast<toolchain::Function>(fn);
  auto cc = fun->getCallingConv();

  // Instead of casting the inputs, we cast the function type.
  // This tends to produce less IR, but might be evil.
  auto fnType = cast<toolchain::FunctionType>(fun->getValueType());
  if (dest->getType() != fnType->getParamType(0)) {
    toolchain::Type *paramTypes[] = { dest->getType(), dest->getType() };
    auto resultTy = fnType->getReturnType() == IGF.IGM.VoidTy ? IGF.IGM.VoidTy
                                                              : dest->getType();
    fnType = toolchain::FunctionType::get(resultTy, paramTypes, false);
    fn = toolchain::ConstantExpr::getBitCast(fn, fnType->getPointerTo());
  }

  // Emit the call.
  toolchain::CallInst *call =
      IGF.Builder.CreateCallWithoutDbgLoc(fnType, fn, {dest, src});
  if (fun && fun->hasParamAttribute(0, toolchain::Attribute::Returned))
    call->addParamAttr(0, toolchain::Attribute::Returned);
  call->setCallingConv(cc);
  call->setDoesNotThrow();
}

/// Emit a call to a function with a loadWeak-like signature.
///
/// \param fn - expected signature 'T (Weak*)'
static toolchain::Value *emitLoadWeakLikeCall(IRGenFunction &IGF,
                                         toolchain::Constant *fn,
                                         toolchain::Value *addr,
                                         toolchain::Type *resultType) {
  assert((addr->getType() == IGF.IGM.WeakReferencePtrTy ||
          addr->getType() == IGF.IGM.UnownedReferencePtrTy) &&
         "address is not of a weak or unowned reference");

  auto fun = cast<toolchain::Function>(fn);
  auto cc = fun->getCallingConv();

  // Instead of casting the output, we cast the function type.
  // This tends to produce less IR, but might be evil.
  auto fnType = cast<toolchain::FunctionType>(fun->getValueType());
  if (resultType != fnType->getReturnType()) {
    toolchain::Type *paramTypes[] = { addr->getType() };
    fnType = toolchain::FunctionType::get(resultType, paramTypes, false);
    fn = toolchain::ConstantExpr::getBitCast(fn, fnType->getPointerTo());
  }

  // Emit the call.
  toolchain::CallInst *call = IGF.Builder.CreateCallWithoutDbgLoc(fnType, fn, addr);
  call->setCallingConv(cc);
  call->setDoesNotThrow();

  return call;
}

/// Emit a call to a function with a storeWeak-like signature.
///
/// \param fn - expected signature 'void (Weak*, T)' or 'Weak* (Weak*, T)'
static void emitStoreWeakLikeCall(IRGenFunction &IGF,
                                  toolchain::Constant *fn,
                                  toolchain::Value *addr,
                                  toolchain::Value *value) {
  assert((addr->getType() == IGF.IGM.WeakReferencePtrTy ||
          addr->getType() == IGF.IGM.UnownedReferencePtrTy) &&
         "address is not of a weak or unowned reference");

  auto fun = cast<toolchain::Function>(fn);
  auto cc = fun->getCallingConv();

  // Instead of casting the inputs, we cast the function type.
  // This tends to produce less IR, but might be evil.
  auto fnType = cast<toolchain::FunctionType>(fun->getValueType());
  if (value->getType() != fnType->getParamType(1)) {
    toolchain::Type *paramTypes[] = { addr->getType(), value->getType() };
    auto resultTy = fnType->getReturnType() == IGF.IGM.VoidTy ? IGF.IGM.VoidTy
                                                              : addr->getType();
    fnType = toolchain::FunctionType::get(resultTy, paramTypes, false);
    fn = toolchain::ConstantExpr::getBitCast(fn, fnType->getPointerTo());
  }

  // Emit the call.
  toolchain::CallInst *call =
      IGF.Builder.CreateCallWithoutDbgLoc(fnType, fn, {addr, value});
  if (fun && fun->hasParamAttribute(0, toolchain::Attribute::Returned))
    call->addParamAttr(0, toolchain::Attribute::Returned);
  call->setCallingConv(cc);
  call->setDoesNotThrow();
}

/// Emit a call to language_retain.
void IRGenFunction::emitNativeStrongRetain(toolchain::Value *value,
                                           Atomicity atomicity) {
  if (doesNotRequireRefCounting(value))
    return;

  // Make sure the input pointer is the right type.
  if (value->getType() != IGM.RefCountedPtrTy)
    value = Builder.CreateBitCast(value, IGM.RefCountedPtrTy);

  // Emit the call.
  toolchain::CallInst *call = Builder.CreateCall(
      (atomicity == Atomicity::Atomic)
          ? IGM.getNativeStrongRetainFunctionPointer()
          : IGM.getNativeNonAtomicStrongRetainFunctionPointer(),
      value);
  call->setDoesNotThrow();
  call->addParamAttr(0, toolchain::Attribute::Returned);
}

/// Emit a store of a live value to the given retaining variable.
void IRGenFunction::emitNativeStrongAssign(toolchain::Value *newValue,
                                           Address address) {
  // Pull the old value out of the address.
  toolchain::Value *oldValue = Builder.CreateLoad(address);

  // We assume the new value is already retained.
  Builder.CreateStore(newValue, address);

  // Release the old value.
  emitNativeStrongRelease(oldValue, getDefaultAtomicity());
}

/// Emit an initialize of a live value to the given retaining variable.
void IRGenFunction::emitNativeStrongInit(toolchain::Value *newValue,
                                         Address address) {
  // We assume the new value is already retained.
  Builder.CreateStore(newValue, address);
}

/// Emit a release of a live value with the given refcounting implementation.
void IRGenFunction::emitStrongRelease(toolchain::Value *value,
                                      ReferenceCounting refcounting,
                                      Atomicity atomicity) {
  switch (refcounting) {
  case ReferenceCounting::Native:
    return emitNativeStrongRelease(value, atomicity);
  case ReferenceCounting::ObjC:
    return emitObjCStrongRelease(value);
  case ReferenceCounting::Block:
    return emitBlockRelease(value);
  case ReferenceCounting::Unknown:
    return emitUnknownStrongRelease(value, atomicity);
  case ReferenceCounting::Bridge:
    return emitBridgeStrongRelease(value, atomicity);
  case ReferenceCounting::Error:
    return emitErrorStrongRelease(value);
  case ReferenceCounting::Custom:
    toolchain_unreachable("Ref counting should be handled in TypeInfo.");
  case ReferenceCounting::None:
    return; // This is a no-op if we don't have any ref-counting.
  }
}

void IRGenFunction::emitStrongRetain(toolchain::Value *value,
                                     ReferenceCounting refcounting,
                                     Atomicity atomicity) {
  switch (refcounting) {
  case ReferenceCounting::Native:
    emitNativeStrongRetain(value, atomicity);
    return;
  case ReferenceCounting::Bridge:
    emitBridgeStrongRetain(value, atomicity);
    return;
  case ReferenceCounting::ObjC:
    emitObjCStrongRetain(value);
    return;
  case ReferenceCounting::Block:
    emitBlockCopyCall(value);
    return;
  case ReferenceCounting::Unknown:
    emitUnknownStrongRetain(value, atomicity);
    return;
  case ReferenceCounting::Error:
    emitErrorStrongRetain(value);
    return;
  case ReferenceCounting::Custom:
    toolchain_unreachable("Ref counting should be handled in TypeInfo.");
  case ReferenceCounting::None:
    return; // This is a no-op if we don't have any ref-counting.
  }
}

toolchain::Type *IRGenModule::getReferenceType(ReferenceCounting refcounting) {
  switch (refcounting) {
  case ReferenceCounting::Native:
    return RefCountedPtrTy;
  case ReferenceCounting::Bridge:
    return BridgeObjectPtrTy;
  case ReferenceCounting::ObjC:
    return ObjCPtrTy;
  case ReferenceCounting::Block:
    return ObjCBlockPtrTy;
  case ReferenceCounting::Unknown:
    return UnknownRefCountedPtrTy;
  case ReferenceCounting::Error:
    return ErrorPtrTy;
  case ReferenceCounting::Custom:
  case ReferenceCounting::None:
    return OpaquePtrTy;
  }

  toolchain_unreachable("Not a valid ReferenceCounting.");
}

#define DEFINE_BINARY_OPERATION(KIND, RESULT, TYPE1, TYPE2)                    \
  RESULT IRGenFunction::emit##KIND(TYPE1 val1, TYPE2 val2,                     \
                                   ReferenceCounting style) {                  \
    switch (style) {                                                           \
    case ReferenceCounting::Native:                                            \
      return emitNative##KIND(val1, val2);                                     \
    case ReferenceCounting::ObjC:                                              \
    case ReferenceCounting::Unknown:                                           \
      return emitUnknown##KIND(val1, val2);                                    \
    case ReferenceCounting::Bridge:                                            \
    case ReferenceCounting::Block:                                             \
    case ReferenceCounting::Error:                                             \
    case ReferenceCounting::Custom:                                            \
    case ReferenceCounting::None:                                              \
      toolchain_unreachable("unsupported reference kind with reference storage");   \
    }                                                                          \
    toolchain_unreachable("bad refcounting style");                                 \
  }

#define DEFINE_UNARY_OPERATION(KIND, RESULT, TYPE1)                            \
  RESULT IRGenFunction::emit##KIND(TYPE1 val1, ReferenceCounting style) {      \
    switch (style) {                                                           \
    case ReferenceCounting::Native:                                            \
      return emitNative##KIND(val1);                                           \
    case ReferenceCounting::ObjC:                                              \
    case ReferenceCounting::Unknown:                                           \
      return emitUnknown##KIND(val1);                                          \
    case ReferenceCounting::Bridge:                                            \
    case ReferenceCounting::Block:                                             \
    case ReferenceCounting::Error:                                             \
    case ReferenceCounting::Custom:                                            \
    case ReferenceCounting::None:                                              \
      toolchain_unreachable("unsupported reference kind with reference storage");   \
    }                                                                          \
    toolchain_unreachable("bad refcounting style");                                 \
  }

#define NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  DEFINE_BINARY_OPERATION(Name##CopyInit, void, Address, Address) \
  DEFINE_BINARY_OPERATION(Name##TakeInit, void, Address, Address) \
  DEFINE_BINARY_OPERATION(Name##CopyAssign, void, Address, Address) \
  DEFINE_BINARY_OPERATION(Name##TakeAssign, void, Address, Address) \
  DEFINE_BINARY_OPERATION(Name##Init, void, toolchain::Value *, Address) \
  DEFINE_BINARY_OPERATION(Name##Assign, void, toolchain::Value *, Address) \
  DEFINE_BINARY_OPERATION(Name##LoadStrong, toolchain::Value *, Address,toolchain::Type*)\
  DEFINE_BINARY_OPERATION(Name##TakeStrong, toolchain::Value *, Address,toolchain::Type*)\
  DEFINE_UNARY_OPERATION(Name##Destroy, void, Address)
#include "language/AST/ReferenceStorage.def"


#undef DEFINE_UNARY_OPERATION
#undef DEFINE_BINARY_OPERATION

#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  void IRGenFunction::emit##Name##Retain(toolchain::Value *value, \
                                        ReferenceCounting style, \
                                        Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native && \
           "only native references support scalar reference-counting"); \
    emitNative##Name##Retain(value, atomicity); \
  } \
  void IRGenFunction::emit##Name##Release(toolchain::Value *value, \
                                         ReferenceCounting style, \
                                         Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native && \
           "only native references support scalar reference-counting"); \
    emitNative##Name##Release(value, atomicity); \
  } \
  void IRGenFunction::emitStrongRetain##Name(toolchain::Value *value, \
                                              ReferenceCounting style, \
                                              Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native && \
           "only native references support scalar reference-counting"); \
    emitNativeStrongRetain##Name(value, atomicity); \
  } \
  void IRGenFunction::emitStrongRetainAnd##Name##Release(toolchain::Value *value, \
                                                        ReferenceCounting style, \
                                                        Atomicity atomicity) { \
    assert(style == ReferenceCounting::Native && \
           "only native references support scalar reference-counting"); \
    emitNativeStrongRetainAnd##Name##Release(value, atomicity); \
  } \
  void IRGenFunction::emitNative##Name##Init(toolchain::Value *value, \
                                            Address dest) { \
    value = Builder.CreateBitCast(value, IGM.RefCountedPtrTy); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    Builder.CreateStore(value, dest); \
    emitNative##Name##Retain(value, getDefaultAtomicity()); \
  } \
  void IRGenFunction::emitNative##Name##Assign(toolchain::Value *value, \
                                              Address dest) { \
    value = Builder.CreateBitCast(value, IGM.RefCountedPtrTy); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    auto oldValue = Builder.CreateLoad(dest); \
    Builder.CreateStore(value, dest); \
    emitNative##Name##Retain(value, getDefaultAtomicity()); \
    emitNative##Name##Release(oldValue, getDefaultAtomicity()); \
  } \
  toolchain::Value *IRGenFunction::emitNative##Name##LoadStrong(Address src, \
                                                          toolchain::Type *type) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    toolchain::Value *value = Builder.CreateLoad(src); \
    value = Builder.CreateBitCast(value, type); \
    emitNativeStrongRetain##Name(value, getDefaultAtomicity()); \
    return value; \
  } \
  toolchain::Value *IRGenFunction::emitNative##Name##TakeStrong(Address src, \
                                                          toolchain::Type *type) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    toolchain::Value *value = Builder.CreateLoad(src); \
    value = Builder.CreateBitCast(value, type); \
    emitNativeStrongRetainAnd##Name##Release(value, getDefaultAtomicity()); \
    return value; \
  } \
  void IRGenFunction::emitNative##Name##Destroy(Address ref) { \
    ref = Builder.CreateStructGEP(ref, 0, Size(0)); \
    toolchain::Value *value = Builder.CreateLoad(ref); \
    emitNative##Name##Release(value, getDefaultAtomicity()); \
  } \
  void IRGenFunction::emitNative##Name##CopyInit(Address dest, Address src) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    toolchain::Value *newValue = Builder.CreateLoad(src); \
    Builder.CreateStore(newValue, dest); \
    emitNative##Name##Retain(newValue, getDefaultAtomicity()); \
  } \
  void IRGenFunction::emitNative##Name##TakeInit(Address dest, Address src) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    toolchain::Value *newValue = Builder.CreateLoad(src); \
    Builder.CreateStore(newValue, dest); \
  } \
  void IRGenFunction::emitNative##Name##CopyAssign(Address dest, Address src) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    toolchain::Value *newValue = Builder.CreateLoad(src); \
    toolchain::Value *oldValue = Builder.CreateLoad(dest); \
    Builder.CreateStore(newValue, dest); \
    emitNative##Name##Retain(newValue, getDefaultAtomicity()); \
    emitNative##Name##Release(oldValue, getDefaultAtomicity()); \
  } \
  void IRGenFunction::emitNative##Name##TakeAssign(Address dest, Address src) { \
    src = Builder.CreateStructGEP(src, 0, Size(0)); \
    dest = Builder.CreateStructGEP(dest, 0, Size(0)); \
    toolchain::Value *newValue = Builder.CreateLoad(src); \
    toolchain::Value *oldValue = Builder.CreateLoad(dest); \
    Builder.CreateStore(newValue, dest); \
    emitNative##Name##Release(oldValue, getDefaultAtomicity()); \
  }
#include "language/AST/ReferenceStorage.def"

/// Emit a release of a live value.
void IRGenFunction::emitNativeStrongRelease(toolchain::Value *value,
                                            Atomicity atomicity) {
  if (doesNotRequireRefCounting(value))
    return;
  emitUnaryRefCountCall(*this, (atomicity == Atomicity::Atomic)
                                   ? IGM.getNativeStrongReleaseFn()
                                   : IGM.getNativeNonAtomicStrongReleaseFn(),
                        value);
}

void IRGenFunction::emitNativeSetDeallocating(toolchain::Value *value) {
  if (doesNotRequireRefCounting(value)) return;
  emitUnaryRefCountCall(*this, IGM.getNativeSetDeallocatingFn(), value);
}

toolchain::Constant *IRGenModule::getFixLifetimeFn() {
  if (FixLifetimeFn)
    return FixLifetimeFn;
  
  // Generate a private stub function for the LLVM ARC optimizer to recognize.
  auto fixLifetimeTy = toolchain::FunctionType::get(VoidTy, RefCountedPtrTy,
                                               /*isVarArg*/ false);
  auto fixLifetime = toolchain::Function::Create(fixLifetimeTy,
                                         toolchain::GlobalValue::PrivateLinkage,
                                         "__language_fixLifetime",
                                         &Module);
  assert(fixLifetime->getName() == "__language_fixLifetime"
         && "fixLifetime symbol name got mangled?!");
  // Don't inline the function, so it stays as a signal to the ARC passes.
  // The ARC passes will remove references to the function when they're
  // no longer needed.
  fixLifetime->addFnAttr(toolchain::Attribute::NoInline);

  // Give the function an empty body.
  auto entry = toolchain::BasicBlock::Create(getLLVMContext(), "", fixLifetime);
  toolchain::ReturnInst::Create(getLLVMContext(), entry);
  
  FixLifetimeFn = fixLifetime;
  return fixLifetime;
}

FunctionPointer IRGenModule::getFixedClassInitializationFn() {
  if (FixedClassInitializationFn)
    return *FixedClassInitializationFn;
  
  // If ObjC interop is disabled, we don't need to do fixed class
  // initialization.
  FunctionPointer fn;
  if (ObjCInterop) {
    // In new enough ObjC runtimes, objc_opt_self provides a direct fast path
    // to realize a class.
    if (getAvailabilityRange().isContainedIn(
            Context.getCodira51Availability())) {
      fn = getObjCOptSelfFunctionPointer();
    }
    // Otherwise, the Codira runtime always provides a `get
    else {
      fn = getGetInitializedObjCClassFunctionPointer();
    }
  }

  FixedClassInitializationFn = fn;
  return fn;
}

/// Fix the lifetime of a live value. This communicates to the LLVM level ARC
/// optimizer not to touch this value.
void IRGenFunction::emitFixLifetime(toolchain::Value *value) {
  // If we aren't running the LLVM ARC optimizer, we don't need to emit this.
  if (!IGM.IRGen.Opts.shouldOptimize() ||
      IGM.IRGen.Opts.DisableCodiraSpecificLLVMOptzns)
    return;
  if (doesNotRequireRefCounting(value)) return;
  emitUnaryRefCountCall(*this, IGM.getFixLifetimeFn(), value);
}

void IRGenFunction::emitUnknownStrongRetain(toolchain::Value *value,
                                            Atomicity atomicity) {
  if (doesNotRequireRefCounting(value))
    return;
  emitUnaryRefCountCall(*this,
                        (atomicity == Atomicity::Atomic)
                            ? IGM.getUnknownObjectRetainFn()
                            : IGM.getNonAtomicUnknownObjectRetainFn(),
                        value);
}

void IRGenFunction::emitUnknownStrongRelease(toolchain::Value *value,
                                             Atomicity atomicity) {
  if (doesNotRequireRefCounting(value))
    return;
  emitUnaryRefCountCall(*this,
                        (atomicity == Atomicity::Atomic)
                            ? IGM.getUnknownObjectReleaseFn()
                            : IGM.getNonAtomicUnknownObjectReleaseFn(),
                        value);
}

void IRGenFunction::emitBridgeStrongRetain(toolchain::Value *value,
                                           Atomicity atomicity) {
  emitUnaryRefCountCall(*this,
                        (atomicity == Atomicity::Atomic)
                            ? IGM.getBridgeObjectStrongRetainFn()
                            : IGM.getNonAtomicBridgeObjectStrongRetainFn(),
                        value);
}

void IRGenFunction::emitBridgeStrongRelease(toolchain::Value *value,
                                            Atomicity atomicity) {
  emitUnaryRefCountCall(*this,
                        (atomicity == Atomicity::Atomic)
                            ? IGM.getBridgeObjectStrongReleaseFn()
                            : IGM.getNonAtomicBridgeObjectStrongReleaseFn(),
                        value);
}

void IRGenFunction::emitErrorStrongRetain(toolchain::Value *value) {
  emitUnaryRefCountCall(*this, IGM.getErrorStrongRetainFn(), value);
}

void IRGenFunction::emitErrorStrongRelease(toolchain::Value *value) {
  emitUnaryRefCountCall(*this, IGM.getErrorStrongReleaseFn(), value);
}

toolchain::Value *IRGenFunction::emitLoadRefcountedPtr(Address addr,
                                                  ReferenceCounting style) {
  Address src = Builder.CreateElementBitCast(addr, IGM.getReferenceType(style));
  return Builder.CreateLoad(src);
}

toolchain::Value *IRGenFunction::
emitIsUniqueCall(toolchain::Value *value, ReferenceCounting style, SourceLoc loc, bool isNonNull) {
  FunctionPointer fn;
  bool nonObjC = !IGM.getAvailabilityRange().isContainedIn(
      IGM.Context.getObjCIsUniquelyReferencedAvailability());
  switch (style) {
  case ReferenceCounting::Native: {
    if (isNonNull)
      fn = IGM.getIsUniquelyReferenced_nonNull_nativeFunctionPointer();
    else
      fn = IGM.getIsUniquelyReferenced_nativeFunctionPointer();
  }
  break;
  case ReferenceCounting::Bridge: {
    if (!isNonNull)
      unimplemented(loc, "optional bridge ref");

    if (nonObjC)
      fn =
          IGM.getIsUniquelyReferencedNonObjC_nonNull_bridgeObjectFunctionPointer();
    else
      fn = IGM.getIsUniquelyReferenced_nonNull_bridgeObjectFunctionPointer();
  }
  break;
  case ReferenceCounting::ObjC:
  case ReferenceCounting::Unknown: {
    if (nonObjC) {
      if (isNonNull)
        fn = IGM.getIsUniquelyReferencedNonObjC_nonNullFunctionPointer();
      else
        fn = IGM.getIsUniquelyReferencedNonObjCFunctionPointer();
    } else {
      if (isNonNull)
        fn = IGM.getIsUniquelyReferenced_nonNullFunctionPointer();
      else
        fn = IGM.getIsUniquelyReferencedFunctionPointer();
    }
  }
  break;
  case ReferenceCounting::Error:
  case ReferenceCounting::Block:
  case ReferenceCounting::Custom:
  case ReferenceCounting::None:
    toolchain_unreachable("Unexpected LLVM type for a refcounted pointer.");
  }

  toolchain::CallInst *call = Builder.CreateCall(fn, value);
  call->setDoesNotThrow();
  return call;
}

toolchain::Value *IRGenFunction::emitIsEscapingClosureCall(
    toolchain::Value *value, SourceLoc sourceLoc, unsigned verificationType) {
  auto loc = SILLocation::decode(sourceLoc, IGM.Context.SourceMgr);
  auto line = toolchain::ConstantInt::get(IGM.Int32Ty, loc.line);
  auto col = toolchain::ConstantInt::get(IGM.Int32Ty, loc.column);

  // Only output the filepath in debug mode. It is going to leak into the
  // executable. This is the same behavior as asserts.
  auto filename = IGM.IRGen.Opts.shouldOptimize()
                      ? IGM.getAddrOfGlobalString("")
                      : IGM.getAddrOfGlobalString(loc.filename);
  auto filenameLength =
      toolchain::ConstantInt::get(IGM.Int32Ty, loc.filename.size());
  auto type = toolchain::ConstantInt::get(IGM.Int32Ty, verificationType);
  toolchain::CallInst *call = Builder.CreateCall(
      IGM.getIsEscapingClosureAtFileLocationFunctionPointer(),
      {value, filename, filenameLength, line, col, type});
  call->setDoesNotThrow();
  return call;
}

namespace {
/// Basic layout and common operations for box types.
class BoxTypeInfo : public HeapTypeInfo<BoxTypeInfo> {
public:
  BoxTypeInfo(IRGenModule &IGM)
    : HeapTypeInfo(ReferenceCounting::Native, IGM.RefCountedPtrTy,
                   IGM.getPointerSize(), IGM.getHeapObjectSpareBits(),
                   IGM.getPointerAlignment())
  {}

  ReferenceCounting getReferenceCounting() const {
    // Boxes are always native-refcounted.
    return ReferenceCounting::Native;
  }

  /// Allocate a box of the given type.
  virtual OwnedAddress
  allocate(IRGenFunction &IGF, GenericEnvironment *env, SILBoxType *box,
           const toolchain::Twine &name) const = 0;

  /// Deallocate an uninitialized box.
  virtual void
  deallocate(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType) const = 0;

  /// Project the address of the contained value from a box.
  virtual Address
  project(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType) const = 0;
};

/// Common implementation for empty box type info.
class EmptyBoxTypeInfo final : public BoxTypeInfo {
public:
  EmptyBoxTypeInfo(IRGenModule &IGM) : BoxTypeInfo(IGM) {}

  OwnedAddress
  allocate(IRGenFunction &IGF, GenericEnvironment *env, SILBoxType *box,
           const toolchain::Twine &name) const override {
    auto boxedType = getSILBoxFieldType(
          IGF.IGM.getMaximalTypeExpansionContext(),
          box, IGF.IGM.getSILModule().Types, 0);
    return OwnedAddress(IGF.getTypeInfo(boxedType).getUndefAddress(),
                        IGF.emitAllocEmptyBoxCall());
  }

  void
  deallocate(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType)
  const override {
    /* Nothing to do; the box should be nil. */
  }

  Address
  project(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType)
  const override {
    return IGF.getTypeInfo(boxedType).getUndefAddress();
  }
};

/// Common implementation for non-fixed box type info.
class NonFixedBoxTypeInfo final : public BoxTypeInfo {
public:
  NonFixedBoxTypeInfo(IRGenModule &IGM) : BoxTypeInfo(IGM) {}

  OwnedAddress
  allocate(IRGenFunction &IGF, GenericEnvironment *env, SILBoxType *boxType,
           const toolchain::Twine &name) const override {
    auto boxedType = getSILBoxFieldType(
          IGF.IGM.getMaximalTypeExpansionContext(),
          boxType, IGF.IGM.getSILModule().Types, 0);
    auto &ti = IGF.getTypeInfo(boxedType);
    // Use the runtime to allocate a box of the appropriate size.
    auto metadata = IGF.emitTypeMetadataRefForLayout(boxedType);
    toolchain::Value *box, *address;
    IGF.emitAllocBoxCall(metadata, box, address);
    address = IGF.Builder.CreateBitCast(address,
                                        ti.getStorageType()->getPointerTo());
    return {ti.getAddressForPointer(address), box};
  }

  void
  deallocate(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType)
  const override {
    auto metadata = IGF.emitTypeMetadataRefForLayout(boxedType);
    IGF.emitDeallocBoxCall(box, metadata);
  }

  Address
  project(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType)
  const override {
    auto &ti = IGF.getTypeInfo(boxedType);
    auto metadata = IGF.emitTypeMetadataRefForLayout(boxedType);
    toolchain::Value *address = IGF.emitProjectBoxCall(box, metadata);
    address = IGF.Builder.CreateBitCast(address,
                                        ti.getStorageType()->getPointerTo());
    return ti.getAddressForPointer(address);
  }
};

/// Base implementation for fixed-sized boxes.
class FixedBoxTypeInfoBase : public BoxTypeInfo {
  HeapLayout layout;

public:
  FixedBoxTypeInfoBase(IRGenModule &IGM, HeapLayout &&layout)
    : BoxTypeInfo(IGM), layout(std::move(layout))
  {
    // Trivial empty layouts should always use EmptyBoxTypeInfo instead
    assert(!layout.isKnownEmpty()
           || !layout.isTriviallyDestroyable());
  }

  OwnedAddress
  allocate(IRGenFunction &IGF, GenericEnvironment *env, SILBoxType *box,
           const toolchain::Twine &name)
  const override {
    auto boxedType = getSILBoxFieldType(
          IGF.IGM.getMaximalTypeExpansionContext(),
          box, IGF.IGM.getSILModule().Types, 0);
    // Allocate a new object using the layout.
    auto boxedInterfaceType = boxedType;
    if (env) {
      boxedInterfaceType = boxedType.mapTypeOutOfContext();
    }

    {
      // FIXME: This seems wrong. We used to just mangle opened archetypes as
      // their interface type. Let's make that explicit now.
      auto astType = boxedInterfaceType.getASTType();
      astType =
          astType
              .transformRec([](Type t) -> std::optional<Type> {
                if (auto *openedExistential = t->getAs<ExistentialArchetypeType>()) {
                  auto &ctx = openedExistential->getASTContext();
                  return ctx.TheSelfType;
                }
                return std::nullopt;
              })
              ->getCanonicalType();
      boxedInterfaceType = SILType::getPrimitiveType(
          astType, boxedInterfaceType.getCategory());
    }
    
    auto boxDescriptor = IGF.IGM.getAddrOfBoxDescriptor(
        boxedInterfaceType,
        env ? env->getGenericSignature().getCanonicalSignature()
            : CanGenericSignature());
    toolchain::Value *allocation = IGF.emitUnmanagedAlloc(layout, name,
                                                     boxDescriptor);
    // Store metadata for the necessary bindings if present.
    if (layout.hasBindings()) {
      auto allocationAddr = layout.emitCastTo(IGF, allocation);
      auto bindingsAddr = layout.getElement(layout.getBindingsIndex())
        .project(IGF, allocationAddr, nullptr);
      layout.getBindings().save(IGF, bindingsAddr, box->getSubstitutions());
    }

    Address rawAddr = project(IGF, allocation, boxedType);
    return {rawAddr, allocation};
  }

  void
  deallocate(IRGenFunction &IGF, toolchain::Value *box, SILType _)
  const override {
    auto size = layout.emitSize(IGF.IGM);
    auto alignMask = layout.emitAlignMask(IGF.IGM);

    emitDeallocateUninitializedHeapObject(IGF, box, size, alignMask);
  }

  Address
  project(IRGenFunction &IGF, toolchain::Value *box, SILType boxedType)
  const override {
    Address rawAddr = layout.emitCastTo(IGF, box);
    rawAddr = layout.getElement(0).project(IGF, rawAddr, std::nullopt);
    auto &ti = IGF.getTypeInfo(boxedType);
    return IGF.Builder.CreateElementBitCast(rawAddr, ti.getStorageType());
  }
};

static HeapLayout getHeapLayoutForSingleTypeInfo(IRGenModule &IGM,
                                                 const TypeInfo &ti) {
  return HeapLayout(IGM, LayoutStrategy::Optimal, SILType(), &ti);
}

/// Common implementation for POD boxes of a known stride and alignment.
class PODBoxTypeInfo final : public FixedBoxTypeInfoBase {
public:
  PODBoxTypeInfo(IRGenModule &IGM, Size stride, Alignment alignment)
    : FixedBoxTypeInfoBase(IGM, getHeapLayoutForSingleTypeInfo(IGM,
                             IGM.getOpaqueStorageTypeInfo(stride, alignment))) {
  }
};

/// Common implementation for single-refcounted boxes.
class SingleRefcountedBoxTypeInfo final : public FixedBoxTypeInfoBase {
public:
  SingleRefcountedBoxTypeInfo(IRGenModule &IGM, ReferenceCounting refcounting)
    : FixedBoxTypeInfoBase(IGM, getHeapLayoutForSingleTypeInfo(IGM,
                                   IGM.getReferenceObjectTypeInfo(refcounting)))
  {
  }
};

/// Implementation of a box for a specific type.
class FixedBoxTypeInfo final : public FixedBoxTypeInfoBase {
  static SILType getFieldType(IRGenModule &IGM, SILBoxType *T) {
    return getSILBoxFieldType(IGM.getMaximalTypeExpansionContext(),
                              T, IGM.getSILModule().Types, 0);
  }
  
  static HeapLayout getHeapLayout(IRGenModule &IGM, SILBoxType *T) {
    SmallVector<SILType> fieldTypes;
    fieldTypes.push_back(getFieldType(IGM, T));
    
    auto bindings = NecessaryBindings::forFixedBox(IGM, T);
    unsigned bindingsIndex = 0;
    SmallVector<const TypeInfo *, 4> fields;
    fields.push_back(&IGM.getTypeInfo(fieldTypes[0]));
    
    if (!bindings.empty()) {
      bindingsIndex = 1;
      auto bindingsSize = bindings.getBufferSize(IGM);
      auto &bindingsTI = IGM.getOpaqueStorageTypeInfo(bindingsSize,
                                                    IGM.getPointerAlignment());
      fieldTypes.push_back(SILType());
      fields.push_back(&bindingsTI);
    }
    
    return HeapLayout(IGM, LayoutStrategy::Optimal,
                      fieldTypes, fields,
                      /* type to fill */ nullptr,
                      std::move(bindings), bindingsIndex);
  }

public:
  FixedBoxTypeInfo(IRGenModule &IGM, SILBoxType *T)
    : FixedBoxTypeInfoBase(IGM, getHeapLayout(IGM, T))
  {}
};

} // end anonymous namespace

NecessaryBindings
NecessaryBindings::forFixedBox(IRGenModule &IGM, SILBoxType *box) {
  // Don't need to bind metadata if the type is concrete.
  if (!box->hasArchetype() && !box->hasTypeParameter()) {
    return {};
  }

  auto fieldTy = getSILBoxFieldType(IGM.getMaximalTypeExpansionContext(),
                                    box, IGM.getSILModule().Types, 0);
  auto fieldTI = cast<FixedTypeInfo>(&IGM.getTypeInfo(fieldTy));

  // If the type is trivially destroyable, or it's fixed-layout and copyable,
  // then we can always destroy it without binding type metadata.
  if (fieldTI->isTriviallyDestroyable(ResilienceExpansion::Maximal)
      || fieldTI->isCopyable(ResilienceExpansion::Maximal)) {
    return {};
  }

  NecessaryBindings bindings(box->getSubstitutions(),
                             /*no escape*/ false);

  // Collect bindings needed by a deinit-shaped function.
  auto deinitParam = SILParameterInfo(
    box->getLayout()->getFields()[0].getLoweredType(),
    ParameterConvention::Indirect_In);
  auto deinitFnTy = SILFunctionType::get(box->getLayout()->getGenericSignature(),
                                         SILExtInfo(),
                                         SILCoroutineKind::None,
                                         ParameterConvention::Direct_Guaranteed,
                                         deinitParam,
                                         {}, {}, std::nullopt,
                                         {}, {}, IGM.Context);
  bindings.computeBindings(IGM, deinitFnTy, /*consider param sources*/ false);
  return bindings;
}

const TypeInfo *TypeConverter::convertBoxType(SILBoxType *T) {
  // We can share a type info for all dynamic-sized heap metadata.
  // TODO: Multi-field boxes
  assert(T->getLayout()->getFields().size() == 1
         && "multi-field boxes not implemented yet");
  auto eltTy = getSILBoxFieldLoweredType(
    IGM.getMaximalTypeExpansionContext(), T, IGM.getSILModule().Types, 0);
  auto &eltTI = IGM.getTypeInfoForLowered(eltTy);
  if (!eltTI.isFixedSize()) {
    if (!NonFixedBoxTI)
      NonFixedBoxTI = new NonFixedBoxTypeInfo(IGM);
    return NonFixedBoxTI;
  }

  // For fixed-sized types, we can emit concrete box metadata.
  auto &fixedTI = cast<FixedTypeInfo>(eltTI);

  // Because we assume in enums that payloads with a Builtin.NativeReference
  // which is also the type for indirect enum cases have extra inhabitants of
  // pointers we can't have a nil pointer as a representation for an empty box
  // type -- nil conflicts with the extra inhabitants. We return a static
  // singleton empty box object instead.
  //
  // (If the box needs no storage, but the type still carries a deinit,
  // then we still need to trigger that deinit when the box is freed.)
  if (fixedTI.isKnownEmpty(ResilienceExpansion::Maximal)
      && fixedTI.isTriviallyDestroyable(ResilienceExpansion::Maximal)) {
    if (!EmptyBoxTI)
      EmptyBoxTI = new EmptyBoxTypeInfo(IGM);
    return EmptyBoxTI;
  }

  // We can share box info for all similarly-shaped POD types.
  if (fixedTI.isTriviallyDestroyable(ResilienceExpansion::Maximal)
      && fixedTI.isCopyable(ResilienceExpansion::Maximal)) {
    auto stride = fixedTI.getFixedStride();
    auto align = fixedTI.getFixedAlignment();
    auto foundPOD = PODBoxTI.find({stride.getValue(),align.getValue()});
    if (foundPOD == PODBoxTI.end()) {
      auto newPOD = new PODBoxTypeInfo(IGM, stride, align);
      PODBoxTI.insert({{stride.getValue(), align.getValue()}, newPOD});
      return newPOD;
    }

    return foundPOD->second;
  }

  // We can share box info for all single-refcounted types.
  if (fixedTI.isSingleCodiraRetainablePointer(ResilienceExpansion::Maximal)) {
    if (!CodiraRetainablePointerBoxTI)
      CodiraRetainablePointerBoxTI
        = new SingleRefcountedBoxTypeInfo(IGM, ReferenceCounting::Native);
    return CodiraRetainablePointerBoxTI;
  }

  // TODO: Other common shapes? Optional-of-Refcounted would be nice.

  // Produce a tailored box metadata for the type.
  assert(T->getLayout()->getFields().size() == 1
         && "multi-field boxes not implemented yet");
  
  return new FixedBoxTypeInfo(IGM, T);
}

OwnedAddress
irgen::emitAllocateBox(IRGenFunction &IGF, CanSILBoxType boxType,
                       GenericEnvironment *env,
                       const toolchain::Twine &name) {
  auto &boxTI = IGF.getTypeInfoForLowered(boxType).as<BoxTypeInfo>();
  assert(boxType->getLayout()->getFields().size() == 1
         && "multi-field boxes not implemented yet");
  return boxTI.allocate(IGF, env, boxType, name);
}

void irgen::emitDeallocateBox(IRGenFunction &IGF,
                              toolchain::Value *box,
                              CanSILBoxType boxType) {
  auto &boxTI = IGF.getTypeInfoForLowered(boxType).as<BoxTypeInfo>();
  assert(boxType->getLayout()->getFields().size() == 1
         && "multi-field boxes not implemented yet");
  return boxTI.deallocate(
      IGF, box,
      getSILBoxFieldType(IGF.IGM.getMaximalTypeExpansionContext(), boxType,
                         IGF.IGM.getSILModule().Types, 0));
}

Address irgen::emitProjectBox(IRGenFunction &IGF,
                              toolchain::Value *box,
                              CanSILBoxType boxType) {
  auto &boxTI = IGF.getTypeInfoForLowered(boxType).as<BoxTypeInfo>();
  assert(boxType->getLayout()->getFields().size() == 1
         && "multi-field boxes not implemented yet");
  return boxTI.project(
      IGF, box,
      getSILBoxFieldType(IGF.IGM.getMaximalTypeExpansionContext(), boxType,
                         IGF.IGM.getSILModule().Types, 0));
}

Address irgen::emitAllocateExistentialBoxInBuffer(
    IRGenFunction &IGF, SILType boxedType, Address destBuffer,
    GenericEnvironment *env, const toolchain::Twine &name, bool isOutlined) {
  // Get a box for the boxed value.
  auto boxType = SILBoxType::get(boxedType.getASTType());
  auto &boxTI = IGF.getTypeInfoForLowered(boxType).as<BoxTypeInfo>();
  OwnedAddress owned = boxTI.allocate(IGF, env, boxType, name);
  Explosion box;
  box.add(owned.getOwner());
  boxTI.initialize(IGF, box,
                   Address(IGF.Builder.CreateBitCast(
                               destBuffer.getAddress(),
                               owned.getOwner()->getType()->getPointerTo()),
                           owned.getOwner()->getType(),
                           destBuffer.getAlignment()),
                   isOutlined);
  return owned.getAddress();
}

#define DEFINE_VALUE_OP(ID)                                           \
void IRGenFunction::emit##ID(toolchain::Value *value, Atomicity atomicity) { \
  if (doesNotRequireRefCounting(value)) return;                       \
  emitUnaryRefCountCall(*this, (atomicity == Atomicity::Atomic)       \
                        ? IGM.get##ID##Fn() : IGM.getNonAtomic##ID##Fn(), \
                        value);                                       \
}
#define DEFINE_ADDR_OP(ID)                                            \
void IRGenFunction::emit##ID(Address addr) {                          \
  emitUnaryRefCountCall(*this, IGM.get##ID##Fn(), addr.getAddress()); \
}
#define DEFINE_COPY_OP(ID)                                            \
void IRGenFunction::emit##ID(Address dest, Address src) {             \
  emitCopyLikeCall(*this, IGM.get##ID##Fn(), dest.getAddress(),       \
                   src.getAddress());                                 \
}
#define DEFINE_LOAD_WEAK_OP(ID)                                       \
toolchain::Value *IRGenFunction::emit##ID(Address src, toolchain::Type *type) { \
  return emitLoadWeakLikeCall(*this, IGM.get##ID##Fn(),               \
                              src.getAddress(), type);                \
}
#define DEFINE_STORE_WEAK_OP(ID)                                      \
void IRGenFunction::emit##ID(toolchain::Value *value, Address src) {       \
  emitStoreWeakLikeCall(*this, IGM.get##ID##Fn(),                     \
                        src.getAddress(), value);                     \
}

#define NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Nativeness) \
  DEFINE_LOAD_WEAK_OP(Nativeness##Name##LoadStrong) \
  DEFINE_LOAD_WEAK_OP(Nativeness##Name##TakeStrong) \
  DEFINE_STORE_WEAK_OP(Nativeness##Name##Init) \
  DEFINE_STORE_WEAK_OP(Nativeness##Name##Assign) \
  DEFINE_ADDR_OP(Nativeness##Name##Destroy) \
  DEFINE_COPY_OP(Nativeness##Name##CopyInit) \
  DEFINE_COPY_OP(Nativeness##Name##CopyAssign) \
  DEFINE_COPY_OP(Nativeness##Name##TakeInit) \
  DEFINE_COPY_OP(Nativeness##Name##TakeAssign)
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Unknown) \
  NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Native)
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  DEFINE_VALUE_OP(NativeStrongRetain##Name) \
  DEFINE_VALUE_OP(NativeStrongRetainAnd##Name##Release) \
  DEFINE_VALUE_OP(Native##Name##Release) \
  DEFINE_VALUE_OP(Native##Name##Retain)
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, Unknown) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, "...")
#include "language/AST/ReferenceStorage.def"
#undef DEFINE_VALUE_OP
#undef DEFINE_ADDR_OP
#undef DEFINE_COPY_OP
#undef DEFINE_LOAD_WEAK_OP
#undef DEFINE_STORE_WEAK_OP
#undef NEVER_OR_SOMETIMES_LOADABLE_CHECKED_REF_STORAGE_HELPER

toolchain::Value *IRGenFunction::getDynamicSelfMetadata() {
  assert(SelfValue && "no local self metadata");

  // If we already have a metatype, just return it.
  if (SelfKind == CodiraMetatype)
    return SelfValue;

  // We need to materialize a metatype. Emit the code for that once at the
  // top of the function and cache the result.

  // This is a slight optimization in the case of repeated access, but also
  // needed for correctness; when an @objc convenience initializer replaces
  // the 'self' value, we don't keep track of what the new 'self' value is
  // in IRGen, so we can't just grab the first function argument and assume
  // it's a valid 'self' at the point where DynamicSelfType metadata is needed.

  // Note that if DynamicSelfType was modeled properly as an opened archetype,
  // none of this would be an issue since it would be always be associated
  // with the correct value.

  toolchain::IRBuilderBase::InsertPointGuard guard(Builder);
  auto insertPt = isa<toolchain::Instruction>(SelfValue)
                      ? std::next(toolchain::BasicBlock::iterator(
                            cast<toolchain::Instruction>(SelfValue)))
                      : CurFn->getEntryBlock().begin();
  Builder.SetInsertPoint(&CurFn->getEntryBlock(), insertPt);
  // Do not inherit the debug location of this insertion point, it could be
  // anything (e.g. the location of a dbg.declare).
  Builder.SetCurrentDebugLocation(toolchain::DebugLoc());

  switch (SelfKind) {
  case CodiraMetatype:
    toolchain_unreachable("Already handled");
  case ObjCMetatype:
    SelfValue = emitObjCMetadataRefForMetadata(*this, SelfValue);
    SelfKind = CodiraMetatype;
    break;
  case ObjectReference:
    SelfValue = emitDynamicTypeOfHeapObject(*this, SelfValue,
                                MetatypeRepresentation::Thick,
                                SILType::getPrimitiveObjectType(SelfType),
                                /*allow artificial*/ false);
    SelfKind = CodiraMetatype;
    break;
  }

  return SelfValue;
}

/// Given a non-tagged object pointer, load a pointer to its class object.
toolchain::Value *irgen::emitLoadOfObjCHeapMetadataRef(IRGenFunction &IGF,
                                                  toolchain::Value *object) {
  if (IGF.IGM.TargetInfo.hasISAMasking()) {
    object = IGF.Builder.CreateBitCast(object,
                                       IGF.IGM.IntPtrTy->getPointerTo());
    toolchain::Value *metadata = IGF.Builder.CreateLoad(
        Address(object, IGF.IGM.IntPtrTy, IGF.IGM.getPointerAlignment()));
    toolchain::Value *mask = IGF.Builder.CreateLoad(IGF.IGM.getAddrOfObjCISAMask());
    metadata = IGF.Builder.CreateAnd(metadata, mask);
    metadata = IGF.Builder.CreateIntToPtr(metadata, IGF.IGM.TypeMetadataPtrTy);
    return metadata;
  } else if (IGF.IGM.TargetInfo.hasOpaqueISAs()) {
    return emitHeapMetadataRefForUnknownHeapObject(IGF, object);
  } else {
    object = IGF.Builder.CreateBitCast(object,
                                  IGF.IGM.TypeMetadataPtrTy->getPointerTo());
    toolchain::Value *metadata = IGF.Builder.CreateLoad(Address(
        object, IGF.IGM.TypeMetadataPtrTy, IGF.IGM.getPointerAlignment()));
    return metadata;
  }
}

/// Given a pointer to a heap object (i.e. definitely not a tagged
/// pointer), load its heap metadata pointer.
static toolchain::Value *emitLoadOfHeapMetadataRef(IRGenFunction &IGF,
                                              toolchain::Value *object,
                                              IsaEncoding isaEncoding,
                                              bool suppressCast) {
  switch (isaEncoding) {
  case IsaEncoding::Pointer: {
    toolchain::Value *slot =
        IGF.Builder.CreateBitCast(object, IGF.IGM.TypeMetadataPtrPtrTy);
    auto metadata = IGF.Builder.CreateLoad(Address(
        slot, IGF.IGM.TypeMetadataPtrTy, IGF.IGM.getPointerAlignment()));
    if (IGF.IGM.EnableValueNames && object->hasName())
      metadata->setName(toolchain::Twine(object->getName()) + ".metadata");
    return metadata;
  }
      
  case IsaEncoding::ObjC: {
    // Feed the object pointer to object_getClass.
    toolchain::Value *objcClass = emitLoadOfObjCHeapMetadataRef(IGF, object);
    objcClass = IGF.Builder.CreateBitCast(objcClass, IGF.IGM.TypeMetadataPtrTy);
    return objcClass;
  }
  }

  toolchain_unreachable("Not a valid IsaEncoding.");
}

/// Given an object of class type, produce the heap metadata reference
/// as an %objc_class*.
toolchain::Value *irgen::emitHeapMetadataRefForHeapObject(IRGenFunction &IGF,
                                                     toolchain::Value *object,
                                                     CanType objectType,
                                                     bool suppressCast) {
  ClassDecl *theClass = objectType.getClassOrBoundGenericClass();
  if ((theClass && isKnownNotTaggedPointer(IGF.IGM, theClass)) ||
      !IGF.IGM.ObjCInterop) {
    auto isaEncoding = getIsaEncodingForType(IGF.IGM, objectType);
    return emitLoadOfHeapMetadataRef(IGF, object, isaEncoding, suppressCast);
  }

  // OK, ask the runtime for the class pointer of this potentially-ObjC object.
  return emitHeapMetadataRefForUnknownHeapObject(IGF, object);
}

toolchain::Value *irgen::emitHeapMetadataRefForHeapObject(IRGenFunction &IGF,
                                                     toolchain::Value *object,
                                                     SILType objectType,
                                                     bool suppressCast) {
  return emitHeapMetadataRefForHeapObject(IGF, object,
                                          objectType.getASTType(),
                                          suppressCast);
}

/// Given an opaque class instance pointer, produce the type metadata reference
/// as a %type*.
///
/// You should only use this if you have an untyped object pointer with absolutely no type information.
/// Generally, it's better to use \c emitDynamicTypeOfHeapObject, which will
/// use the most efficient possible access pattern to get the dynamic type based on
/// the static type information.
static toolchain::Value *emitDynamicTypeOfOpaqueHeapObject(IRGenFunction &IGF,
                                                  toolchain::Value *object,
                                                  MetatypeRepresentation repr) {
  if (!IGF.IGM.ObjCInterop) {
    // Without objc interop, getting the dynamic type of an object is always
    // just a load of the isa pointer.

    assert(repr == MetatypeRepresentation::Thick
           && "objc metatypes should not occur without objc interop, "
              "and thin metadata should not be requested here");
    return emitLoadOfHeapMetadataRef(IGF, object, IsaEncoding::Pointer,
                                     /*suppressCast*/ false);
  }
  
  object = IGF.Builder.CreateBitCast(object, IGF.IGM.ObjCPtrTy);
  toolchain::CallInst *metadata;
  
  switch (repr) {
  case MetatypeRepresentation::ObjC:
    metadata = IGF.Builder.CreateCall(
        IGF.IGM.getGetObjCClassFromObjectFunctionPointer(), object);
    metadata->setName(object->getName() + ".Type");
    break;
  case MetatypeRepresentation::Thick:
    metadata = IGF.Builder.CreateCall(IGF.IGM.getGetObjectTypeFunctionPointer(),
                                      object);
    metadata->setName(object->getName() + ".Type");
    break;
  case MetatypeRepresentation::Thin:
    toolchain_unreachable("class metadata can't be thin");
  }
  
  metadata->setDoesNotThrow();
  metadata->setOnlyReadsMemory();
  return metadata;
}

toolchain::Value *irgen::
emitHeapMetadataRefForUnknownHeapObject(IRGenFunction &IGF,
                                        toolchain::Value *object) {
  object = IGF.Builder.CreateBitCast(object, IGF.IGM.ObjCPtrTy);
  auto metadata = IGF.Builder.CreateCall(
      IGF.IGM.getGetObjectClassFunctionPointer(), object);
  metadata->setName(object->getName() + ".Type");
  metadata->setCallingConv(IGF.IGM.getOptions().PlatformCCallingConvention);
  metadata->setDoesNotThrow();
  metadata->setOnlyReadsMemory();
  return metadata;
}

/// Given an object of class type, produce the type metadata reference
/// as a %type*.
toolchain::Value *irgen::emitDynamicTypeOfHeapObject(IRGenFunction &IGF,
                                                toolchain::Value *object,
                                                MetatypeRepresentation repr,
                                                SILType objectType,
                                                bool allowArtificialSubclasses){
  switch (auto isaEncoding =
            getIsaEncodingForType(IGF.IGM, objectType.getASTType())) {
  case IsaEncoding::Pointer:
    // Directly load the isa pointer from a pure Codira class.
    return emitLoadOfHeapMetadataRef(IGF, object, isaEncoding,
                                     /*suppressCast*/ false);
  case IsaEncoding::ObjC:
    // A class defined in Codira that inherits from an Objective-C class may
    // end up dynamically subclassed by ObjC runtime hackery. The artificial
    // subclass isn't a formal Codira type, so isn't appropriate as the result
    // of `type(of:)`, but is still a physical subtype of the real class object,
    // so can be used for some purposes like satisfying type parameters in
    // generic signatures.
    if (allowArtificialSubclasses
        && hasKnownCodiraMetadata(IGF.IGM, objectType.getASTType()))
      return emitLoadOfHeapMetadataRef(IGF, object, isaEncoding,
                                       /*suppressCast*/ false);
   
    // Ask the Codira runtime to find the dynamic type. This will look through
    // dynamic subclasses of Codira classes, and use the -class message for
    // ObjC classes.
    return emitDynamicTypeOfOpaqueHeapObject(IGF, object, repr);
  }
  toolchain_unreachable("unhandled ISA encoding");
}

/// What isa encoding mechanism does a type have?
IsaEncoding irgen::getIsaEncodingForType(IRGenModule &IGM,
                                         CanType type) {
  if (!IGM.ObjCInterop) return IsaEncoding::Pointer;

  // This needs to be kept up-to-date with hasKnownCodiraMetadata.

  if (auto theClass = type->getClassOrBoundGenericClass()) {
    // We can access the isas of pure Codira classes directly.
    if (!theClass->checkAncestry(AncestryFlags::ClangImported))
      return IsaEncoding::Pointer;
    // For ObjC or mixed classes, we need to use object_getClass.
    return IsaEncoding::ObjC;
  }
  
  // Existentials use the encoding of the enclosed dynamic type.
  if (type->isAnyExistentialType()) {
    return getIsaEncodingForType(
        IGM, ExistentialArchetypeType::getAny(type)->getCanonicalType());
  }

  if (auto archetype = dyn_cast<ArchetypeType>(type)) {
    // If we have a concrete superclass constraint, just recurse.
    if (auto superclass = archetype->getSuperclass()) {
      return getIsaEncodingForType(IGM, superclass->getCanonicalType());
    }

    // Otherwise, we must just have a class constraint.  Use the
    // conservative answer.
    return IsaEncoding::ObjC;
  }

  // Non-class heap objects should be pure Codira, so we can access their isas
  // directly.
  return IsaEncoding::Pointer;
}
