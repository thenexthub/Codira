//===--- GenType.h - Auxiliary Interface for Type IR Generation -*- C++ -*-===//
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
// This file defines the private interface used for turning AST types
// into LLVM IR types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENTYPE_H
#define LANGUAGE_IRGEN_GENTYPE_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/ilist.h"
#include "toolchain/ADT/ilist_node.h"
#include "toolchain/ADT/StringMap.h"
#include "IRGenModule.h"
#include "IRGenFunction.h"
#include "LegacyLayoutFormat.h"

namespace toolchain {
  namespace vfs {
    class FileSystem;
  }
}

namespace language {
  class ArchetypeType;
  class CanType;
  class ClassDecl;
  class AnyFunctionType;
  class InOutType;
  class MetatypeType;
  class ModuleType;
  class NominalTypeDecl;
  class EnumDecl;
  class ProtocolCompositionType;
  class ProtocolDecl;
  class ProtocolType;
  class SILFunctionType;
  class StructDecl;
  class TupleType;
  class TypeBase;
  class Type;
  class EnumDecl;
  class UnownedStorageType;
  class WeakStorageType;
  enum IsTake_t : bool;
  
namespace irgen {
  class Alignment;
  class GenericContextScope;
  class ProtocolInfo;
  class Size;
  class FixedTypeInfo;
  class LoadableTypeInfo;
  class TypeInfo;
  
/// The helper class for generating types.
class TypeConverter {
public:
  enum class Mode : unsigned {
    /// Normal type lowering mode where resilient types are opaque.
    Normal,

    /// Used for computing backward deployment class layouts, where we emit a
    /// static class metadata layout using known sizes and alignments of any
    /// resiliently-typed fields from a previous Codira version. On newer Codira
    /// versions we use a runtime mechanism to re-initialize the class metadata
    /// in-place with the current known layout.
    Legacy,

    /// A temporary hack for lldb where all resilient types are transparent and
    /// treated like fixed-size (but still lowered in a way that matches the
    /// runtime layout produced for resilient types, which is important for some
    /// types like enums where enabling resilience changes the layout).
    CompletelyFragile

    /// When adding or removing fields, remember to update NumLoweringModes below.
  };

  static unsigned const NumLoweringModes = 3;

  IRGenModule &IGM;
private:
  // Set using the GenericContextScope RAII object.
  friend GenericContextScope;
  CanGenericSignature CurGenericSignature;
  // Enter a generic context for lowering the parameters of a generic function
  // type.
  void setGenericContext(CanGenericSignature signature);

  Mode LoweringMode = Mode::Normal;

  toolchain::DenseMap<ProtocolDecl*, std::unique_ptr<const ProtocolInfo>> Protocols;
  const TypeInfo *FirstType;
  
  const LoadableTypeInfo *NativeObjectTI = nullptr;
  const LoadableTypeInfo *UnknownObjectTI = nullptr;
  const LoadableTypeInfo *BridgeObjectTI = nullptr;
  const LoadableTypeInfo *RawPointerTI = nullptr;
  const LoadableTypeInfo *RawUnsafeContinuationTI = nullptr;
  const LoadableTypeInfo *JobTI = nullptr;
  const LoadableTypeInfo *ExecutorTI = nullptr;
  const LoadableTypeInfo *WitnessTablePtrTI = nullptr;
  const TypeInfo *TypeMetadataPtrTI = nullptr;
  const TypeInfo *CodiraContextPtrTI = nullptr;
  const TypeInfo *TaskContinuationFunctionPtrTI = nullptr;
  const TypeInfo *ObjCClassPtrTI = nullptr;
  const LoadableTypeInfo *EmptyTI = nullptr;
  const LoadableTypeInfo *IntegerLiteralTI = nullptr;

  const TypeInfo *ResilientStructTI[2][2] = {
    {nullptr, nullptr},
    {nullptr, nullptr},
  };

  const TypeInfo *DynamicTupleTI[2] = {nullptr, nullptr};
  
  toolchain::DenseMap<std::pair<unsigned, unsigned>, const LoadableTypeInfo *>
    OpaqueStorageTypes;

  const LoadableTypeInfo *NonFixedBoxTI = nullptr;
  const LoadableTypeInfo *EmptyBoxTI = nullptr;
  toolchain::DenseMap<std::pair<unsigned, unsigned>, const LoadableTypeInfo *>
    PODBoxTI;
  const LoadableTypeInfo *CodiraRetainablePointerBoxTI = nullptr,
                         *UnknownObjectRetainablePointerBoxTI = nullptr;

