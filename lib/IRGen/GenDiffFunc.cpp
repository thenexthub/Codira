//===- GenDiffFunc.cpp - Codira IR Generation For @differentiable Functions ===//
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
// This file implements IR generation for `@differentiable` function types in
// Codira.
//
//===----------------------------------------------------------------------===//

#include "language/AST/Decl.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/Pattern.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILType.h"
#include "toolchain/IR/DerivedTypes.h"

#include "Explosion.h"
#include "GenHeap.h"
#include "GenRecord.h"
#include "GenType.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "IndirectTypeInfo.h"
#include "NonFixedTypeInfo.h"

#pragma clang diagnostic ignored "-Winconsistent-missing-override"

using namespace language;
using namespace irgen;

//----------------------------------------------------------------------------//
// `@differentiable` (non-linear) function type info
//----------------------------------------------------------------------------//

namespace {
class DifferentiableFuncFieldInfo final
    : public RecordField<DifferentiableFuncFieldInfo> {
public:
  DifferentiableFuncFieldInfo(
      NormalDifferentiableFunctionTypeComponent component, const TypeInfo &type,
      IndexSubset *parameterIndices, IndexSubset *resultIndices)
      : RecordField(type), component(component),
        parameterIndices(parameterIndices), resultIndices(resultIndices) {}

  /// The field index.
  const NormalDifferentiableFunctionTypeComponent component;

  /// The parameter indices.
  IndexSubset *parameterIndices;
  /// The result indices.
  IndexSubset *resultIndices;

  std::string getFieldName() const {
    switch (component) {
    case NormalDifferentiableFunctionTypeComponent::Original:
      return "original";
    case NormalDifferentiableFunctionTypeComponent::JVP:
      return "jvp";
    case NormalDifferentiableFunctionTypeComponent::VJP:
      return "vjp";
    }
    toolchain_unreachable("invalid component type");
  }

  SILType getType(IRGenModule &IGM, SILType t) const {
    auto fnTy = t.castTo<SILFunctionType>();
    auto origFnTy = fnTy->getWithoutDifferentiability();
    if (component == NormalDifferentiableFunctionTypeComponent::Original)
      return SILType::getPrimitiveObjectType(origFnTy);
    auto kind = *component.getAsDerivativeFunctionKind();
    auto assocTy = origFnTy->getAutoDiffDerivativeFunctionType(
        parameterIndices, resultIndices, kind, IGM.getSILTypes(),
        LookUpConformanceInModule());
    return SILType::getPrimitiveObjectType(assocTy);
  }
};

class DifferentiableFuncTypeInfo final
    : public RecordTypeInfo<DifferentiableFuncTypeInfo, LoadableTypeInfo,
                            DifferentiableFuncFieldInfo> {
  using super = RecordTypeInfo<DifferentiableFuncTypeInfo, LoadableTypeInfo,
                               DifferentiableFuncFieldInfo>;

public:
  DifferentiableFuncTypeInfo(ArrayRef<DifferentiableFuncFieldInfo> fields,
                             unsigned explosionSize, toolchain::Type *ty, Size size,
                             SpareBitVector &&spareBits, Alignment align,
                             IsTriviallyDestroyable_t isTriviallyDestroyable, IsFixedSize_t alwaysFixedSize)
      : super(fields, explosionSize, FieldsAreABIAccessible, ty, size, std::move(spareBits), align,
              isTriviallyDestroyable, IsCopyable, alwaysFixedSize, IsABIAccessible) {}

  Address projectFieldAddress(IRGenFunction &IGF, Address addr, SILType T,
                              const DifferentiableFuncFieldInfo &field) const {
    return field.projectAddress(IGF, addr, getNonFixedOffsets(IGF, T));
  }

  void initializeFromParams(IRGenFunction &IGF, Explosion &params, Address src,
                            SILType T, bool isOutlined) const override {
    toolchain_unreachable("unexploded @differentiable function as argument?");
  }

  void addToAggLowering(IRGenModule &IGM, CodiraAggLowering &lowering,
                        Size offset) const override {
    for (auto &field : getFields()) {
      auto fieldOffset = offset + field.getFixedByteOffset();
      cast<LoadableTypeInfo>(field.getTypeInfo())
          .addToAggLowering(IGM, lowering, fieldOffset);
    }
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts || !areFieldsABIAccessible()) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }

    if (getFields().empty()) {
      return IGM.typeLayoutCache.getEmptyEntry();
    }

    std::vector<TypeLayoutEntry *> fields;
    for (auto &field : getFields()) {
      auto fieldTy = field.getType(IGM, T);
      fields.push_back(field.getTypeInfo().buildTypeLayoutEntry(IGM, fieldTy, useStructLayouts));
    }

    // if (fields.size() == 1) {
    //   return fields[0];
    // }

    return IGM.typeLayoutCache.getOrCreateAlignedGroupEntry(
        fields, T, getBestKnownAlignment().getValue(), *this);
  }

  std::nullopt_t getNonFixedOffsets(IRGenFunction &IGF) const {
    return std::nullopt;
  }
  std::nullopt_t getNonFixedOffsets(IRGenFunction &IGF, SILType T) const {
    return std::nullopt;
  }
};

class DifferentiableFuncTypeBuilder
    : public RecordTypeBuilder<DifferentiableFuncTypeBuilder,
                               DifferentiableFuncFieldInfo,
                               NormalDifferentiableFunctionTypeComponent> {

  SILFunctionType *originalType;
  IndexSubset *parameterIndices;
  IndexSubset *resultIndices;

public:
  DifferentiableFuncTypeBuilder(IRGenModule &IGM, SILFunctionType *fnTy)
      : RecordTypeBuilder(IGM),
        originalType(fnTy->getWithoutDifferentiability()),
        parameterIndices(fnTy->getDifferentiabilityParameterIndices()),
        resultIndices(fnTy->getDifferentiabilityResultIndices()) {
    // TODO: Ban 'Normal' and 'Forward'.
    assert(fnTy->getDifferentiabilityKind() == DifferentiabilityKind::Reverse ||
           fnTy->getDifferentiabilityKind() == DifferentiabilityKind::Normal ||
           fnTy->getDifferentiabilityKind() == DifferentiabilityKind::Forward);
  }

  TypeInfo *createFixed(ArrayRef<DifferentiableFuncFieldInfo> fields,
                        FieldsAreABIAccessible_t unused,
                        StructLayout &&layout) {
    toolchain_unreachable("@differentiable functions are always loadable");
  }

  DifferentiableFuncTypeInfo *
  createLoadable(ArrayRef<DifferentiableFuncFieldInfo> fields,
                 FieldsAreABIAccessible_t unused,
                 StructLayout &&layout, unsigned explosionSize) {
    return DifferentiableFuncTypeInfo::create(
        fields, explosionSize, layout.getType(), layout.getSize(),
        std::move(layout.getSpareBits()), layout.getAlignment(), layout.isTriviallyDestroyable(),
        layout.isAlwaysFixedSize());
  }

  TypeInfo *createNonFixed(ArrayRef<DifferentiableFuncFieldInfo> fields,
                           FieldsAreABIAccessible_t fieldsAccessible,
                           StructLayout &&layout) {
    toolchain_unreachable("@differentiable functions are always loadable");
  }

  DifferentiableFuncFieldInfo
  getFieldInfo(unsigned index,
               NormalDifferentiableFunctionTypeComponent component,
               const TypeInfo &fieldTI) {
    return DifferentiableFuncFieldInfo(component, fieldTI, parameterIndices,
                                       resultIndices);
  }

  SILType getType(NormalDifferentiableFunctionTypeComponent component) {
    if (component == NormalDifferentiableFunctionTypeComponent::Original)
      return SILType::getPrimitiveObjectType(originalType->getCanonicalType());
    auto kind = *component.getAsDerivativeFunctionKind();
    auto assocTy = originalType->getAutoDiffDerivativeFunctionType(
        parameterIndices, resultIndices, kind, IGM.getSILTypes(),
        LookUpConformanceInModule());
    return SILType::getPrimitiveObjectType(assocTy);
  }

  StructLayout performLayout(ArrayRef<const TypeInfo *> fieldTypes) {
    return StructLayout(IGM, /*type=*/std::nullopt, LayoutKind::NonHeapObject,
                        LayoutStrategy::Universal, fieldTypes);
  }
};
} // end anonymous namespace

