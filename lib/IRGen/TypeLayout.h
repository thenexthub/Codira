//===------------- TypeLayout.h ---------------  Type layouts ---*- C++ -*-===//
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

#ifndef LANGUAGE_IRGEN_TYPE_LAYOUT_H
#define LANGUAGE_IRGEN_TYPE_LAYOUT_H

#include "FixedTypeInfo.h"
#include "TypeInfo.h"
#include "language/SIL/SILType.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/Debug.h"

namespace language {
namespace irgen {

class EnumTypeLayoutEntry;
class LayoutStringBuilder;

enum class TypeLayoutEntryKind : uint8_t {
  Empty,
  Scalar,
  Archetype,
  AlignedGroup,
  Resilient,
  Enum,
  TypeInfoBased,
  Array,
};

enum class ScalarKind : uint8_t {
  TriviallyDestroyable,
  Immovable,
  ErrorReference,
  NativeStrongReference,
  NativeUnownedReference,
  NativeWeakReference,
  UnknownReference,
  UnknownUnownedReference,
  UnknownWeakReference,
  BlockReference,
  BridgeReference,
  ObjCReference,
  BlockStorage,
  ThickFunc,
  ExistentialReference,
  CustomReference,
};

/// Convert a ReferenceCounting into the appropriate Scalar reference
ScalarKind refcountingToScalarKind(ReferenceCounting refCounting);

class TypeLayoutEntry {
protected:
  /// Memoize the value of layoutString()
  /// None -> Not yet computed
  /// Optional(nullptr) -> No layout string
  /// Optional(Constant*) -> Layout string
  mutable std::optional<toolchain::Constant *> _layoutString;

public:
  TypeLayoutEntryKind kind;
  uint8_t hasArchetypeField : 1;
  uint8_t hasResilientField : 1;
  uint8_t hasDependentResilientField : 1;

  TypeLayoutEntry()
      : kind(TypeLayoutEntryKind::Empty), hasArchetypeField(false),
        hasResilientField(false), hasDependentResilientField(false) {}

  TypeLayoutEntry(TypeLayoutEntryKind kind)
      : kind(kind), hasArchetypeField(false), hasResilientField(false),
        hasDependentResilientField(false) {}

  virtual ~TypeLayoutEntry();

  virtual void computeProperties();

  bool containsResilientField() const;
  bool containsArchetypeField() const;
  bool containsDependentResilientField() const;


  bool isEmpty() const { return kind == TypeLayoutEntryKind::Empty; }

  TypeLayoutEntryKind getKind() const { return kind; }

  virtual toolchain::Value *alignmentMask(IRGenFunction &IGF) const;
  virtual toolchain::Value *size(IRGenFunction &IGF) const;

  /// Return the size of the type if known statically
  virtual std::optional<Size> fixedSize(IRGenModule &IGM) const;

  /// Return if the type and its subtypes are trivially destructible.
  virtual bool isTriviallyDestroyable() const;
  virtual bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                                   unsigned index) const;
  virtual bool isSingleRetainablePointer() const;

  /// Return if the size of the type is known statically
  virtual bool isFixedSize(IRGenModule &IGM) const;

  /// Return the alignment of the type if known statically
  virtual std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const;

  /// Return the number of extra inhabitants if known statically
  virtual std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const;
  virtual toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const;
  virtual toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const;
  virtual toolchain::Constant *layoutString(IRGenModule &IGM,
                                       GenericSignature genericSig) const;
  virtual bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                              GenericSignature genericSig) const;

  virtual void destroy(IRGenFunction &IGF, Address addr) const;

  void assign(IRGenFunction &IGF, Address dest, Address src,
              IsTake_t isTake) const;
  void initialize(IRGenFunction &IGF, Address dest, Address src,
                  IsTake_t isTake) const;

  virtual void assignWithCopy(IRGenFunction &IGF, Address dest,
                              Address src) const;
  virtual void assignWithTake(IRGenFunction &IGF, Address dest,
                              Address src) const;

  virtual void initWithCopy(IRGenFunction &IGF, Address dest,
                              Address src) const;
  virtual void initWithTake(IRGenFunction &IGF, Address dest,
                            Address src) const;

  /// Returns a pointer to the object (T*) inside of the buffer.
  virtual toolchain::Value *initBufferWithCopyOfBuffer(IRGenFunction &IGF,
                                                  Address dest,
                                                  Address src) const;