  toolchain::StringMap<YAMLTypeInfoNode> LegacyTypeInfos;
  toolchain::DenseMap<NominalTypeDecl *, std::string> DeclMangledNames;

  /// The key is the number of witness tables.
  toolchain::DenseMap<unsigned, toolchain::StructType *> OpaqueExistentialTypes;

  const LoadableTypeInfo *createPrimitive(toolchain::Type *T,
                                          Size size, Alignment align);
  const FixedTypeInfo *createImmovable(toolchain::Type *T,
                                       Size size, Alignment align);
  const TypeInfo *createOpaqueImmovable(toolchain::Type *T, Alignment minAlign);

  void addForwardDecl(TypeBase *key);

  const TypeInfo *convertType(CanType T);
  const TypeInfo *convertAnyNominalType(CanType T, NominalTypeDecl *D);
  const TypeInfo *convertTupleType(TupleType *T);
  const TypeInfo *convertClassType(CanType type, ClassDecl *D);
  const TypeInfo *convertEnumType(TypeBase *key, CanType type, EnumDecl *D);
  const TypeInfo *convertStructType(TypeBase *key, CanType type, StructDecl *D);
  const TypeInfo *convertFunctionType(SILFunctionType *T);
  const TypeInfo *convertNormalDifferentiableFunctionType(SILFunctionType *T);
  const TypeInfo *convertLinearDifferentiableFunctionType(SILFunctionType *T);
  const TypeInfo *convertBlockStorageType(SILBlockStorageType *T);
  const TypeInfo *convertBoxType(SILBoxType *T);
  const TypeInfo *convertArchetypeType(ArchetypeType *T);
  const TypeInfo *convertInOutType(InOutType *T);
  const TypeInfo *convertSILMoveOnlyWrappedType(SILMoveOnlyWrappedType *T) {
    return convertType(T->getInnerType());
  }
  const TypeInfo *convertExistentialMetatypeType(ExistentialMetatypeType *T);
  const TypeInfo *convertMetatypeType(MetatypeType *T);
  const TypeInfo *convertModuleType(ModuleType *T);
  const TypeInfo *convertProtocolType(ProtocolType *T);
  const TypeInfo *convertProtocolCompositionType(ProtocolCompositionType *T);
  const TypeInfo *convertParameterizedProtocolType(ParameterizedProtocolType *T);
  const TypeInfo *convertExistentialType(ExistentialType *T);
  const TypeInfo *convertPackType(SILPackType *T);
  const LoadableTypeInfo *convertBuiltinNativeObject();
  const LoadableTypeInfo *convertBuiltinUnknownObject();
  const LoadableTypeInfo *convertBuiltinBridgeObject();
  const TypeInfo *convertResilientStruct(IsCopyable_t copyable,
                                         IsABIAccessible_t abiAccessible);
  const TypeInfo *convertDynamicTupleType(IsCopyable_t copyable);
#define REF_STORAGE(Name, ...) \
  const TypeInfo *convert##Name##StorageType(Name##StorageType *T);
#include "language/AST/ReferenceStorage.def"
  const TypeInfo *convertBuiltinFixedArrayType(BuiltinFixedArrayType *T);


public:
  TypeConverter(IRGenModule &IGM);
  ~TypeConverter();

  Mode getLoweringMode() const {
    return LoweringMode;
  }

  const TypeInfo *getTypeEntry(CanType type);
  const TypeInfo &getCompleteTypeInfo(CanType type);