//----------------------------------------------------------------------------//
// `@differentiable(_linear)` function type info
//----------------------------------------------------------------------------//

namespace {
class LinearFuncFieldInfo final : public RecordField<LinearFuncFieldInfo> {
public:
  LinearFuncFieldInfo(LinearDifferentiableFunctionTypeComponent component,
                      const TypeInfo &type, IndexSubset *parameterIndices)
      : RecordField(type), component(component),
        parameterIndices(parameterIndices) {}

  /// The field index.
  const LinearDifferentiableFunctionTypeComponent component;

  /// The parameter indices.
  IndexSubset *parameterIndices;

  std::string getFieldName() const {
    switch (component) {
    case LinearDifferentiableFunctionTypeComponent::Original:
      return "original";
    case LinearDifferentiableFunctionTypeComponent::Transpose:
      return "transpose";
    }
    toolchain_unreachable("invalid component type");
  }

  SILType getType(IRGenModule &IGM, SILType t) const {
    auto fnTy = t.castTo<SILFunctionType>();
    auto origFnTy = fnTy->getWithoutDifferentiability();
    switch (component) {
    case LinearDifferentiableFunctionTypeComponent::Original:
      return SILType::getPrimitiveObjectType(origFnTy);
    case LinearDifferentiableFunctionTypeComponent::Transpose:
      auto transposeTy = origFnTy->getAutoDiffTransposeFunctionType(
          parameterIndices, IGM.getSILTypes(),
          LookUpConformanceInModule());
      return SILType::getPrimitiveObjectType(transposeTy);
    }
    toolchain_unreachable("invalid component type");
  }
};

class LinearFuncTypeInfo final
    : public RecordTypeInfo<LinearFuncTypeInfo, LoadableTypeInfo,
                            LinearFuncFieldInfo> {
  using super =
      RecordTypeInfo<LinearFuncTypeInfo, LoadableTypeInfo, LinearFuncFieldInfo>;

public:
  LinearFuncTypeInfo(ArrayRef<LinearFuncFieldInfo> fields,
                     unsigned explosionSize, toolchain::Type *ty, Size size,
                     SpareBitVector &&spareBits, Alignment align, IsTriviallyDestroyable_t isTriviallyDestroyable,
                     IsFixedSize_t alwaysFixedSize)
      : super(fields, explosionSize, FieldsAreABIAccessible, ty, size, std::move(spareBits), align,
              isTriviallyDestroyable, IsCopyable, alwaysFixedSize, IsABIAccessible) {}

  Address projectFieldAddress(IRGenFunction &IGF, Address addr, SILType T,
                              const LinearFuncFieldInfo &field) const {
    return field.projectAddress(IGF, addr, getNonFixedOffsets(IGF, T));
  }

  void initializeFromParams(IRGenFunction &IGF, Explosion &params, Address src,
                            SILType T, bool isOutlined) const override {
    toolchain_unreachable("unexploded @differentiable function as argument?");
  }

  void addToAggLowering(IRGenModule &IGM, CodiraAggLowering &lowering,
                        Size offset) const override {
    for (auto &field : getFields()) {
      auto fieldOffset = offset + field.getFixedByteOffset();
      cast<LoadableTypeInfo>(field.getTypeInfo())
          .addToAggLowering(IGM, lowering, fieldOffset);
    }
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts || !areFieldsABIAccessible()) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }

    if (getFields().empty()) {
      return IGM.typeLayoutCache.getEmptyEntry();
    }

    std::vector<TypeLayoutEntry *> fields;
    for (auto &field : getFields()) {
      auto fieldTy = field.getType(IGM, T);
      fields.push_back(field.getTypeInfo().buildTypeLayoutEntry(IGM, fieldTy, useStructLayouts));
    }

    // if (fields.size() == 1) {
    //   return fields[0];
    // }