  virtual toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                               toolchain::Value *numEmptyCases,
                                               Address addr) const;

  virtual void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                         toolchain::Value *numEmptyCases,
                                         Address enumAddr) const;

  const EnumTypeLayoutEntry *getAsEnum() const;

  bool isAlignedGroup() const;

  virtual std::optional<const FixedTypeInfo *> getFixedTypeInfo() const;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  TOOLCHAIN_DUMP_METHOD virtual void dump() const {
    assert(isEmpty() && "Missing subclass implementation?");
    toolchain::dbgs() << "{empty}";
  }
#endif
protected:
  toolchain::Value *
  getEnumTagSinglePayloadGeneric(IRGenFunction &IGF, Address addr,
                                 toolchain::Value *numEmptyCases,
                                 toolchain::function_ref<toolchain::Value *(Address addr)>
                                     getExtraInhabitantIndexFun) const;

  void storeEnumTagSinglePayloadGeneric(
      IRGenFunction &IGF, toolchain::Value *tag, toolchain::Value *numEmptyCases,
      Address addr,
      toolchain::function_ref<void(Address addr, toolchain::Value *tag)>
          storeExtraInhabitantIndexFun) const;

  void gatherProperties(TypeLayoutEntry *fromEntry);
};

class ScalarTypeLayoutEntry : public TypeLayoutEntry,
                              public toolchain::FoldingSetNode {
public:
  const FixedTypeInfo &typeInfo;
  SILType representative;

  ScalarKind scalarKind;

  ScalarTypeLayoutEntry(const FixedTypeInfo &ti, SILType representative,
                        ScalarKind scalarKind)
      : TypeLayoutEntry(TypeLayoutEntryKind::Scalar), typeInfo(ti),
        representative(representative), scalarKind(scalarKind) {}

  ~ScalarTypeLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, const TypeInfo &ti,
                      SILType ty);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Type *getStorageType(IRGenFunction &IGF) const;
  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  static bool classof(const TypeLayoutEntry *entry);

  std::optional<const FixedTypeInfo *> getFixedTypeInfo() const override;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif

};

class ArchetypeLayoutEntry : public TypeLayoutEntry,
                             public toolchain::FoldingSetNode {
  SILType archetype;

public:
  ArchetypeLayoutEntry(SILType archetype)
      : TypeLayoutEntry(TypeLayoutEntryKind::Archetype), archetype(archetype) {}

  ~ArchetypeLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, SILType archetype);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  static bool classof(const TypeLayoutEntry *entry);

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif
};

class ResilientTypeLayoutEntry : public TypeLayoutEntry,
                                 public toolchain::FoldingSetNode {
  SILType ty;

public:
  ResilientTypeLayoutEntry(SILType ty)
      : TypeLayoutEntry(TypeLayoutEntryKind::Resilient), ty(ty) {}

  ~ResilientTypeLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, SILType ty);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  static bool classof(const TypeLayoutEntry *entry);

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif
};

class AlignedGroupEntry : public TypeLayoutEntry, public toolchain::FoldingSetNode {
  std::vector<TypeLayoutEntry *> entries;
  SILType ty;
  Alignment::int_type minimumAlignment;

  std::optional<const FixedTypeInfo *> fixedTypeInfo;

public:
  AlignedGroupEntry(const std::vector<TypeLayoutEntry *> &entries, SILType ty,
                    Alignment::int_type minimumAlignment,
                    std::optional<const FixedTypeInfo *> fixedTypeInfo)
      : TypeLayoutEntry(TypeLayoutEntryKind::AlignedGroup), entries(entries),
        ty(ty), minimumAlignment(minimumAlignment),
        fixedTypeInfo(fixedTypeInfo) {}

  ~AlignedGroupEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID,
                      const std::vector<TypeLayoutEntry *> &entries,
                      Alignment::int_type minimumAlignment);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  static bool classof(const TypeLayoutEntry *entry);

  std::optional<const FixedTypeInfo *> getFixedTypeInfo() const override;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif

private:
  /// Memoize the value of fixedSize()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed size
  /// Optional(Size) -> Fixed Size
  mutable std::optional<std::optional<Size>> _fixedSize = std::nullopt;
  /// Memoize the value of fixedAlignment()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed Alignment
  /// Optional(Alignment) -> Fixed Alignment
  mutable std::optional<std::optional<Alignment>> _fixedAlignment =
      std::nullopt;

  /// Memoize the value of fixedXICount()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed xi count
  /// Optional(Count) -> Fixed XICount
  mutable std::optional<std::optional<uint32_t>> _fixedXICount = std::nullopt;

  toolchain::Value *withExtraInhabitantProvidingEntry(
      IRGenFunction &IGF, Address addr, toolchain::Type *returnType,
      toolchain::function_ref<toolchain::Value *(TypeLayoutEntry *,
                                       Address /*entry addr*/,
                                       toolchain::Value * /*entry xi count*/)>
          entryFun) const;
  void
  withEachEntry(IRGenFunction &IGF, Address dest, Address src,
                toolchain::function_ref<void(TypeLayoutEntry *entry,
                                        Address entryDest, Address entrySrc)>
                    entryFun) const;
};

class EnumTypeLayoutEntry : public TypeLayoutEntry,
                            public toolchain::FoldingSetNode {
public:
  /// More efficient value semantics implementations for certain enum layouts.
  enum CopyDestroyStrategy {
    /// No special behavior.
    Normal,
    /// The payload is trivially destructible, so copying is bitwise (if
    /// allowed), and destruction is a noop.
    TriviallyDestroyable,
    /// The payload is a single reference-counted value, and we have
    /// a single no-payload case which uses the null extra inhabitant, so
    /// copy and destroy can pass through to retain and release entry
    /// points.
    NullableRefcounted,
    /// The payload's value witnesses can handle the extra inhabitants we use
    /// for no-payload tags, so we can forward all our calls to them.
    ForwardToPayload,
  };

  unsigned numEmptyCases;
  unsigned minimumAlignment;
  std::vector<TypeLayoutEntry *> cases;
  SILType ty;
  std::optional<const FixedTypeInfo *> fixedTypeInfo;

  EnumTypeLayoutEntry(unsigned numEmptyCases,
                      const std::vector<TypeLayoutEntry *> &cases, SILType ty,
                      std::optional<const FixedTypeInfo *> fixedTypeInfo,
                      Alignment::int_type minimumAlignment,
                      std::optional<Size> fixedSize)
      : TypeLayoutEntry(TypeLayoutEntryKind::Enum),
        numEmptyCases(numEmptyCases), minimumAlignment(minimumAlignment),
        cases(cases), ty(ty), fixedTypeInfo(fixedTypeInfo) {
    if (fixedSize) {
      _fixedSize = fixedSize;
    }
  }

  ~EnumTypeLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, unsigned numEmptyCases,
                      const std::vector<TypeLayoutEntry *> &cases);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  CopyDestroyStrategy copyDestroyKind(IRGenModule &IGM) const;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  toolchain::Value *getEnumTag(IRGenFunction &IGF, Address enumAddr) const;

  void destructiveProjectEnumData(IRGenFunction &IGF, Address enumAddr) const;

  void destructiveInjectEnumTag(IRGenFunction &IGF, toolchain::Value *tag,
                                Address enumAddr) const;

  bool isMultiPayloadEnum() const;
  bool isSingleton() const;

  std::optional<const FixedTypeInfo *> getFixedTypeInfo() const override;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif

private:
  /// Memoize the value of fixedSize()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed size
  /// Optional(Size) -> Fixed Size
  mutable std::optional<std::optional<Size>> _fixedSize = std::nullopt;
  /// Memoize the value of fixedAlignment()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed Alignment
  /// Optional(Alignment) -> Fixed Alignment
  mutable std::optional<std::optional<Alignment>> _fixedAlignment =
      std::nullopt;

  /// Memoize the value of fixedXICount()
  /// None -> Not yet computed
  /// Optional(None) -> Not fixed xi count
  /// Optional(Count) -> Fixed XICount
  mutable std::optional<std::optional<uint32_t>> _fixedXICount = std::nullopt;

  toolchain::Value *maxPayloadSize(IRGenFunction &IGF) const;
  toolchain::BasicBlock *testSinglePayloadEnumContainsPayload(IRGenFunction &IGF,
                                                         Address addr) const;

  void initializeSinglePayloadEnum(IRGenFunction &IGF, Address dest,
                                   Address src, IsTake_t isTake) const;
  void assignSinglePayloadEnum(IRGenFunction &IGF, Address dest, Address src,
                               IsTake_t isTake) const;

  void initializeMultiPayloadEnum(IRGenFunction &IGF, Address dest,
                                   Address src, IsTake_t isTake) const;
  void assignMultiPayloadEnum(IRGenFunction &IGF, Address dest, Address src,
                              IsTake_t isTake) const;

  std::pair<Address, toolchain::Value *>
  getMultiPayloadEnumTagByteAddrAndNumBytes(IRGenFunction &IGF,
                                            Address addr) const;

  toolchain::Value *
  getEnumTagSinglePayloadForSinglePayloadEnum(IRGenFunction &IGF, Address addr,
                                              toolchain::Value *numEmptyCases) const;
  void storeEnumTagSinglePayloadForSinglePayloadEnum(IRGenFunction &IGF,
                                                     toolchain::Value *tag,
                                                     toolchain::Value *numEmptyCases,
                                                     Address enumAddr) const;
  toolchain::Value *
  getEnumTagSinglePayloadForMultiPayloadEnum(IRGenFunction &IGF, Address addr,
                                             toolchain::Value *numEmptyCases) const;
  void storeEnumTagSinglePayloadForMultiPayloadEnum(IRGenFunction &IGF,
                                                    toolchain::Value *tag,
                                                    toolchain::Value *numEmptyCases,
                                                    Address enumAddr) const;
  toolchain::Value *getEnumTagMultipayload(IRGenFunction &IGF,
                                      Address enumAddr) const;

  void storeEnumTagMultipayload(IRGenFunction &IGF, toolchain::Value *tag,
                                Address enumAddr) const;

  /// Store a value to the enum's tag bytes.
  void storeMultiPayloadTag(IRGenFunction &IGF, toolchain::Value *value,
                            Address enumAddr) const;
  /// Store a value to the enum's payload bytes.
  void storeMultiPayloadValue(IRGenFunction &IGF, toolchain::Value *value,
                              Address enumAddr) const;

  void destroyMultiPayloadEnum(IRGenFunction &IGF, Address enumAddr) const;
  void destroySinglePayloadEnum(IRGenFunction &IGF, Address enumAddr) const;

  void multiPayloadEnumForPayloadAndEmptyCases(
      IRGenFunction &IGF, Address addr,
      toolchain::function_ref<void(TypeLayoutEntry *payload, toolchain::Value *tagIndex)>
          payloadFunction,
      toolchain::function_ref<void()> noPayloadFunction) const;

  bool buildSinglePayloadRefCountString(IRGenModule &IGM,
                                        LayoutStringBuilder &B,
                                        GenericSignature genericSig) const;

  bool buildMultiPayloadRefCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                                       GenericSignature genericSig) const;

  static bool classof(const TypeLayoutEntry *entry);
};

/// TypeLayouts that defer to the existing typeinfo infrastructure in cases that
/// type layouts don't have the functionality implemented yet (e.g. multi enum
/// extra inhabitants).
class TypeInfoBasedTypeLayoutEntry : public TypeLayoutEntry,
                                     public toolchain::FoldingSetNode {
public:
  const FixedTypeInfo &typeInfo;
  SILType representative;

  TypeInfoBasedTypeLayoutEntry(const FixedTypeInfo &ti, SILType representative)
      : TypeLayoutEntry(TypeLayoutEntryKind::TypeInfoBased), typeInfo(ti),
        representative(representative) {}

  ~TypeInfoBasedTypeLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, const TypeInfo &ti,
                      SILType ty);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Type *getStorageType(IRGenFunction &IGF) const;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  std::optional<const FixedTypeInfo *> getFixedTypeInfo() const override;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif
};

class ArrayLayoutEntry : public TypeLayoutEntry, public toolchain::FoldingSetNode {
public:
  TypeLayoutEntry *elementLayout;
  SILType elementType;
  CanType countType;

  ArrayLayoutEntry(TypeLayoutEntry *elementLayout, SILType elementType,
                   CanType countType)
      : TypeLayoutEntry(TypeLayoutEntryKind::Array),
        elementLayout(elementLayout), elementType(elementType),
        countType(countType) {}