  const TypeLayoutEntry
  &getTypeLayoutEntry(SILType T, bool useStructLayouts);
  const LoadableTypeInfo &getNativeObjectTypeInfo();
  const LoadableTypeInfo &getUnknownObjectTypeInfo();
  const LoadableTypeInfo &getBridgeObjectTypeInfo();
  const LoadableTypeInfo &getRawPointerTypeInfo();
  const LoadableTypeInfo &getRawUnsafeContinuationTypeInfo();
  const LoadableTypeInfo &getJobTypeInfo();
  const LoadableTypeInfo &getExecutorTypeInfo();
  const TypeInfo &getTypeMetadataPtrTypeInfo();
  const TypeInfo &getCodiraContextPtrTypeInfo();
  const TypeInfo &getTaskContinuationFunctionPtrTypeInfo();
  const TypeInfo &getObjCClassPtrTypeInfo();
  const LoadableTypeInfo &getWitnessTablePtrTypeInfo();
  const LoadableTypeInfo &getEmptyTypeInfo();
  const LoadableTypeInfo &getIntegerLiteralTypeInfo();
  const TypeInfo &getResilientStructTypeInfo(IsCopyable_t copyable,
                                             IsABIAccessible_t abiAccessible);
  const TypeInfo &getDynamicTupleTypeInfo(IsCopyable_t isCopyable);
  const ProtocolInfo &getProtocolInfo(ProtocolDecl *P, ProtocolInfoKind kind);
  const LoadableTypeInfo &getOpaqueStorageTypeInfo(Size storageSize,
                                                   Alignment storageAlign);
  const TypeInfo &getMetatypeTypeInfo(MetatypeRepresentation representation);

#define REF_STORAGE(Name, ...) \
  const TypeInfo *create##Name##StorageType(toolchain::Type *valueType, \
                                            ReferenceCounting style, \
                                            bool isOptional);
#include "language/AST/ReferenceStorage.def"

  toolchain::Type *getExistentialType(unsigned numWitnessTables);

  /// Retrieve the generic signature for the current generic context, or null if no
  /// generic environment is active.
  CanGenericSignature getCurGenericContext() { return CurGenericSignature; }
  
  /// Retrieve the generic environment for the current generic context.
  ///
  /// Fails if there is no generic context.
  GenericEnvironment *getGenericEnvironment();

private:
  friend class LoweringModeScope;

  void setLoweringMode(Mode mode) {
    LoweringMode = mode;
  }

  /// Read a YAML legacy type layout dump. Returns false on success, true on
  /// error.
  bool readLegacyTypeInfo(toolchain::vfs::FileSystem &fs, StringRef path);

  std::optional<YAMLTypeInfoNode>
  getLegacyTypeInfo(NominalTypeDecl *decl) const;

  // Debugging aids.
#ifndef NDEBUG
  bool isExemplarArchetype(ArchetypeType *arch) const;
#endif

  ArchetypeType *getExemplarArchetype(ArchetypeType *t);
  CanType getExemplarType(CanType t);
  
  class Types_t {
    toolchain::DenseMap<TypeBase *, const TypeInfo *> IndependentCache[NumLoweringModes];
    toolchain::DenseMap<TypeBase *, const TypeInfo *> DependentCache[NumLoweringModes];

    toolchain::DenseMap<TypeBase *, const TypeLayoutEntry *>
        IndependentTypeLayoutCache[NumLoweringModes];
    toolchain::DenseMap<TypeBase *, const TypeLayoutEntry *>
        DependentTypeLayoutCache[NumLoweringModes];

  public:
    toolchain::DenseMap<TypeBase *, const TypeInfo *> &getCacheFor(bool isDependent,
                                                              Mode mode);
    toolchain::DenseMap<TypeBase *, const TypeLayoutEntry *> &
    getTypeLayoutCacheFor(bool isDependent, Mode mode);
  };
  Types_t Types;
};

/// An RAII interface for entering a generic context for type conversion in
/// a scope.
class GenericContextScope {
  TypeConverter &TC;
  CanGenericSignature newSig, oldSig;
public:
  GenericContextScope(TypeConverter &TC, CanGenericSignature sig)
    : TC(TC), newSig(sig), oldSig(TC.CurGenericSignature)
  {
    TC.setGenericContext(newSig);
  }
  
  GenericContextScope(IRGenModule &IGM, CanGenericSignature sig)
    : GenericContextScope(IGM.Types, sig)
  {}
  
  ~GenericContextScope() {
    assert(TC.CurGenericSignature == newSig);
    TC.setGenericContext(oldSig);
  }
};

/// An RAII interface for forcing types to be lowered bypassing resilience.
class LoweringModeScope {
  TypeConverter::Mode OldLoweringMode;
  TypeConverter &TC;
public:
  LoweringModeScope(TypeConverter &TC, TypeConverter::Mode LoweringMode)
      : TC(TC) {
    OldLoweringMode = TC.getLoweringMode();
    TC.setLoweringMode(LoweringMode);
  }