    return IGM.typeLayoutCache.getOrCreateAlignedGroupEntry(
        fields, T, getBestKnownAlignment().getValue(), *this);
  }

  std::nullopt_t getNonFixedOffsets(IRGenFunction &IGF) const {
    return std::nullopt;
  }
  std::nullopt_t getNonFixedOffsets(IRGenFunction &IGF, SILType T) const {
    return std::nullopt;
  }
};

class LinearFuncTypeBuilder
    : public RecordTypeBuilder<LinearFuncTypeBuilder, LinearFuncFieldInfo,
                               LinearDifferentiableFunctionTypeComponent> {

  SILFunctionType *originalType;
  IndexSubset *parameterIndices;

public:
  LinearFuncTypeBuilder(IRGenModule &IGM, SILFunctionType *fnTy)
      : RecordTypeBuilder(IGM),
        originalType(fnTy->getWithoutDifferentiability()),
        parameterIndices(fnTy->getDifferentiabilityParameterIndices()) {
    assert(fnTy->getDifferentiabilityKind() == DifferentiabilityKind::Linear);
  }

  TypeInfo *createFixed(ArrayRef<LinearFuncFieldInfo> fields,
                        FieldsAreABIAccessible_t areFieldsABIAccessible,
                        StructLayout &&layout) {
    toolchain_unreachable("@differentiable functions are always loadable");
  }

  LinearFuncTypeInfo *createLoadable(ArrayRef<LinearFuncFieldInfo> fields,
                                     FieldsAreABIAccessible_t unused,
                                     StructLayout &&layout,
                                     unsigned explosionSize) {
    return LinearFuncTypeInfo::create(
        fields, explosionSize, layout.getType(), layout.getSize(),
        std::move(layout.getSpareBits()), layout.getAlignment(), layout.isTriviallyDestroyable(),
        layout.isAlwaysFixedSize());
  }

  TypeInfo *createNonFixed(ArrayRef<LinearFuncFieldInfo> fields,
                           FieldsAreABIAccessible_t fieldsAccessible,
                           StructLayout &&layout) {
    toolchain_unreachable("@differentiable functions are always loadable");
  }

  LinearFuncFieldInfo
  getFieldInfo(unsigned index, LinearDifferentiableFunctionTypeComponent field,
               const TypeInfo &fieldTI) {
    return LinearFuncFieldInfo(field, fieldTI, parameterIndices);
  }

  SILType getType(LinearDifferentiableFunctionTypeComponent component) {
    switch (component) {
    case LinearDifferentiableFunctionTypeComponent::Original:
      return SILType::getPrimitiveObjectType(originalType->getCanonicalType());
    case LinearDifferentiableFunctionTypeComponent::Transpose:
      auto transposeTy = originalType->getAutoDiffTransposeFunctionType(
          parameterIndices, IGM.getSILTypes(),
          LookUpConformanceInModule());
      return SILType::getPrimitiveObjectType(transposeTy);
    }
    toolchain_unreachable("invalid component type");
  }

  StructLayout performLayout(ArrayRef<const TypeInfo *> fieldTypes) {
    return StructLayout(IGM, /*type=*/std::nullopt, LayoutKind::NonHeapObject,
                        LayoutStrategy::Universal, fieldTypes);
  }
};
} // end anonymous namespace

//----------------------------------------------------------------------------//
// Type converter entry points
//----------------------------------------------------------------------------//

const TypeInfo *
TypeConverter::convertNormalDifferentiableFunctionType(SILFunctionType *type) {
  DifferentiableFuncTypeBuilder builder(IGM, type);
  return builder.layout({NormalDifferentiableFunctionTypeComponent::Original,
                         NormalDifferentiableFunctionTypeComponent::JVP,
                         NormalDifferentiableFunctionTypeComponent::VJP});
}

const TypeInfo *
TypeConverter::convertLinearDifferentiableFunctionType(SILFunctionType *type) {
  LinearFuncTypeBuilder builder(IGM, type);
  return builder.layout({LinearDifferentiableFunctionTypeComponent::Original,
                         LinearDifferentiableFunctionTypeComponent::Transpose});
}