  ~ArrayLayoutEntry();

  void computeProperties() override;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;
  static void Profile(toolchain::FoldingSetNodeID &ID, TypeLayoutEntry *elementLayout,
                      SILType elementType, CanType countType);

  toolchain::Value *alignmentMask(IRGenFunction &IGF) const override;
  toolchain::Value *size(IRGenFunction &IGF) const override;
  toolchain::Value *extraInhabitantCount(IRGenFunction &IGF) const override;
  bool isTriviallyDestroyable() const override;
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override;
  bool isSingleRetainablePointer() const override;
  std::optional<Size> fixedSize(IRGenModule &IGM) const override;
  bool isFixedSize(IRGenModule &IGM) const override;
  std::optional<Alignment> fixedAlignment(IRGenModule &IGM) const override;
  std::optional<uint32_t> fixedXICount(IRGenModule &IGM) const override;
  toolchain::Value *isBitwiseTakable(IRGenFunction &IGF) const override;
  toolchain::Type *getStorageType(IRGenFunction &IGF) const;

  void destroy(IRGenFunction &IGF, Address addr) const override;

  void assignWithCopy(IRGenFunction &IGF, Address dest,
                      Address src) const override;
  void assignWithTake(IRGenFunction &IGF, Address dest,
                      Address src) const override;

  void initWithCopy(IRGenFunction &IGF, Address dest,
                    Address src) const override;
  void initWithTake(IRGenFunction &IGF, Address dest,
                    Address src) const override;

  toolchain::Value *getEnumTagSinglePayload(IRGenFunction &IGF,
                                       toolchain::Value *numEmptyCases,
                                       Address addr) const override;

  void storeEnumTagSinglePayload(IRGenFunction &IGF, toolchain::Value *tag,
                                 toolchain::Value *numEmptyCases,
                                 Address enumAddr) const override;

  toolchain::Constant *layoutString(IRGenModule &IGM,
                               GenericSignature genericSig) const override;
  bool refCountString(IRGenModule &IGM, LayoutStringBuilder &B,
                      GenericSignature genericSig) const override;

  std::optional<const FixedTypeInfo *> getFixedTypeInfo() const override;

#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  void dump() const override;
#endif
};

class TypeLayoutCache {
  toolchain::BumpPtrAllocator bumpAllocator;

  toolchain::FoldingSet<ScalarTypeLayoutEntry> scalarEntries;
  toolchain::FoldingSet<ArchetypeLayoutEntry> archetypeEntries;
  toolchain::FoldingSet<AlignedGroupEntry> alignedGroupEntries;
  toolchain::FoldingSet<EnumTypeLayoutEntry> enumEntries;
  toolchain::FoldingSet<ResilientTypeLayoutEntry> resilientEntries;
  toolchain::FoldingSet<TypeInfoBasedTypeLayoutEntry> typeInfoBasedEntries;
  toolchain::FoldingSet<ArrayLayoutEntry> arrayEntries;

  TypeLayoutEntry emptyEntry;
public:
  ~TypeLayoutCache();
  ScalarTypeLayoutEntry *getOrCreateScalarEntry(const TypeInfo &ti,
                                                SILType representative,
                                                ScalarKind kind);

  ArchetypeLayoutEntry *getOrCreateArchetypeEntry(SILType archetype);

  AlignedGroupEntry *
  getOrCreateAlignedGroupEntry(const std::vector<TypeLayoutEntry *> &entries,
                               SILType ty, Alignment::int_type minimumAlignment,
                               const TypeInfo &ti);

  EnumTypeLayoutEntry *
  getOrCreateEnumEntry(unsigned numEmptyCase,
                       const std::vector<TypeLayoutEntry *> &nonEmptyCases,
                       SILType ty, const TypeInfo &ti);

  TypeInfoBasedTypeLayoutEntry *
  getOrCreateTypeInfoBasedEntry(const TypeInfo &ti, SILType representative);

  ResilientTypeLayoutEntry *getOrCreateResilientEntry(SILType ty);

  ArrayLayoutEntry *getOrCreateArrayEntry(TypeLayoutEntry *elementLayout,
                                          SILType elementType,
                                          CanType countType);

  TypeLayoutEntry *getEmptyEntry();
};

} // namespace irgen
} // namespace language
#endif