  LoweringModeScope(IRGenModule &IGM, TypeConverter::Mode LoweringMode)
      : LoweringModeScope(IGM.Types, LoweringMode) {}

  ~LoweringModeScope() {
    TC.setLoweringMode(OldLoweringMode);
  }
};

/// If a type is visibly a singleton aggregate (a tuple with one element, a
/// struct with one field, or an enum with a single payload case), return the
/// type of its field, which it is guaranteed to have identical layout to.
///
/// This can use more concrete type layout information than
/// SILType::getSingletonAggregateFieldType, because we have full access to the
/// LLVM-level layout of types in IRGen.
SILType getSingletonAggregateFieldType(IRGenModule &IGM,
                                       SILType t,
                                       ResilienceExpansion expansion);

/// An IRGenFunction interface for generating type layout verifiers.
class IRGenTypeVerifierFunction : public IRGenFunction {
private:
  FunctionPointer VerifierFn;

  struct VerifierArgumentBuffers {
    Address runtimeBuf, staticBuf;
  };
  toolchain::DenseMap<toolchain::Type *, VerifierArgumentBuffers> VerifierArgBufs;

public:
  IRGenTypeVerifierFunction(IRGenModule &IGM, toolchain::Function *f);
  
  void emit(ArrayRef<CanType> typesToVerify);
  
  /// Call a runtime function that verifies that the two LLVM values are
  /// equivalent, logging a detailed error if they differ.
  void verifyValues(toolchain::Value *typeMetadata,
                    toolchain::Value *runtimeValue,
                    toolchain::Value *compilerValue,
                    const toolchain::Twine &description);
  
  /// Call a runtime function that verifies that the contents of the two
  /// memory buffers are equivalent, logging a detailed error if they differ.
  void verifyBuffers(toolchain::Value *typeMetadata,
                     Address runtimeValue,
                     Address compilerValue,
                     Size size,
                     const toolchain::Twine &description);
};

template <class FixedTypeInfoType>
TypeLayoutEntry *buildTypeLayoutEntryForFields(IRGenModule &IGM, SILType T,
                                               const FixedTypeInfoType &TI) {
  std::vector<TypeLayoutEntry *> fields;

  auto minimumAlignment = TI.getFixedAlignment().getValue();
  Alignment::int_type minFieldAlignment = 1;
  for (auto &field : TI.getFields()) {
    auto fieldTy = field.getType(IGM, T);
    auto fieldAlignment = cast<FixedTypeInfo>(field.getTypeInfo())
                              .getFixedAlignment()
                              .getValue();
    if (minFieldAlignment < fieldAlignment)
      minFieldAlignment = fieldAlignment;
    fields.push_back(field.getTypeInfo().buildTypeLayoutEntry(IGM, fieldTy));
  }

  if (fields.empty() && minFieldAlignment >= minimumAlignment) {
    return IGM.typeLayoutCache.getEmptyEntry();
  }

  // if (fields.size() == 1 && minFieldAlignment >= minimumAlignment) {
  //   return fields[0];
  // }
  if (minimumAlignment < minFieldAlignment)
    minimumAlignment = minFieldAlignment;
  return IGM.typeLayoutCache.getOrCreateAlignedGroupEntry(
      fields, minimumAlignment, true);
}

/// Emit a call to the deinit for T to destroy the value at the given address,
/// if a deinit is available.
///
/// Returns true if the deinit call was emitted, or false if there is no deinit.
/// No code emission occurs if the function returns false.
bool tryEmitDestroyUsingDeinit(IRGenFunction &IGF,
                               Address address,
                               SILType T);
                               
/// Emit a call to the deinit for T to destroy the value in the given explosion,
/// if a deinit is available.
///
/// Returns true if the deinit call was emitted, or false if there is no deinit.
/// No code emission occurs if the function returns false.
bool tryEmitConsumeUsingDeinit(IRGenFunction &IGF,
                               Explosion &explosion,
                               SILType T);

/// Most fixed size types currently are always ABI accessible (value operations
/// can be done without metadata). One notable exception is non-copyable types
/// with a deinit. Their type metadata is required to call destroy if the deinit
/// function is not available to the current SIL module.
IsABIAccessible_t isTypeABIAccessibleIfFixedSize(IRGenModule &IGM, CanType ty);

} // end namespace irgen
} // end namespace language

#endif
