//===--- GenExistential.cpp - Codira IR Generation for Existential Types ---===//
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
//  This file implements IR generation for existential types in Codira.
//
//===----------------------------------------------------------------------===//

#include "GenExistential.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/ExistentialLayout.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/IRGen/Linking.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/TypeLowering.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/raw_ostream.h"

#include "BitPatternBuilder.h"
#include "EnumPayload.h"
#include "Explosion.h"
#include "FixedTypeInfo.h"
#include "GenClass.h"
#include "GenHeap.h"
#include "GenMeta.h"
#include "GenOpaque.h"
#include "GenPoly.h"
#include "GenProto.h"
#include "GenType.h"
#include "HeapTypeInfo.h"
#include "IndirectTypeInfo.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "MetadataRequest.h"
#include "NonFixedTypeInfo.h"
#include "Outlining.h"
#include "ProtocolInfo.h"
#include "TypeInfo.h"

using namespace language;
using namespace irgen;

namespace {
  /// The layout of an existential buffer.  This is intended to be a
  /// small, easily-computed type that can be passed around by value.
  class OpaqueExistentialLayout {
  private:
    unsigned NumTables;
    // If you add anything to the layout computation, you might need
    // to update certain uses;  check the external uses of getNumTables().
  public:
    explicit OpaqueExistentialLayout(unsigned numTables)
      : NumTables(numTables) {}

    unsigned getNumTables() const { return NumTables; }

    Size getSize(IRGenModule &IGM) const {
      return getFixedBufferSize(IGM)
           + IGM.getPointerSize() * (getNumTables() + 1);
    }

    Alignment getAlignment(IRGenModule &IGM) const {
      return getFixedBufferAlignment(IGM);
    }

    // friend bool operator==(ExistentialLayout a, ExistentialLayout b) {
    //   return a.NumTables == b.NumTables;
    // }

    /// Given the address of an existential object, drill down to the
    /// buffer.
    Address projectExistentialBuffer(IRGenFunction &IGF, Address addr) const {
      return IGF.Builder.CreateStructGEP(addr, 0, Size(0));

    }

    /// Given the address of an existential object, drill down to the
    /// witness-table field.
    Address projectWitnessTable(IRGenFunction &IGF, Address addr,
                                unsigned which) const {
      assert(which < getNumTables());
      return IGF.Builder.CreateStructGEP(addr, which + 2,
                                         getFixedBufferSize(IGF.IGM)
                                         + IGF.IGM.getPointerSize() * (which + 1));
    }

    /// Given the address of an existential object, load its witness table.
    toolchain::Value *loadWitnessTable(IRGenFunction &IGF, Address addr,
                                  unsigned which) const {
      return IGF.Builder.CreateLoad(projectWitnessTable(IGF, addr, which));
    }

    /// Given the address of an existential object, drill down to the
    /// metadata field.
    Address projectMetadataRef(IRGenFunction &IGF, Address addr) {
      return IGF.Builder.CreateStructGEP(addr, 1, getFixedBufferSize(IGF.IGM));
    }
    
    /// Give the offset of the metadata field of an existential object.
    Size getMetadataRefOffset(IRGenModule &IGM) {
      return getFixedBufferSize(IGM);
    }

    /// Given the address of an existential object, load its metadata
    /// object.
    toolchain::Value *loadMetadataRef(IRGenFunction &IGF, Address addr) {
      return IGF.Builder.CreateLoad(projectMetadataRef(IGF, addr));
    }
  };

  /// A helper class for implementing existential type infos that
  /// store an existential value of some sort.
  template <class Derived, class Base>
  class ExistentialTypeInfoBase : public Base,
      private toolchain::TrailingObjects<Derived, const ProtocolDecl *> {
    friend class toolchain::TrailingObjects<Derived, const ProtocolDecl *>;

    /// The number of non-trivial protocols for this existential.
    unsigned NumStoredProtocols;

  protected:
    const Derived &asDerived() const {
      return *static_cast<const Derived*>(this);
    }
    Derived &asDerived() {
      return *static_cast<Derived*>(this);
    }

    template <class... As>
    ExistentialTypeInfoBase(ArrayRef<const ProtocolDecl *> protocols,
                            As &&...args)
        : Base(std::forward<As>(args)...),
          NumStoredProtocols(protocols.size()) {
      std::uninitialized_copy(protocols.begin(), protocols.end(),
          this->template getTrailingObjects<const ProtocolDecl *>());
    }

  public:
    template <class... As>
    static const Derived *
    create(ArrayRef<const ProtocolDecl *> protocols, As &&...args)
    {
      void *buffer = operator new(
          toolchain::TrailingObjects<Derived, const ProtocolDecl *>::
              template totalSizeToAlloc<const ProtocolDecl *>(
                  protocols.size()));
      return new (buffer) Derived(protocols, std::forward<As>(args)...);
    }

    /// Returns the number of protocol witness tables directly carried
    /// by values of this type.
    unsigned getNumStoredProtocols() const { return NumStoredProtocols; }

    /// Returns the protocols that values of this type are known to
    /// implement.  This can be empty, meaning that values of this
    /// type are not know to implement any protocols, although we do
    /// still know how to manipulate them.
    ArrayRef<const ProtocolDecl *> getStoredProtocols() const {
      return {this->template getTrailingObjects<const ProtocolDecl *>(),
              NumStoredProtocols};
    }

    /// Given the address of an existential object, find the witness
    /// table of a directly-stored witness table.
    toolchain::Value *loadWitnessTable(IRGenFunction &IGF, Address obj,
                                  unsigned which) const {
      return IGF.Builder.CreateLoad(
                           asDerived().projectWitnessTable(IGF, obj, which));
    }

    void emitCopyOfTables(IRGenFunction &IGF, Address dest, Address src) const {
      if (NumStoredProtocols == 0) return;

      Explosion temp;
      asDerived().emitLoadOfTables(IGF, src, temp);
      asDerived().emitStoreOfTables(IGF, temp, dest);
    }

    void emitLoadOfTables(IRGenFunction &IGF, Address existential,
                          Explosion &out) const {
      for (unsigned i = 0; i != NumStoredProtocols; ++i) {
        auto tableAddr = asDerived().projectWitnessTable(IGF, existential, i);
        out.add(IGF.Builder.CreateLoad(tableAddr));
      }
    }

    void emitStoreOfTables(IRGenFunction &IGF, Explosion &in,
                           Address existential) const {
      for (unsigned i = 0; i != NumStoredProtocols; ++i) {
        auto tableAddr = asDerived().projectWitnessTable(IGF, existential, i);
        IGF.Builder.CreateStore(in.claimNext(), tableAddr);
      }
    }
  };

  /// A type implementation for address-only reference storage of
  /// class existential types.
  template <class Impl, class Base>
  class AddressOnlyClassExistentialTypeInfoBase :
      public ExistentialTypeInfoBase<Impl, IndirectTypeInfo<Impl, Base>> {
    using super = ExistentialTypeInfoBase<Impl, IndirectTypeInfo<Impl, Base>>;

    using super::asDerived;
    using super::emitCopyOfTables;

  protected:
    using super::getNumStoredProtocols;
    const ReferenceCounting Refcounting;

    template <class... As>
    AddressOnlyClassExistentialTypeInfoBase(
        ArrayRef<const ProtocolDecl *> protocols,
        ReferenceCounting refcounting,
        As &&...args)
      : super(protocols, std::forward<As>(args)...),
        Refcounting(refcounting) {
    }

  public:
    Address projectWitnessTable(IRGenFunction &IGF, Address container,
                                unsigned index) const {
      assert(index < getNumStoredProtocols());
      return IGF.Builder.CreateStructGEP(container, index + 1,
                                    (index + 1) * IGF.IGM.getPointerSize());
    }

    Address projectValue(IRGenFunction &IGF, Address existential) const {
      return IGF.Builder.CreateStructGEP(existential, 0, Size(0),
                            existential.getAddress()->getName() +
                            asDerived().getStructNameSuffix());
    }

    void assignWithCopy(IRGenFunction &IGF, Address dest, Address src, SILType T,
                        bool isOutlined) const override {
      if (isOutlined || T.hasLocalArchetype()) {
        Address destValue = projectValue(IGF, dest);
        Address srcValue = projectValue(IGF, src);
        asDerived().emitValueAssignWithCopy(IGF, destValue, srcValue);
        emitCopyOfTables(IGF, dest, src);
      } else {
        OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                             DeinitIsNotNeeded);
        collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                         IsNotInitialization, IsNotTake);
      }
    }

    void initializeWithCopy(IRGenFunction &IGF, Address dest, Address src,
                            SILType T, bool isOutlined) const override {
      if (isOutlined || T.hasLocalArchetype()) {
        Address destValue = projectValue(IGF, dest);
        Address srcValue = projectValue(IGF, src);
        asDerived().emitValueInitializeWithCopy(IGF, destValue, srcValue);
        emitCopyOfTables(IGF, dest, src);
      } else {
        OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                             DeinitIsNotNeeded);
        collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                         IsInitialization, IsNotTake);
      }
    }

    void assignWithTake(IRGenFunction &IGF, Address dest, Address src, SILType T,
                        bool isOutlined) const override {
      if (isOutlined || T.hasLocalArchetype()) {
        Address destValue = projectValue(IGF, dest);
        Address srcValue = projectValue(IGF, src);
        asDerived().emitValueAssignWithTake(IGF, destValue, srcValue);
        emitCopyOfTables(IGF, dest, src);
      } else {
        OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                             DeinitIsNotNeeded);
        collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                         IsNotInitialization, IsTake);
      }
    }

    void initializeWithTake(IRGenFunction &IGF, Address dest, Address src,
                            SILType T, bool isOutlined,
                            bool zeroizeIfSensitive) const override {
      if (isOutlined || T.hasLocalArchetype()) {
        Address destValue = projectValue(IGF, dest);
        Address srcValue = projectValue(IGF, src);
        asDerived().emitValueInitializeWithTake(IGF, destValue, srcValue);
        emitCopyOfTables(IGF, dest, src);
      } else {
        OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                             DeinitIsNotNeeded);
        collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                         IsInitialization, IsTake);
      }
    }

    void destroy(IRGenFunction &IGF, Address existential, SILType T,
                 bool isOutlined) const override {
      if (isOutlined || T.hasLocalArchetype()) {
        Address valueAddr = projectValue(IGF, existential);
        asDerived().emitValueDestroy(IGF, valueAddr);
      } else {
        OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                             DeinitIsNeeded);
        collector.emitCallToOutlinedDestroy(existential, T, *this);
      }
    }
  };

  /// A helper class for working with existential types that can be
  /// exploded into scalars.
  ///
  /// The subclass must provide:
  ///   void emitValueRetain(IRGenFunction &IGF, toolchain::Value *value) const;
  ///   void emitValueRelease(IRGenFunction &IGF, toolchain::Value *value) const;
  ///   void emitValueFixLifetime(IRGenFunction &IGF,
  ///                               toolchain::Value *value) const;
  ///   const LoadableTypeInfo &
  ///       getValueTypeInfoForExtraInhabitants(IRGenModule &IGM) const;
  /// The value type info is only used to manage extra inhabitants, so it's
  /// okay for it to implement different semantics.
  template <class Derived, class Base>
  class ScalarExistentialTypeInfoBase :
    public ExistentialTypeInfoBase<Derived, ScalarTypeInfo<Derived, Base>> {

    using super =
           ExistentialTypeInfoBase<Derived, ScalarTypeInfo<Derived, Base>>;

  protected:
    template <class... T>
    ScalarExistentialTypeInfoBase(T &&...args)
      : super(std::forward<T>(args)...) {}

    using super::asDerived;

  public:
    /// The storage type of a class existential is a struct containing
    /// a refcounted pointer to the class instance value followed by
    /// witness table pointers for each conformed-to protocol. Unlike opaque
    /// existentials, a class existential does not need to store type
    /// metadata as an additional element, since it can be derived from the
    /// class instance.
    toolchain::StructType *getStorageType() const {
      return cast<toolchain::StructType>(TypeInfo::getStorageType());
    }

    using super::getNumStoredProtocols;

    unsigned getExplosionSize() const final {
      return 1 + getNumStoredProtocols();
    }

    void getSchema(ExplosionSchema &schema) const override {
      schema.add(ExplosionSchema::Element::forScalar(asDerived().getValueType()));

      toolchain::StructType *ty = getStorageType();
      for (unsigned i = 1, e = getExplosionSize(); i != e; ++i)
        schema.add(ExplosionSchema::Element::forScalar(ty->getElementType(i)));
    }

    void addToAggLowering(IRGenModule &IGM, CodiraAggLowering &lowering,
                          Size offset) const override {
      auto ptrSize = IGM.getPointerSize();
      LoadableTypeInfo::addScalarToAggLowering(IGM, lowering,
                                               asDerived().getValueType(),
                                               offset, ptrSize);

      toolchain::StructType *ty = getStorageType();
      for (unsigned i = 1, e = getExplosionSize(); i != e; ++i)
        LoadableTypeInfo::addScalarToAggLowering(IGM, lowering,
                                                 ty->getElementType(i),
                                                 offset + i * ptrSize, ptrSize);
    }

    /// Given the address of a class existential container, returns
    /// the address of a witness table pointer.
    Address projectWitnessTable(IRGenFunction &IGF, Address address,
                                unsigned n) const {
      assert(n < getNumStoredProtocols() && "witness table index out of bounds");
      return IGF.Builder.CreateStructGEP(address, n+1,
                                         IGF.IGM.getPointerSize() * (n+1));
    }

    /// Return the type of the instance value.
    toolchain::Type *getValueType() const {
      return getStorageType()->getElementType(0);
    }

    /// Given the address of a class existential container, returns
    /// the address of its instance pointer.
    Address projectValue(IRGenFunction &IGF, Address address) const {
      return IGF.Builder.CreateStructGEP(address, 0, Size(0));
    }

    toolchain::Value *loadValue(IRGenFunction &IGF, Address addr) const {
      return IGF.Builder.CreateLoad(asDerived().projectValue(IGF, addr));
    }

    /// Given a class existential container, returns a witness table
    /// pointer out of the container, and the type metadata pointer for the
    /// value.
    toolchain::Value *
    extractWitnessTable(IRGenFunction &IGF, Explosion &container,
                        unsigned which) const {
      assert(which < getNumStoredProtocols() && "witness table index out of bounds");
      ArrayRef<toolchain::Value *> values = container.claim(getExplosionSize());
      return values[which+1];
    }

    /// Deconstruct an existential object into witness tables and instance
    /// pointer.
    std::pair<ArrayRef<toolchain::Value*>, toolchain::Value*>
    getWitnessTablesAndValue(Explosion &container) const {
      toolchain::Value *instance = container.claimNext();
      ArrayRef<toolchain::Value*> witnesses = container.claim(getNumStoredProtocols());
      return {witnesses, instance};
    }

    /// Given an existential object, returns the payload value.
    toolchain::Value *getValue(IRGenFunction &IGF, Explosion &container) const {
      toolchain::Value *instance = container.claimNext();
      (void)container.claim(getNumStoredProtocols());
      return instance;
    }

    void loadAsCopy(IRGenFunction &IGF, Address address,
                    Explosion &out) const override {
      // Load the instance pointer, which is unknown-refcounted.
      toolchain::Value *instance = asDerived().loadValue(IGF, address);
      asDerived().emitValueRetain(IGF, instance, IGF.getDefaultAtomicity());
      out.add(instance);

      // Load the witness table pointers.
      asDerived().emitLoadOfTables(IGF, address, out);
    }

    void loadAsTake(IRGenFunction &IGF, Address address,
                    Explosion &e) const override {
      // Load the instance pointer.
      e.add(asDerived().loadValue(IGF, address));

      // Load the witness table pointers.
      asDerived().emitLoadOfTables(IGF, address, e);
    }

    void assign(IRGenFunction &IGF, Explosion &e, Address address,
                bool isOutlined, SILType T) const override {
      // Assign the value.
      Address instanceAddr = asDerived().projectValue(IGF, address);
      toolchain::Value *old = IGF.Builder.CreateLoad(instanceAddr);
      IGF.Builder.CreateStore(e.claimNext(), instanceAddr);
      asDerived().emitValueRelease(IGF, old, IGF.getDefaultAtomicity());

      // Store the witness table pointers.
      asDerived().emitStoreOfTables(IGF, e, address);
    }

    void initialize(IRGenFunction &IGF, Explosion &e, Address address,
                    bool isOutlined) const override {
      // Store the instance pointer.
      IGF.Builder.CreateStore(e.claimNext(),
                              asDerived().projectValue(IGF, address));

      // Store the witness table pointers.
      asDerived().emitStoreOfTables(IGF, e, address);
    }

    void copy(IRGenFunction &IGF, Explosion &src, Explosion &dest,
              Atomicity atomicity)
    const override {
      // Copy the instance pointer.
      toolchain::Value *value = src.claimNext();
      dest.add(value);
      asDerived().emitValueRetain(IGF, value, atomicity);

      // Transfer the witness table pointers.
      src.transferInto(dest, getNumStoredProtocols());
    }

    void consume(IRGenFunction &IGF, Explosion &src, Atomicity atomicity,
                 SILType T)
    const override {
      // Copy the instance pointer.
      toolchain::Value *value = src.claimNext();
      asDerived().emitValueRelease(IGF, value, atomicity);

      // Throw out the witness table pointers.
      (void)src.claim(getNumStoredProtocols());
    }

    void fixLifetime(IRGenFunction &IGF, Explosion &src) const override {
      // Copy the instance pointer.
      toolchain::Value *value = src.claimNext();
      asDerived().emitValueFixLifetime(IGF, value);

      // Throw out the witness table pointers.
      (void)src.claim(getNumStoredProtocols());
    }

    void destroy(IRGenFunction &IGF, Address addr, SILType T,
                 bool isOutlined) const override {
      // Small type (scalar) do not create outlined function
      toolchain::Value *value = asDerived().loadValue(IGF, addr);
      asDerived().emitValueRelease(IGF, value, IGF.getDefaultAtomicity());
    }

    void packIntoEnumPayload(IRGenModule &IGM,
                             IRBuilder &builder,
                             EnumPayload &payload,
                             Explosion &src,
                             unsigned offset) const override {
      payload.insertValue(IGM, builder, src.claimNext(), offset);
      auto wordSize = IGM.getPointerSize().getValueInBits();
      for (unsigned i = 0; i < getNumStoredProtocols(); ++i) {
        offset += wordSize;
        payload.insertValue(IGM, builder, src.claimNext(), offset);
      }
    }

    void unpackFromEnumPayload(IRGenFunction &IGF,
                               const EnumPayload &payload,
                               Explosion &dest,
                               unsigned offset) const override {
      ExplosionSchema schema;
      getSchema(schema);
      dest.add(payload.extractValue(IGF, schema[0].getScalarType(), offset));
      auto wordSize = IGF.IGM.getPointerSize().getValueInBits();
      for (unsigned i = 0; i < getNumStoredProtocols(); ++i) {
        offset += wordSize;
        dest.add(payload.extractValue(IGF, IGF.IGM.WitnessTablePtrTy, offset));
      }
    }


    // Extra inhabitants of the various scalar existential containers.
    // We use the heap object extra inhabitants over the class pointer value.
    // We could get even more extra inhabitants from the witness table
    // pointer(s), but it's unlikely we would ever need to.

    bool mayHaveExtraInhabitants(IRGenModule &IGM) const override {
      assert(asDerived().getValueTypeInfoForExtraInhabitants(IGM)
                        .mayHaveExtraInhabitants(IGM));
      return true;
    }

    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override {
      return asDerived().getValueTypeInfoForExtraInhabitants(IGM)
                        .getFixedExtraInhabitantCount(IGM);
    }

    APInt getFixedExtraInhabitantValue(IRGenModule &IGM,
                                       unsigned bits,
                                       unsigned index) const override {
      // Note that we pass down the original bit-width.
      return asDerived().getValueTypeInfoForExtraInhabitants(IGM)
                        .getFixedExtraInhabitantValue(IGM, bits, index);
    }

    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src,
                                         SILType T, bool isOutlined)
    const override {
      // NB: We assume that the witness table slots are zero if an extra
      // inhabitant is stored in the container.
      src = projectValue(IGF, src);
      return asDerived().getValueTypeInfoForExtraInhabitants(IGF.IGM)
                      .getExtraInhabitantIndex(IGF, src, SILType(), isOutlined);
    }

    void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index,
                              Address dest, SILType T, bool isOutlined)
    const override {
      Address valueDest = projectValue(IGF, dest);
      asDerived().getValueTypeInfoForExtraInhabitants(IGF.IGM)
            .storeExtraInhabitant(IGF, index, valueDest, SILType(), isOutlined);
    }

    APInt getFixedExtraInhabitantMask(IRGenModule &IGM) const override {
      // Ask the value type for its mask.
      APInt bits = asDerived().getValueTypeInfoForExtraInhabitants(IGM)
                              .getFixedExtraInhabitantMask(IGM);

      // Zext out to the size of the existential.
      auto totalSize = asDerived().getFixedSize().getValueInBits();
      auto mask = BitPatternBuilder(IGM.Triple.isLittleEndian());
      mask.append(bits);
      mask.padWithClearBitsTo(totalSize);
      return mask.build().value();
    }
  };

/// A type implementation for existential types.
#define REF_STORAGE_HELPER(Name, Super) \
  private: \
    bool shouldStoreExtraInhabitantsInRef(IRGenModule &IGM) const { \
      if (IGM.getReferenceStorageExtraInhabitantCount( \
                                   ReferenceOwnership::Name, Refcounting) > 1) \
        return true; \
      return getNumStoredProtocols() == 0; \
    } \
  public: \
    bool mayHaveExtraInhabitants(IRGenModule &IGM) const override { \
      return getFixedExtraInhabitantCount(IGM) > 0; \
    } \
    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override { \
      if (shouldStoreExtraInhabitantsInRef(IGM)) { \
        return IGM.getReferenceStorageExtraInhabitantCount( \
                          ReferenceOwnership::Name, Refcounting) - IsOptional; \
      } else { \
        return Super::getFixedExtraInhabitantCount(IGM); \
      } \
    } \
    APInt getFixedExtraInhabitantValue(IRGenModule &IGM, \
                                       unsigned bits, \
                                       unsigned index) const override { \
      if (shouldStoreExtraInhabitantsInRef(IGM)) { \
        return IGM.getReferenceStorageExtraInhabitantValue(bits, \
                                                     index + IsOptional, \
                                                     ReferenceOwnership::Name, \
                                                     Refcounting); \
      } else { \
        return Super::getFixedExtraInhabitantValue(IGM, bits, index); \
      } \
    } \
    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src, \
                                         SILType T, bool isOutlined) \
    const override { \
      Address valueSrc = projectValue(IGF, src); \
      if (shouldStoreExtraInhabitantsInRef(IGF.IGM)) { \
        return IGF.getReferenceStorageExtraInhabitantIndex(valueSrc, \
                                       ReferenceOwnership::Name, Refcounting); \
      } else { \
        return Super::getExtraInhabitantIndex(IGF, src, T, isOutlined); \
      } \
    } \
    void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index, \
                              Address dest, SILType T, bool isOutlined) \
    const override { \
      Address valueDest = projectValue(IGF, dest); \
      if (shouldStoreExtraInhabitantsInRef(IGF.IGM)) { \
        return IGF.storeReferenceStorageExtraInhabitant(index, valueDest, \
                                       ReferenceOwnership::Name, Refcounting); \
      } else { \
        return Super::storeExtraInhabitant(IGF, index, dest, T, isOutlined); \
      } \
    } \
    APInt getFixedExtraInhabitantMask(IRGenModule &IGM) const override { \
      if (shouldStoreExtraInhabitantsInRef(IGM)) { \
        APInt bits = IGM.getReferenceStorageExtraInhabitantMask( \
                                                     ReferenceOwnership::Name, \
                                                     Refcounting); \
        /* Zext out to the size of the existential. */ \
        auto mask = BitPatternBuilder(IGM.Triple.isLittleEndian()); \
        mask.append(bits); \
        mask.padWithClearBitsTo(getFixedSize().getValueInBits()); \
        return mask.build().value(); \
      } else { \
        return Super::getFixedExtraInhabitantMask(IGM); \
      } \
    }
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  class AddressOnly##Name##ClassExistentialTypeInfo final : \
      public AddressOnlyClassExistentialTypeInfoBase< \
                                  AddressOnly##Name##ClassExistentialTypeInfo, \
                                  FixedTypeInfo> { \
    bool IsOptional; \
  public: \
    AddressOnly##Name##ClassExistentialTypeInfo( \
        ArrayRef<const ProtocolDecl *> protocols, \
        toolchain::Type *ty, \
        SpareBitVector &&spareBits, \
        Size size, Alignment align, \
        ReferenceCounting refcounting, \
        bool isOptional) \
      : AddressOnlyClassExistentialTypeInfoBase(protocols, refcounting, \
                                                ty, size, std::move(spareBits), \
                                                align, IsNotTriviallyDestroyable, \
                                                IsNotBitwiseTakable, \
                                                IsCopyable, \
                                                IsFixedSize, \
                                                IsABIAccessible), \
        IsOptional(isOptional) {} \
    TypeLayoutEntry \
    *buildTypeLayoutEntry(IRGenModule &IGM, \
                          SILType T, \
                          bool useStructLayouts) const override { \
      if (!useStructLayouts) { \
        return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T); \
      } else if (Refcounting == ReferenceCounting::Native) { \
        return IGM.typeLayoutCache.getOrCreateScalarEntry( \
            *this, T, ScalarKind::Native##Name##Reference); \
      } else { \
        return IGM.typeLayoutCache.getOrCreateScalarEntry( \
             *this, T, ScalarKind::Unknown##Name##Reference); \
      } \
    } \
    void emitValueAssignWithCopy(IRGenFunction &IGF, \
                                 Address dest, Address src) const { \
      IGF.emit##Name##CopyAssign(dest, src, Refcounting); \
    } \
    void emitValueInitializeWithCopy(IRGenFunction &IGF, \
                                     Address dest, Address src) const { \
      IGF.emit##Name##CopyInit(dest, src, Refcounting); \
    } \
    void emitValueAssignWithTake(IRGenFunction &IGF, \
                                Address dest, Address src) const { \
      IGF.emit##Name##TakeAssign(dest, src, Refcounting); \
    } \
    void emitValueInitializeWithTake(IRGenFunction &IGF, \
                                    Address dest, Address src) const { \
      IGF.emit##Name##TakeInit(dest, src, Refcounting); \
    } \
    void emitValueDestroy(IRGenFunction &IGF, Address addr) const { \
      IGF.emit##Name##Destroy(addr, Refcounting); \
    } \
    StringRef getStructNameSuffix() const { return "." #name "ref"; } \
    REF_STORAGE_HELPER(Name, FixedTypeInfo) \
  };
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, ...) \
  class Loadable##Name##ClassExistentialTypeInfo final \
    : public ScalarExistentialTypeInfoBase< \
                                     Loadable##Name##ClassExistentialTypeInfo, \
                                     LoadableTypeInfo> { \
    ReferenceCounting Refcounting; \
    toolchain::Type *ValueType; \
    bool IsOptional; \
  public: \
    Loadable##Name##ClassExistentialTypeInfo( \
        ArrayRef<const ProtocolDecl *> storedProtocols, \
        toolchain::Type *valueTy, toolchain::Type *ty, \
        const SpareBitVector &spareBits, \
        Size size, Alignment align, \
        ReferenceCounting refcounting, \
        bool isOptional) \
      : ScalarExistentialTypeInfoBase(storedProtocols, ty, size, \
                                      spareBits, align, \
                                      IsNotTriviallyDestroyable, \
                                      IsCopyable, \
                                      IsFixedSize, IsABIAccessible), \
        Refcounting(refcounting), ValueType(valueTy), IsOptional(isOptional) { \
      assert(refcounting == ReferenceCounting::Native || \
             refcounting == ReferenceCounting::Unknown); \
    } \
    TypeLayoutEntry \
    *buildTypeLayoutEntry(IRGenModule &IGM, \
                          SILType T, \
                          bool useStructLayouts) const override { \
      if (!useStructLayouts) { \
        return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T); \
      } \
      ScalarKind kind; \
      switch (Refcounting) { \
        case ReferenceCounting::Native:  kind = ScalarKind::NativeStrongReference; break; \
        case ReferenceCounting::ObjC:    kind = ScalarKind::ObjCReference; break; \
        case ReferenceCounting::Block:   kind = ScalarKind::BlockReference; break; \
        case ReferenceCounting::Unknown: kind = ScalarKind::UnknownReference; break; \
        case ReferenceCounting::Bridge:  kind = ScalarKind::BridgeReference; break; \
        case ReferenceCounting::Error:   kind = ScalarKind::ErrorReference; break; \
        case ReferenceCounting::None:    kind = ScalarKind::TriviallyDestroyable; break; \
        case ReferenceCounting::Custom:  kind = ScalarKind::CustomReference; break; \
      } \
      return IGM.typeLayoutCache.getOrCreateScalarEntry(*this, T, kind); \
    } \
    toolchain::Type *getValueType() const { \
      return ValueType; \
    } \
    Address projectValue(IRGenFunction &IGF, Address addr) const { \
      Address valueAddr = ScalarExistentialTypeInfoBase::projectValue(IGF, addr); \
      return IGF.Builder.CreateElementBitCast(valueAddr, ValueType); \
    } \
    void emitValueRetain(IRGenFunction &IGF, toolchain::Value *value, \
                         Atomicity atomicity) const { \
      IGF.emit##Name##Retain(value, Refcounting, atomicity); \
    } \
    void emitValueRelease(IRGenFunction &IGF, toolchain::Value *value, \
                          Atomicity atomicity) const { \
      IGF.emit##Name##Release(value, Refcounting, atomicity); \
    } \
    void emitValueFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const { \
      IGF.emitFixLifetime(value); \
    } \
    const LoadableTypeInfo & \
    getValueTypeInfoForExtraInhabitants(IRGenModule &IGM) const { \
      toolchain_unreachable("should have overridden all actual uses of this"); \
    } \
    REF_STORAGE_HELPER(Name, LoadableTypeInfo) \
  };
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, name, "...") \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, "...")

/// A type implementation for static reference storage class existential types
/// that do not generate dynamic (i.e. runtime) logic.
#define UNCHECKED_REF_STORAGE(Name, ...) \
  class Name##ClassExistentialTypeInfo final \
    : public ScalarExistentialTypeInfoBase<Name##ClassExistentialTypeInfo, \
                                           LoadableTypeInfo> { \
  bool IsOptional; \
  public: \
    Name##ClassExistentialTypeInfo( \
        ArrayRef<const ProtocolDecl *> storedProtocols, \
        toolchain::Type *ty, \
        const SpareBitVector &spareBits, \
        Size size, Alignment align, \
        bool isOptional) \
      : ScalarExistentialTypeInfoBase(storedProtocols, ty, size, \
                                      spareBits, align, IsTriviallyDestroyable,\
                                      IsCopyable, IsFixedSize, IsABIAccessible), \
        IsOptional(isOptional) {} \
    TypeLayoutEntry \
    *buildTypeLayoutEntry(IRGenModule &IGM, \
                          SILType T, \
                          bool useStructLayouts) const override { \
      if (!useStructLayouts) { \
        return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T); \
      } \
      return IGM.typeLayoutCache.getOrCreateScalarEntry(*this, T, ScalarKind::TriviallyDestroyable); \
    } \
    const LoadableTypeInfo & \
    getValueTypeInfoForExtraInhabitants(IRGenModule &IGM) const { \
      if (!IGM.ObjCInterop) \
        return IGM.getNativeObjectTypeInfo(); \
      else \
        return IGM.getUnknownObjectTypeInfo(); \
    } \
    unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override { \
      return getValueTypeInfoForExtraInhabitants(IGM) \
                  .getFixedExtraInhabitantCount(IGM) - IsOptional; \
    } \
    APInt getFixedExtraInhabitantValue(IRGenModule &IGM, \
                                       unsigned bits, \
                                       unsigned index) const override { \
      /* Note that we pass down the original bit-width. */ \
      return getValueTypeInfoForExtraInhabitants(IGM) \
                  .getFixedExtraInhabitantValue(IGM, bits, \
                                                index + IsOptional); \
    } \
    toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, \
                                         Address src, SILType T, \
                                         bool isOutlined) const override { \
      return PointerInfo::forHeapObject(IGF.IGM) \
        .withNullable(IsNullable_t(IsOptional)) \
        .getExtraInhabitantIndex(IGF, src); \
    } \
    /* FIXME -- Use REF_STORAGE_HELPER and make */ \
    /* getValueTypeInfoForExtraInhabitants call toolchain_unreachable() */ \
    void emitValueRetain(IRGenFunction &IGF, toolchain::Value *value, \
                         Atomicity atomicity) const {} \
    void emitValueRelease(IRGenFunction &IGF, toolchain::Value *value, \
                          Atomicity atomicity) const {} \
    void emitValueFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const {} \
  };
#include "language/AST/ReferenceStorage.def"
#undef REF_STORAGE_HELPER
} // end anonymous namespace

static toolchain::Function *getAssignBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout);

static toolchain::Function *getDestroyBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout);

static toolchain::Function *
getProjectBoxedOpaqueExistentialFunction(IRGenFunction &IGF,
                                         OpenedExistentialAccess accessKind,
                                         OpaqueExistentialLayout existLayout);

namespace {


/// A TypeInfo implementation for existential types, i.e., types like:
///   Printable
///   Printable & Serializable
///   Any
/// with the semantic translation:
///   \exists t : Printable . t
/// t here is an ArchetypeType.
///
/// This is used for both ProtocolTypes and ProtocolCompositionTypes.
class OpaqueExistentialTypeInfo final :
    public ExistentialTypeInfoBase<OpaqueExistentialTypeInfo,
             IndirectTypeInfo<OpaqueExistentialTypeInfo, FixedTypeInfo>> {

  using super =
           ExistentialTypeInfoBase<OpaqueExistentialTypeInfo,
             IndirectTypeInfo<OpaqueExistentialTypeInfo, FixedTypeInfo>>;
  friend super;

  OpaqueExistentialTypeInfo(ArrayRef<const ProtocolDecl *> protocols,
                            toolchain::Type *ty, Size size,
                            IsCopyable_t copyable,
                            SpareBitVector &&spareBits,
                            Alignment align)
    : super(protocols, ty, size,
            std::move(spareBits), align,
            IsNotTriviallyDestroyable,
            // Copyable existentials are bitwise-takable and borrowable.
            // Noncopyable existentials are bitwise-takable only.
            copyable ? IsBitwiseTakableAndBorrowable : IsBitwiseTakableOnly,
            copyable,
            IsFixedSize, IsABIAccessible) {}

public:
  OpaqueExistentialLayout getLayout() const {
    return OpaqueExistentialLayout(getNumStoredProtocols());
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }
    return IGM.typeLayoutCache.getOrCreateScalarEntry(
        *this, T, ScalarKind::ExistentialReference);
  }

  Address projectWitnessTable(IRGenFunction &IGF, Address obj,
                              unsigned index) const {
    return getLayout().projectWitnessTable(IGF, obj, index);
  }

  void assignWithCopy(IRGenFunction &IGF, Address dest, Address src, SILType T,
                      bool isOutlined) const override {

    // Use copy-on-write existentials?
    auto fn = getAssignBoxedOpaqueExistentialBufferFunction(
        IGF.IGM, getLayout());
    auto *type = IGF.IGM.getExistentialPtrTy(getLayout().getNumTables());
    auto *destAddr = IGF.Builder.CreateBitCast(dest.getAddress(), type);
    auto *srcAddr = IGF.Builder.CreateBitCast(src.getAddress(), type);
    auto call = IGF.Builder.CreateCallWithoutDbgLoc(fn->getFunctionType(), fn,
                                                    {destAddr, srcAddr});
    call->setCallingConv(IGF.IGM.DefaultCC);
    call->setDoesNotThrow();
    return;
  }

  toolchain::Value *copyType(IRGenFunction &IGF, Address dest, Address src) const {
    auto layout = getLayout();

    toolchain::Value *metadata = layout.loadMetadataRef(IGF, src);
    IGF.Builder.CreateStore(metadata, layout.projectMetadataRef(IGF, dest));

    // Load the witness tables and copy them into the new object.
    emitCopyOfTables(IGF, dest, src);

    return metadata;
  }

  void initializeWithCopy(IRGenFunction &IGF, Address dest, Address src,
                          SILType T, bool isOutlined) const override {
    if (isOutlined || T.hasLocalArchetype()) {
      toolchain::Value *metadata = copyType(IGF, dest, src);

      auto layout = getLayout();

      // Project down to the buffers and ask the witnesses to do a
      // copy-initialize.
      Address srcBuffer = layout.projectExistentialBuffer(IGF, src);
      Address destBuffer = layout.projectExistentialBuffer(IGF, dest);
      emitInitializeBufferWithCopyOfBufferCall(IGF, metadata, destBuffer,
                                               srcBuffer);
    } else {
      // Create an outlined function to avoid explosion
      OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                           DeinitIsNotNeeded);
      collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                       IsInitialization, IsNotTake);
    }
  }

  void initializeWithTake(IRGenFunction &IGF, Address dest, Address src,
                          SILType T, bool isOutlined,
                          bool zeroizeIfSensitive) const override {
    if (isOutlined || T.hasLocalArchetype()) {
      // memcpy the existential container. This is safe because: either the
      // value is stored inline and is therefore by convention bitwise takable
      // or the value is stored in a reference counted heap buffer, in which
      // case a memcpy of the reference is also correct.
      IGF.emitMemCpy(dest, src, getLayout().getSize(IGF.IGM));
    } else {
      // Create an outlined function to avoid explosion
      OutliningMetadataCollector collector(T, IGF, LayoutIsNeeded,
                                           DeinitIsNotNeeded);
      collector.emitCallToOutlinedCopy(dest, src, T, *this,
                                       IsInitialization, IsTake);
    }
  }

  void destroy(IRGenFunction &IGF, Address buffer, SILType T,
               bool isOutlined) const override {
    // Use copy-on-write existentials?
    auto fn = getDestroyBoxedOpaqueExistentialBufferFunction(
        IGF.IGM, getLayout());
    auto *addr = IGF.Builder.CreateBitCast(
        buffer.getAddress(),
        IGF.IGM.getExistentialPtrTy(getLayout().getNumTables()));
    auto call =
        IGF.Builder.CreateCallWithoutDbgLoc(fn->getFunctionType(), fn, {addr});
    call->setCallingConv(IGF.IGM.DefaultCC);
    call->setDoesNotThrow();
    return;
  }
               
  // Opaque existentials have extra inhabitants and spare bits in their type
  // metadata pointer, matching those of a standalone thick metatype (which
  // in turn match those of a heap object).
  unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override {
    return getHeapObjectExtraInhabitantCount(IGM);
  }
  APInt getFixedExtraInhabitantValue(IRGenModule &IGM,
                                     unsigned bits,
                                     unsigned index) const override {
    auto offset = getLayout().getMetadataRefOffset(IGM).getValueInBits();
    return getHeapObjectFixedExtraInhabitantValue(IGM, bits, index, offset);
  }
  APInt getFixedExtraInhabitantMask(IRGenModule &IGM) const override {
    auto mask = BitPatternBuilder(IGM.Triple.isLittleEndian());
    mask.appendClearBits(getLayout().getMetadataRefOffset(IGM).getValueInBits());
    mask.appendSetBits(IGM.getPointerSize().getValueInBits());
    mask.padWithClearBitsTo(getFixedSize().getValueInBits());
    return mask.build().value();
  }
  toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF,
                                       Address src, SILType T,
                                       bool isOutlined) const override {
    auto type = getLayout().projectMetadataRef(IGF, src);
    return getHeapObjectExtraInhabitantIndex(IGF, type);
  }
  void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index,
                            Address dest, SILType T, bool isOutlined)
  const override {
    auto type = getLayout().projectMetadataRef(IGF, dest);
    return storeHeapObjectExtraInhabitant(IGF, index, type);
  }
};


/// A type info implementation for class existential types, that is,
/// an existential type known to conform to one or more class protocols.
/// Class existentials can be represented directly as an aggregation
/// of a refcounted pointer plus witness tables instead of using an indirect
/// buffer.
class ClassExistentialTypeInfo final
    : public ScalarExistentialTypeInfoBase<ClassExistentialTypeInfo,
                                           ReferenceTypeInfo> {
  ReferenceCounting Refcounting;
 
  friend ExistentialTypeInfoBase;
  ClassExistentialTypeInfo(ArrayRef<const ProtocolDecl *> protocols,
                           toolchain::Type *ty,
                           Size size,
                           SpareBitVector &&spareBits,
                           Alignment align,
                           ReferenceCounting refcounting)
    : ScalarExistentialTypeInfoBase(protocols, ty, size,
                                    std::move(spareBits), align),
      Refcounting(refcounting) {
    assert(refcounting == ReferenceCounting::Native ||
           refcounting == ReferenceCounting::Unknown ||
           refcounting == ReferenceCounting::ObjC);
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    // We can't create an objc typeinfo by itself, so don't destructure if we
    // have one
    if (!useStructLayouts ||
        Refcounting == ReferenceCounting::ObjC) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }

    /// The storage type of a class existential is a struct containing
    /// a refcounted pointer to the class instance value followed by
    /// witness table pointers for each conformed-to protocol.
    std::vector<TypeLayoutEntry *> alignedGroup;
    const TypeInfo &typeinfo = Refcounting == ReferenceCounting::Native
                                   ? IGM.getNativeObjectTypeInfo()
                                   : IGM.getUnknownObjectTypeInfo();

    alignedGroup.push_back(IGM.typeLayoutCache.getOrCreateScalarEntry(
        typeinfo, T, refcountingToScalarKind(Refcounting)));
    for (unsigned i = 0; i < getNumStoredProtocols(); i++) {
      alignedGroup.push_back(IGM.typeLayoutCache.getOrCreateScalarEntry(
          IGM.getWitnessTablePtrTypeInfo(),
          SILType::getBuiltinIntegerType(IGM.getPointerSize().getValue(),
                                         IGM.Context),
          ScalarKind::TriviallyDestroyable));
    }

    return IGM.typeLayoutCache.getOrCreateAlignedGroupEntry(
        alignedGroup, T, getBestKnownAlignment().getValue(), *this);
  }

  /// Given an explosion with multiple pointer elements in them, pack them
  /// into an enum payload explosion.
  /// FIXME: Assumes the explosion is broken into word-sized integer chunks.
  /// Should use EnumPayload.
  void mergeExplosion(Explosion &In, Explosion &Out, IRGenFunction &IGF)
  const {
    // We always have at least one entry.
    auto *part = In.claimNext();
    Out.add(IGF.Builder.CreatePtrToInt(part, IGF.IGM.IntPtrTy));

    for (unsigned i = 0; i != getNumStoredProtocols(); ++i) {
      part = In.claimNext();
      Out.add(IGF.Builder.CreatePtrToInt(part, IGF.IGM.IntPtrTy));
    }
  }

  // Given an exploded enum payload consisting of consecutive word-sized
  // chunks, cast them to their underlying component types.
  // FIXME: Assumes the payload is word-chunked. Should use
  void decomposeExplosion(Explosion &InE, Explosion &OutE,
                          IRGenFunction &IGF) const {
    // The first entry is always the weak*.
    toolchain::Value *weak = InE.claimNext();
    if (Refcounting == ReferenceCounting::Native)
      OutE.add(IGF.Builder.CreateBitOrPointerCast(weak,
                                          IGF.IGM.RefCountedPtrTy));
    else
      OutE.add(IGF.Builder.CreateBitOrPointerCast(weak,
                                          IGF.IGM.UnknownRefCountedPtrTy));

    // Collect the witness tables.
    for (unsigned i = 0, e = getNumStoredProtocols(); i != e; ++i) {
      toolchain::Value *witness = InE.claimNext();
      OutE.add(IGF.Builder.CreateBitOrPointerCast(witness,
                                                  IGF.IGM.WitnessTablePtrTy));
    }
  }

public:

  toolchain::PointerType *getPayloadType() const {
    auto *ty = getStorageType();
    toolchain::StructType *structTy = cast<toolchain::StructType>(ty);
    return cast<toolchain::PointerType>(structTy->elements()[0]);
  }

  bool isSingleRetainablePointer(ResilienceExpansion expansion,
                                 ReferenceCounting *refcounting) const override{
    if (refcounting) *refcounting = Refcounting;
    return getNumStoredProtocols() == 0;
  }
  
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override {
    return index == 0;
  }

  const LoadableTypeInfo &
  getValueTypeInfoForExtraInhabitants(IRGenModule &IGM) const {
    if (Refcounting == ReferenceCounting::Native)
      return IGM.getNativeObjectTypeInfo();
    else
      return IGM.getUnknownObjectTypeInfo();
  }

  void strongRetain(IRGenFunction &IGF, Explosion &e,
                    Atomicity atomicity) const override {
    IGF.emitStrongRetain(e.claimNext(), Refcounting, atomicity);
    (void)e.claim(getNumStoredProtocols());
  }

  void strongRelease(IRGenFunction &IGF, Explosion &e,
                     Atomicity atomicity) const override {
    IGF.emitStrongRelease(e.claimNext(), Refcounting, atomicity);
    (void)e.claim(getNumStoredProtocols());
  }

  virtual ReferenceCounting getReferenceCountingType() const override {
    return Refcounting;
  }

// We can just re-use the reference storage types.
#define NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  void name##LoadStrong(IRGenFunction &IGF, Address existential, \
                        Explosion &out, bool isOptional) const override { \
    if (isOptional) { \
      Explosion temp; \
      Address valueAddr = projectValue(IGF, existential); \
      toolchain::Type *resultType = IGF.IGM.getReferenceType(Refcounting); \
      temp.add(IGF.emit##Name##LoadStrong(valueAddr, resultType, Refcounting));\
      emitLoadOfTables(IGF, existential, temp); \
      mergeExplosion(temp, out, IGF); \
    } else { \
      Address valueAddr = projectValue(IGF, existential); \
      out.add(IGF.emit##Name##LoadStrong(valueAddr, \
                                         getPayloadType(), \
                                         Refcounting)); \
      emitLoadOfTables(IGF, existential, out); \
    } \
  } \
  void name##TakeStrong(IRGenFunction &IGF, Address existential, \
                         Explosion &out, bool isOptional) const override { \
    if (isOptional) { \
      Explosion temp; \
      Address valueAddr = projectValue(IGF, existential); \
      toolchain::Type *resultType = IGF.IGM.getReferenceType(Refcounting); \
      temp.add(IGF.emit##Name##TakeStrong(valueAddr, resultType, Refcounting));\
      emitLoadOfTables(IGF, existential, temp); \
      mergeExplosion(temp, out, IGF); \
    } else { \
      Address valueAddr = projectValue(IGF, existential); \
      out.add(IGF.emit##Name##TakeStrong(valueAddr, \
                                         getPayloadType(), \
                                         Refcounting)); \
      emitLoadOfTables(IGF, existential, out); \
    } \
  } \
  void name##Init(IRGenFunction &IGF, Explosion &in, \
                  Address existential, bool isOptional) const override { \
    toolchain::Value *value = nullptr; \
    if (isOptional) { \
      Explosion temp; \
      decomposeExplosion(in, temp, IGF); \
      value = temp.claimNext(); \
      assert(value->getType() == IGF.IGM.getReferenceType(Refcounting)); \
      emitStoreOfTables(IGF, temp, existential); \
    } else { \
      value = in.claimNext(); \
      emitStoreOfTables(IGF, in, existential); \
    } \
    Address valueAddr = projectValue(IGF, existential); \
    IGF.emit##Name##Init(value, valueAddr, Refcounting); \
  } \
  void name##Assign(IRGenFunction &IGF, Explosion &in, \
                    Address existential, bool isOptional) const override { \
    toolchain::Value *value = nullptr; \
    if (isOptional) { \
      Explosion temp; \
      decomposeExplosion(in, temp, IGF); \
      value = temp.claimNext(); \
      assert(value->getType() == IGF.IGM.getReferenceType(Refcounting)); \
      emitStoreOfTables(IGF, temp, existential); \
    } else { \
      value = in.claimNext(); \
      emitStoreOfTables(IGF, in, existential); \
    } \
    Address valueAddr = projectValue(IGF, existential); \
    IGF.emit##Name##Assign(value, valueAddr, Refcounting); \
  }
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  void name##Retain(IRGenFunction &IGF, Explosion &e, \
                    Atomicity atomicity) const override { \
    IGF.emit##Name##Retain(e.claimNext(), Refcounting, atomicity); \
    (void)e.claim(getNumStoredProtocols()); \
  } \
  void name##Release(IRGenFunction &IGF, Explosion &e, \
                     Atomicity atomicity) const override { \
    IGF.emit##Name##Release(e.claimNext(), Refcounting, atomicity); \
    (void)e.claim(getNumStoredProtocols()); \
  } \
  void strongRetain##Name(IRGenFunction &IGF, Explosion &e, \
                          Atomicity atomicity) const override { \
    IGF.emitStrongRetain##Name(e.claimNext(), Refcounting, atomicity); \
    (void)e.claim(getNumStoredProtocols()); \
  } \
  void strongRetain##Name##Release(IRGenFunction &IGF, \
                                   Explosion &e, \
                                   Atomicity atomicity) const override { \
    IGF.emitStrongRetainAnd##Name##Release(e.claimNext(), Refcounting, \
                                           atomicity); \
    (void)e.claim(getNumStoredProtocols()); \
  }
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...)                    \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name)                        \
  const TypeInfo *create##Name##StorageType(TypeConverter &TC,                 \
                                            bool isOptional) const override {  \
    auto spareBits = BitPatternBuilder(TC.IGM.Triple.isLittleEndian());        \
    auto ref = TC.IGM.getReferenceStorageSpareBits(ReferenceOwnership::Name,   \
                                                   Refcounting);               \
    spareBits.append(ref);                                                     \
    for (unsigned i = 0, e = getNumStoredProtocols(); i != e; ++i) {           \
      spareBits.append(TC.IGM.getWitnessTablePtrSpareBits());                  \
    }                                                                          \
    auto storageTy =                                                           \
        buildReferenceStorageType(TC.IGM, TC.IGM.Name##ReferenceStructTy);     \
    return AddressOnly##Name##ClassExistentialTypeInfo::create(                \
        getStoredProtocols(), storageTy, spareBits.build(), getFixedSize(),    \
        getFixedAlignment(), Refcounting, isOptional);                         \
  }
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...)                \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name)                        \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name)                       \
  const TypeInfo *create##Name##StorageType(TypeConverter &TC,                 \
                                            bool isOptional) const override {  \
    auto ref = TC.IGM.getReferenceStorageSpareBits(ReferenceOwnership::Name,   \
                                                   Refcounting);               \
    auto spareBits = BitPatternBuilder(TC.IGM.Triple.isLittleEndian());        \
    spareBits.append(ref);                                                     \
    for (unsigned i = 0, e = getNumStoredProtocols(); i != e; ++i) {           \
      spareBits.append(TC.IGM.getWitnessTablePtrSpareBits());                  \
    }                                                                          \
    auto storageTy =                                                           \
        buildReferenceStorageType(TC.IGM, TC.IGM.Name##ReferenceStructTy);     \
    if (TC.IGM.isLoadableReferenceAddressOnly(Refcounting)) {                  \
      return AddressOnly##Name##ClassExistentialTypeInfo::create(              \
          getStoredProtocols(), storageTy, spareBits.build(), getFixedSize(),  \
          getFixedAlignment(), Refcounting, isOptional);                       \
    } else {                                                                   \
      return Loadable##Name##ClassExistentialTypeInfo::create(                 \
          getStoredProtocols(), getValueType(), storageTy, spareBits.build(),  \
          getFixedSize(), getFixedAlignment(), Refcounting, isOptional);       \
    }                                                                          \
  }
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  const TypeInfo * \
  create##Name##StorageType(TypeConverter &TC, \
                            bool isOptional) const override { \
    assert(Refcounting == ReferenceCounting::Native); \
    auto ref = TC.IGM.getReferenceStorageSpareBits( \
                                                   ReferenceOwnership::Name, \
                                                   ReferenceCounting::Native); \
    auto spareBits = BitPatternBuilder(TC.IGM.Triple.isLittleEndian()); \
    spareBits.append(ref); \
    for (unsigned i = 0, e = getNumStoredProtocols(); i != e; ++i) { \
      spareBits.append(TC.IGM.getWitnessTablePtrSpareBits()); \
    } \
    auto storageTy = buildReferenceStorageType(TC.IGM, \
                              TC.IGM.Name##ReferencePtrTy->getElementType()); \
    return Loadable##Name##ClassExistentialTypeInfo::create( \
                                                  getStoredProtocols(), \
                                                  getValueType(), \
                                                  storageTy, \
                                                  spareBits.build(), \
                                                  getFixedSize(), \
                                                  getFixedAlignment(), \
                                                  ReferenceCounting::Native, \
                                                  isOptional); \
  }
#define UNCHECKED_REF_STORAGE(Name, ...) \
  const TypeInfo * \
  create##Name##StorageType(TypeConverter &TC, \
                            bool isOptional) const override { \
    return Name##ClassExistentialTypeInfo::create(getStoredProtocols(), \
                                                  getStorageType(), \
                                                  getSpareBits(), \
                                                  getFixedSize(), \
                                                  getFixedAlignment(), \
                                                  isOptional); \
  }
#include "language/AST/ReferenceStorage.def"
#undef NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER
#undef ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER

  void emitValueRetain(IRGenFunction &IGF, toolchain::Value *value,
                       Atomicity atomicity) const {
    IGF.emitStrongRetain(value, Refcounting, atomicity);
  }

  void emitValueRelease(IRGenFunction &IGF, toolchain::Value *value,
                        Atomicity atomicity) const {
    IGF.emitStrongRelease(value, Refcounting, atomicity);
  }

  void emitValueFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const {
    IGF.emitFixLifetime(value);
  }

  LoadedRef loadRefcountedPtr(IRGenFunction &IGF, SourceLoc loc,
                              Address existential) const override {
    Address valueAddr = projectValue(IGF, existential);
    return LoadedRef(IGF.emitLoadRefcountedPtr(valueAddr, Refcounting), true, Refcounting);
  }

  toolchain::StructType *buildReferenceStorageType(IRGenModule &IGM,
                                              toolchain::Type *refStorageTy) const {
    SmallVector<toolchain::Type*, 8> fieldTys;
    fieldTys.push_back(refStorageTy);
    fieldTys.resize(getNumStoredProtocols() + 1, IGM.WitnessTablePtrTy);
    auto storageTy = toolchain::StructType::get(IGM.getLLVMContext(), fieldTys);
    return storageTy;
  }
};

/// A type implementation for existential metatypes.
class ExistentialMetatypeTypeInfo final
  : public ScalarExistentialTypeInfoBase<ExistentialMetatypeTypeInfo,
                                         LoadableTypeInfo> {
  const LoadableTypeInfo &MetatypeTI;

  friend ExistentialTypeInfoBase;
  ExistentialMetatypeTypeInfo(ArrayRef<const ProtocolDecl *> storedProtocols,
                              toolchain::Type *ty, Size size,
                              SpareBitVector &&spareBits,
                              Alignment align,
                              const LoadableTypeInfo &metatypeTI)
    : ScalarExistentialTypeInfoBase(storedProtocols, ty, size,
                                    std::move(spareBits), align,
                                    IsTriviallyDestroyable,
                                    IsCopyable,
                                    IsFixedSize, IsABIAccessible),
      MetatypeTI(metatypeTI) {}

public:
  const LoadableTypeInfo &
  getValueTypeInfoForExtraInhabitants(IRGenModule &IGM) const {
    return MetatypeTI;
  }

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }
    return IGM.typeLayoutCache.getOrCreateScalarEntry(*this, T,
                                                      ScalarKind::TriviallyDestroyable);
  }

  void emitValueRetain(IRGenFunction &IGF, toolchain::Value *value,
                       Atomicity atomicity) const {
    // do nothing
  }

  void emitValueRelease(IRGenFunction &IGF, toolchain::Value *value,
                        Atomicity atomicity) const {
    // do nothing
  }

  void emitValueFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const {
    // do nothing
  }
};

/// Type info for error existentials, currently the only kind of boxed
/// existential.
class ErrorExistentialTypeInfo : public HeapTypeInfo<ErrorExistentialTypeInfo>
{
  const ProtocolDecl *ErrorProto;
  ReferenceCounting Refcounting;

public:
  ErrorExistentialTypeInfo(toolchain::PointerType *storage,
                           Size size, SpareBitVector spareBits,
                           Alignment align,
                           const ProtocolDecl *errorProto,
                           ReferenceCounting refcounting)
    : HeapTypeInfo(refcounting, storage, size, spareBits, align),
      ErrorProto(errorProto), Refcounting(refcounting) {}

  TypeLayoutEntry
  *buildTypeLayoutEntry(IRGenModule &IGM,
                        SILType T,
                        bool useStructLayouts) const override {
    if (!useStructLayouts) {
      return IGM.typeLayoutCache.getOrCreateTypeInfoBasedEntry(*this, T);
    }
    ScalarKind kind;
    switch (Refcounting) {
    case ReferenceCounting::Native:
      kind = ScalarKind::NativeStrongReference;
      break;
    case ReferenceCounting::Error:
      kind = ScalarKind::ErrorReference;
      break;
    default:
      toolchain_unreachable("unsupported refcounting type");
    }
    return IGM.typeLayoutCache.getOrCreateScalarEntry(*this, T, kind);
  }

  ReferenceCounting getReferenceCounting() const {
    // Error uses its own RC entry points.
    return Refcounting;
  }
  
  ArrayRef<const ProtocolDecl *> getStoredProtocols() const {
    return ErrorProto;
  }
};
  
} // end anonymous namespace

static const TypeInfo *
createErrorExistentialTypeInfo(IRGenModule &IGM,
                               const ExistentialLayout &layout) {
  // The Error existential has a special boxed representation. It has
  // space only for witnesses to the Error protocol.
  assert(layout.isErrorExistential());
  auto *protocol = layout.getProtocols()[0];

  auto refcounting = (!IGM.ObjCInterop
                      ? ReferenceCounting::Native
                      : ReferenceCounting::Error);

  return new ErrorExistentialTypeInfo(IGM.ErrorPtrTy,
                                      IGM.getPointerSize(),
                                      IGM.getHeapObjectSpareBits(),
                                      IGM.getPointerAlignment(),
                                      protocol,
                                      refcounting);
}

toolchain::Type *TypeConverter::getExistentialType(unsigned numWitnessTables) {
  toolchain::StructType *&type = OpaqueExistentialTypes[numWitnessTables];
  if (type)
    return type;
  
  SmallVector<toolchain::Type*, 5> fields;

  fields.push_back(IGM.getFixedBufferTy());
  fields.push_back(IGM.TypeMetadataPtrTy);

  for (auto i : range(numWitnessTables)) {
    fields.push_back(IGM.WitnessTablePtrTy);
    (void) i;
  }

  toolchain::SmallString<40> typeName;
  toolchain::raw_svector_ostream(typeName)
      << "__opaque_existential_type_"
      << numWitnessTables;

  type = toolchain::StructType::create(IGM.getLLVMContext(), StringRef(typeName));
  type->setBody(fields);

  return type;
}

toolchain::PointerType *IRGenModule::getExistentialPtrTy(unsigned numTables) {
  return Types.getExistentialType(numTables)->getPointerTo();
}

toolchain::Type *IRGenModule::getExistentialType(unsigned numTables) {
  return Types.getExistentialType(numTables);
}

static const TypeInfo *createExistentialTypeInfo(IRGenModule &IGM, CanType T) {
  auto layout = T.getExistentialLayout();

  SmallVector<toolchain::Type*, 5> fields;
  SmallVector<const ProtocolDecl *, 4> protosWithWitnessTables;

  // Check for special existentials.
  if (layout.isErrorExistential()) {
    // Error has a special runtime representation.
    return createErrorExistentialTypeInfo(IGM, layout);
  }

  toolchain::StructType *type;

  // Note: Protocol composition types are not nominal, but we name them
  // anyway.
  if (auto existential = T->getAs<ExistentialType>()) {
    T = existential->getConstraintType()->getCanonicalType();
  }

  if (isa<ProtocolType>(T) || isa<ParameterizedProtocolType>(T))
    type = IGM.createNominalType(T);
  else
    type = IGM.createNominalType(cast<ProtocolCompositionType>(T.getPointer()));

  assert(type->isOpaque() && "creating existential type in concrete struct");

  // In an opaque metadata, the first two fields are the fixed buffer
  // followed by the metadata reference.  In a class metadata, the
  // first field is the class instance.
  //
  // Leave space in the buffer for both, but make sure we set it up later.
  fields.push_back(nullptr);
  fields.push_back(nullptr);

  // The existential container is class-constrained if any of its protocol
  // constraints are.
  bool allowsTaggedPointers = true;

  for (auto protoDecl : layout.getProtocols()) {
    if (protoDecl->getAttrs().hasAttribute<UnsafeNoObjCTaggedPointerAttr>())
      allowsTaggedPointers = false;

    // ObjC protocols need no layout or witness table info. All dispatch is done
    // through objc_msgSend.
    if (!Lowering::TypeConverter::protocolRequiresWitnessTable(protoDecl))
      continue;

    // Each protocol gets a witness table.
    protosWithWitnessTables.push_back(protoDecl);
    fields.push_back(IGM.WitnessTablePtrTy);
  }

  // If the existential is class, lower it to a class
  // existential representation.
  if (layout.requiresClass()) {
    // If we're not using the Objective-C runtime, we can use the
    // native reference counting entry points.
    ReferenceCounting refcounting = T->getReferenceCounting();

    toolchain::PointerType *reprTy = nullptr;
    if (auto superclass = layout.getSuperclass()) {
      auto &superTI = IGM.getTypeInfoForUnlowered(superclass);
      reprTy = cast<toolchain::PointerType>(superTI.getStorageType());
    } else if (refcounting == ReferenceCounting::Native) {
      reprTy = IGM.RefCountedPtrTy;
    } else {
      reprTy = IGM.UnknownRefCountedPtrTy;
    }

    fields[1] = reprTy;

    // Replace the type metadata pointer with the class instance.
    auto classFields = toolchain::ArrayRef(fields).slice(1);
    type->setBody(classFields);

    Alignment align = IGM.getPointerAlignment();
    Size size = classFields.size() * IGM.getPointerSize();

    auto spareBits = BitPatternBuilder(IGM.Triple.isLittleEndian());

    // The class pointer is an unknown heap object, so it may be a tagged
    // pointer, if the platform has those.
    if (allowsTaggedPointers &&
        refcounting != ReferenceCounting::Native &&
        IGM.TargetInfo.hasObjCTaggedPointers()) {
      spareBits.appendClearBits(IGM.getPointerSize().getValueInBits());
    } else {
      // If the platform doesn't use ObjC tagged pointers, we can go crazy.
      spareBits.append(IGM.getHeapObjectSpareBits());
    }

    for (unsigned i = 1, e = classFields.size(); i < e; ++i) {
      spareBits.append(IGM.getWitnessTablePtrSpareBits());
    }
    return ClassExistentialTypeInfo::create(protosWithWitnessTables, type,
                                            size, spareBits.build(), align,
                                            refcounting);
  }

  // Set up the first two fields.
  fields[0] = IGM.getFixedBufferTy();
  fields[1] = IGM.TypeMetadataPtrTy;
  type->setBody(fields);

  OpaqueExistentialLayout opaque(protosWithWitnessTables.size());
  Alignment align = opaque.getAlignment(IGM);
  Size size = opaque.getSize(IGM);
  IsCopyable_t copyable = T->isNoncopyable() ? IsNotCopyable : IsCopyable;
  
  // There are spare bits in the metadata pointer and witness table pointers
  // consistent with a native object reference.
  // NB: There are spare bits we could theoretically use in the type metadata
  // and witness table pointers, but opaque existentials are address-
  // only, and we can't soundly take advantage of spare bits for in-memory
  // representations.
  // Maybe an ABI break that made opaque existentials loadable could use those
  // bits, though.
  auto spareBits = SpareBitVector::getConstant(size.getValueInBits(), false);
  return OpaqueExistentialTypeInfo::create(protosWithWitnessTables, type, size,
                                           copyable,
                                           std::move(spareBits),
                                           align);
}

const TypeInfo *TypeConverter::convertProtocolType(ProtocolType *T) {
  return createExistentialTypeInfo(IGM, CanType(T));
}

const TypeInfo *
TypeConverter::convertProtocolCompositionType(ProtocolCompositionType *T) {
  return createExistentialTypeInfo(IGM, CanType(T));
}

const TypeInfo *
TypeConverter::convertParameterizedProtocolType(ParameterizedProtocolType *T) {
  return createExistentialTypeInfo(IGM, CanType(T));
}

const TypeInfo *
TypeConverter::convertExistentialType(ExistentialType *T) {
  return createExistentialTypeInfo(IGM, CanType(T));
}

const TypeInfo *
TypeConverter::convertExistentialMetatypeType(ExistentialMetatypeType *T) {
  assert(T->hasRepresentation() &&
         "metatype should have been assigned a representation by SIL");

  auto instanceT = CanExistentialMetatypeType(T).getInstanceType();
  while (isa<ExistentialMetatypeType>(instanceT))
    instanceT = cast<ExistentialMetatypeType>(instanceT).getInstanceType();

  auto layout = instanceT.getExistentialLayout();

  SmallVector<const ProtocolDecl *, 4> protosWithWitnessTables;
  SmallVector<toolchain::Type*, 4> fields;

  assert(T->getRepresentation() != MetatypeRepresentation::Thin &&
         "existential metatypes cannot have thin representation");
  auto &baseTI = cast<LoadableTypeInfo>(getMetatypeTypeInfo(T->getRepresentation()));
  fields.push_back(baseTI.getStorageType());

  auto spareBits = BitPatternBuilder(IGM.Triple.isLittleEndian());
  spareBits.append(baseTI.getSpareBits());

  for (auto protoDecl : layout.getProtocols()) {
    if (!Lowering::TypeConverter::protocolRequiresWitnessTable(protoDecl))
      continue;

    // Each protocol gets a witness table.
    protosWithWitnessTables.push_back(protoDecl);
    fields.push_back(IGM.WitnessTablePtrTy);
    spareBits.append(IGM.getWitnessTablePtrSpareBits());
  }

  toolchain::StructType *type = toolchain::StructType::get(IGM.getLLVMContext(), fields);

  Size size = IGM.getPointerSize() * fields.size();
  Alignment align = IGM.getPointerAlignment();

  return ExistentialMetatypeTypeInfo::create(protosWithWitnessTables, type,
                                             size, spareBits.build(), align,
                                             baseTI);
}

static void setMetadataRef(IRGenFunction &IGF,
                           ArchetypeType *archetype,
                           toolchain::Value *metadata,
                           MetadataState metadataState) {
  assert(metadata->getType() == IGF.IGM.TypeMetadataPtrTy);
  IGF.setUnscopedLocalTypeMetadata(CanType(archetype),
                         MetadataResponse::forBounded(metadata, metadataState));
}

static void setWitnessTable(IRGenFunction &IGF,
                            ArchetypeType *archetype,
                            unsigned protocolIndex,
                            toolchain::Value *wtable) {
  assert(wtable->getType() == IGF.IGM.WitnessTablePtrTy);
  assert(protocolIndex < archetype->getConformsTo().size());
  auto protocol = archetype->getConformsTo()[protocolIndex];
  IGF.setUnscopedLocalTypeData(CanType(archetype),
                  LocalTypeDataKind::forAbstractProtocolWitnessTable(protocol),
                               wtable);
}

/// Inform IRGenFunction that the given archetype has the given value
/// witness value within this scope.
static void bindArchetype(IRGenFunction &IGF,
                          ArchetypeType *archetype,
                          toolchain::Value *metadata,
                          MetadataState metadataState,
                          ArrayRef<toolchain::Value*> wtables) {
  // Set the metadata pointer.
  setTypeMetadataName(IGF.IGM, metadata, CanType(archetype));
  setMetadataRef(IGF, archetype, metadata, metadataState);

  // Set the protocol witness tables.

  unsigned wtableI = 0;
  for (unsigned i = 0, e = archetype->getConformsTo().size(); i != e; ++i) {
    auto proto = archetype->getConformsTo()[i];
    if (!Lowering::TypeConverter::protocolRequiresWitnessTable(proto))
      continue;
    auto wtable = wtables[wtableI++];
    setProtocolWitnessTableName(IGF.IGM, wtable, CanType(archetype), proto);
    setWitnessTable(IGF, archetype, i, wtable);
  }
  assert(wtableI == wtables.size());
}

/// Emit protocol witness table pointers for the given protocol conformances,
/// passing each emitted witness table index into the given function body.
static void forEachProtocolWitnessTable(
    IRGenFunction &IGF, CanType srcType, toolchain::Value **srcMetadataCache,
    CanType destType, ArrayRef<const ProtocolDecl *> protocols,
    ArrayRef<ProtocolConformanceRef> conformances,
    toolchain::function_ref<void(unsigned, toolchain::Value *)> body) {
  // Collect the conformances that need witness tables.
  auto layout = destType.getExistentialLayout();
  auto destProtocols = layout.getProtocols();

  SmallVector<ProtocolConformanceRef, 2> witnessConformances;
  assert(destProtocols.size() == conformances.size() &&
         "mismatched protocol conformances");
  for (unsigned i = 0, size = destProtocols.size(); i < size; ++i) {
    auto destProtocol = destProtocols[i];
    if (Lowering::TypeConverter::protocolRequiresWitnessTable(destProtocol))
      witnessConformances.push_back(conformances[i]);
  }

  assert(protocols.size() == witnessConformances.size() &&
         "mismatched protocol conformances");

  for (unsigned i = 0, e = protocols.size(); i < e; ++i) {
    assert(protocols[i] == witnessConformances[i].getProtocol());
    auto table = emitWitnessTableRef(IGF, srcType, srcMetadataCache,
                                     witnessConformances[i]);
    body(i, table);
  }
}

/// Project the address of the value inside a boxed existential container.
ContainedAddress irgen::emitBoxedExistentialProjection(IRGenFunction &IGF,
                                              Explosion &base,
                                              SILType baseTy,
                                              CanType projectedType) {
  // TODO: Non-ErrorType boxed existentials.
  assert(baseTy.canUseExistentialRepresentation(
           ExistentialRepresentation::Boxed, Type()));
  
  // Get the reference to the existential box.
  toolchain::Value *box = base.claimNext();
  // Allocate scratch space to invoke the runtime.
  Address scratch = IGF.createAlloca(IGF.IGM.Int8PtrTy,
                                     IGF.IGM.getPointerAlignment(),
                                     "project_error_scratch");
  Address out = IGF.createAlloca(IGF.IGM.OpenedErrorTripleTy,
                                 IGF.IGM.getPointerAlignment(),
                                 "project_error_out");

  IGF.Builder.CreateCall(IGF.IGM.getGetErrorValueFunctionPointer(),
                         {box, scratch.getAddress(), out.getAddress()});
  // Load the 'out' values.
  auto &projectedTI = IGF.getTypeInfoForLowered(projectedType);
  auto projectedPtrAddr = IGF.Builder.CreateStructGEP(out, 0, Size(0));
  toolchain::Value *projectedPtr = IGF.Builder.CreateLoad(projectedPtrAddr);
  projectedPtr = IGF.Builder.CreateBitCast(projectedPtr,
                               projectedTI.getStorageType()->getPointerTo());
  auto projected = projectedTI.getAddressForPointer(projectedPtr);
  return ContainedAddress(out, projected);
}

/// Project the address of the value inside a boxed existential container,
/// and open an archetype to its contained type.
Address irgen::emitOpenExistentialBox(IRGenFunction &IGF,
                                      Explosion &base,
                                      SILType baseTy,
                                      CanArchetypeType openedArchetype) {
  ContainedAddress box = emitBoxedExistentialProjection(IGF, base, baseTy,
                                                        openedArchetype);
  Address out = box.getContainer();
  auto metadataAddr = IGF.Builder.CreateStructGEP(out, 1,
                                                  IGF.IGM.getPointerSize());
  auto metadata = IGF.Builder.CreateLoad(metadataAddr);
  auto witnessAddr = IGF.Builder.CreateStructGEP(out, 2,
                                                 2 * IGF.IGM.getPointerSize());
  auto witness = IGF.Builder.CreateLoad(witnessAddr);
  
  bindArchetype(IGF, openedArchetype, metadata, MetadataState::Complete,
                witness);
  return box.getAddress();
}

/// Allocate a boxed existential container with uninitialized space to hold a
/// value of a given type.
OwnedAddress irgen::emitBoxedExistentialContainerAllocation(IRGenFunction &IGF,
                                  SILType destType,
                                  CanType formalSrcType,
                                  ArrayRef<ProtocolConformanceRef> conformances) {
  // TODO: Non-Error boxed existentials.
  assert(destType.canUseExistentialRepresentation(
           ExistentialRepresentation::Boxed, Type()));

  auto &destTI = IGF.getTypeInfo(destType).as<ErrorExistentialTypeInfo>();
  auto srcMetadata = IGF.emitTypeMetadataRef(formalSrcType);
  // Should only be one conformance, for the Error protocol.
  assert(conformances.size() == 1 && destTI.getStoredProtocols().size() == 1);
  const ProtocolDecl *proto = destTI.getStoredProtocols()[0];
  (void) proto;
  assert(proto == conformances[0].getProtocol());
  auto witness = emitWitnessTableRef(IGF, formalSrcType, &srcMetadata,
                                     conformances[0]);
  
  // Call the runtime to allocate the box.
  // TODO: When there's a store or copy_addr immediately into the box, peephole
  // it into the initializer parameter to allocError.
  auto result = IGF.Builder.CreateCall(
      IGF.IGM.getAllocErrorFunctionPointer(),
      {srcMetadata, witness,
       toolchain::ConstantPointerNull::get(IGF.IGM.OpaquePtrTy),
       toolchain::ConstantInt::get(IGF.IGM.Int1Ty, 0)});

  // Extract the box and value address from the result.
  auto box = IGF.Builder.CreateExtractValue(result, 0);
  auto addr = IGF.Builder.CreateExtractValue(result, 1);

  auto archetype =
      ExistentialArchetypeType::get(destType.getASTType());
  auto &srcTI = IGF.getTypeInfoForUnlowered(AbstractionPattern(archetype),
                                            formalSrcType);
  addr = IGF.Builder.CreateBitCast(addr,
                                   srcTI.getStorageType()->getPointerTo());
  return OwnedAddress(srcTI.getAddressForPointer(addr), box);
}

/// Deallocate a boxed existential container with uninitialized space to hold a
/// value of a given type.
void irgen::emitBoxedExistentialContainerDeallocation(IRGenFunction &IGF,
                                                      Explosion &container,
                                                      SILType containerType,
                                                      CanType valueType) {
  // TODO: Non-Error boxed existentials.
  assert(containerType.canUseExistentialRepresentation(
           ExistentialRepresentation::Boxed, Type()));

  auto box = container.claimNext();
  auto srcMetadata = IGF.emitTypeMetadataRef(valueType);

  IGF.Builder.CreateCall(IGF.IGM.getDeallocErrorFunctionPointer(),
                         {box, srcMetadata});
}

/// Emit a class existential container from a class instance value
/// as an explosion.
void irgen::emitClassExistentialContainer(IRGenFunction &IGF,
                               Explosion &out,
                               SILType outType,
                               toolchain::Value *instance,
                               CanType instanceFormalType,
                               SILType instanceLoweredType,
                               ArrayRef<ProtocolConformanceRef> conformances) {
  // As a special case, an Error existential can be represented as a
  // reference to an already existing NSError or CFError instance.
  if (outType.isExistentialType()) {
    auto layout = outType.getASTType().getExistentialLayout();
    if (layout.isErrorExistential()) {
      // Bitcast the incoming class reference to Error.
      out.add(IGF.Builder.CreateBitCast(instance, IGF.IGM.ErrorPtrTy));
      return;
    }
  }
  
  assert(outType.isClassExistentialType() &&
         "creating a non-class existential type");

  auto &destTI = IGF.getTypeInfo(outType).as<ClassExistentialTypeInfo>();

  // Cast the instance pointer to an opaque refcounted pointer.
  auto opaqueInstance = IGF.Builder.CreateBitCast(instance,
                                               destTI.getPayloadType());
  out.add(opaqueInstance);

  // Emit the witness table pointers.
  toolchain::Value *instanceMetadata = nullptr;
  forEachProtocolWitnessTable(IGF, instanceFormalType, &instanceMetadata,
                              outType.getASTType(),
                              destTI.getStoredProtocols(),
                              conformances,
                              [&](unsigned i, toolchain::Value *ptable) {
    out.add(ptable);
  });
}

#ifndef NDEBUG
static size_t numProtocolsWithWitnessTables(
    ArrayRef<ProtocolConformanceRef> conformances) {
  return toolchain::count_if(conformances, [](ProtocolConformanceRef conformance) {
    auto proto = conformance.getProtocol();
    return Lowering::TypeConverter::protocolRequiresWitnessTable(proto);
  });
}
#endif

/// Emit an existential container initialization operation for a concrete type.
/// Returns the address of the uninitialized fixed-size buffer for the concrete
/// value.
Address irgen::emitOpaqueExistentialContainerInit(IRGenFunction &IGF,
                                  Address dest,
                                  SILType destType,
                                  CanType formalSrcType,
                                  SILType loweredSrcType,
                                  ArrayRef<ProtocolConformanceRef> conformances) {
  assert(!destType.isClassExistentialType() &&
         "initializing a class existential container as opaque");
  auto &destTI = IGF.getTypeInfo(destType).as<OpaqueExistentialTypeInfo>();
  OpaqueExistentialLayout destLayout = destTI.getLayout();
  assert(destTI.getStoredProtocols().size()
             == numProtocolsWithWitnessTables(conformances));

  // First, write out the metadata.
  toolchain::Value *metadata = IGF.emitTypeMetadataRef(formalSrcType);
  IGF.Builder.CreateStore(metadata, destLayout.projectMetadataRef(IGF, dest));


  // Next, write the protocol witness tables.
  forEachProtocolWitnessTable(IGF, formalSrcType, &metadata,
                              destType.getASTType(),
                              destTI.getStoredProtocols(), conformances,
                              [&](unsigned i, toolchain::Value *ptable) {
    Address ptableSlot = destLayout.projectWitnessTable(IGF, dest, i);
    IGF.Builder.CreateStore(ptable, ptableSlot);
  });

  // Finally, evaluate into the buffer.

  // Project down to the destination fixed-size buffer.
  return destLayout.projectExistentialBuffer(IGF, dest);
}

/// Emit an existential metatype container from a metatype value
/// as an explosion.
void irgen::emitExistentialMetatypeContainer(IRGenFunction &IGF,
                               Explosion &out, SILType outType,
                               toolchain::Value *metatype, SILType metatypeType,
                               ArrayRef<ProtocolConformanceRef> conformances) {
  assert(outType.is<ExistentialMetatypeType>());
  auto &destTI = IGF.getTypeInfo(outType).as<ExistentialMetatypeTypeInfo>();
  out.add(metatype);

  auto srcType = metatypeType.castTo<MetatypeType>().getInstanceType();
  auto destType = outType.castTo<ExistentialMetatypeType>().getInstanceType();
  while (auto destMetatypeType = dyn_cast<ExistentialMetatypeType>(destType)) {
    destType = destMetatypeType.getInstanceType();
    srcType = cast<AnyMetatypeType>(srcType).getInstanceType();
  }

  // Emit the witness table pointers.
  toolchain::Value *srcMetadata = nullptr;
  forEachProtocolWitnessTable(IGF, srcType, &srcMetadata, destType,
                              destTI.getStoredProtocols(),
                              conformances,
                              [&](unsigned i, toolchain::Value *ptable) {
                                out.add(ptable);
                              });
}

void irgen::emitMetatypeOfOpaqueExistential(IRGenFunction &IGF, Address buffer,
                                            SILType type, Explosion &out) {
  assert(type.isExistentialType());
  assert(!type.isClassExistentialType());
  auto &baseTI = IGF.getTypeInfo(type).as<OpaqueExistentialTypeInfo>();

  // Get the static metadata.
  auto existLayout = baseTI.getLayout();
  toolchain::Value *metadata = existLayout.loadMetadataRef(IGF, buffer);

  // Project the buffer and apply the 'typeof' value witness.
  toolchain::Value *object;

  auto *addr = IGF.Builder.CreateBitCast(
      buffer.getAddress(),
      IGF.IGM.getExistentialPtrTy(existLayout.getNumTables()));
  auto *projectFunc = getProjectBoxedOpaqueExistentialFunction(
      IGF, OpenedExistentialAccess::Immutable, existLayout);
  auto *addrOfValue = IGF.Builder.CreateCallWithoutDbgLoc(
      projectFunc->getFunctionType(), projectFunc, {addr, metadata});
  addrOfValue->setCallingConv(IGF.IGM.DefaultCC);
  addrOfValue->setDoesNotThrow();
  object = addrOfValue;

  toolchain::Value *dynamicType = IGF.Builder.CreateCall(
      IGF.IGM.getGetDynamicTypeFunctionPointer(),
      {object, metadata, toolchain::ConstantInt::get(IGF.IGM.Int1Ty, 1)});
  out.add(dynamicType);

  // Get the witness tables.
  baseTI.emitLoadOfTables(IGF, buffer, out);
}

void irgen::emitMetatypeOfBoxedExistential(IRGenFunction &IGF, Explosion &value,
                                           SILType type, Explosion &out) {
  // TODO: Non-Error boxed existentials.
  assert(type.canUseExistentialRepresentation(
           ExistentialRepresentation::Boxed, Type()));

  // Get the reference to the existential box.
  toolchain::Value *box = value.claimNext();

  // Allocate scratch space to invoke the runtime.
  Address scratchAddr = IGF.createAlloca(IGF.IGM.Int8PtrTy,
                                         IGF.IGM.getPointerAlignment(),
                                         "project_error_scratch");
  Address outAddr = IGF.createAlloca(IGF.IGM.OpenedErrorTripleTy,
                                     IGF.IGM.getPointerAlignment(),
                                     "project_error_out");

  IGF.Builder.CreateCall(IGF.IGM.getGetErrorValueFunctionPointer(),
                         {box, scratchAddr.getAddress(), outAddr.getAddress()});

  auto projectedPtrAddr = IGF.Builder.CreateStructGEP(outAddr, 0, Size(0));
  auto projectedPtr = IGF.Builder.CreateLoad(projectedPtrAddr);

  auto metadataAddr = IGF.Builder.CreateStructGEP(outAddr, 1,
                                                  IGF.IGM.getPointerSize());
  auto metadata = IGF.Builder.CreateLoad(metadataAddr);

  auto dynamicType = IGF.Builder.CreateCall(
      IGF.IGM.getGetDynamicTypeFunctionPointer(),
      {projectedPtr, metadata, toolchain::ConstantInt::get(IGF.IGM.Int1Ty, 1)});

  auto witnessAddr = IGF.Builder.CreateStructGEP(outAddr, 2,
                                                 2 * IGF.IGM.getPointerSize());
  auto witness = IGF.Builder.CreateLoad(witnessAddr);

  out.add(dynamicType);
  out.add(witness);
}

void irgen::emitMetatypeOfClassExistential(IRGenFunction &IGF, Explosion &value,
                                           SILType metatypeTy,
                                           SILType existentialTy,
                                           Explosion &out) {
  assert(existentialTy.isClassExistentialType());
  auto &baseTI = IGF.getTypeInfo(existentialTy).as<ClassExistentialTypeInfo>();

  // Extract the class instance pointer.
  auto tablesAndValue = baseTI.getWitnessTablesAndValue(value);

  // Get the type metadata.
  toolchain::Value *instance = tablesAndValue.second;

  auto metaTy = metatypeTy.castTo<ExistentialMetatypeType>();
  auto repr = metaTy->getRepresentation();
  assert(repr != MetatypeRepresentation::Thin &&
         "Class metatypes should not have a thin representation");
  assert((IGF.IGM.ObjCInterop || repr != MetatypeRepresentation::ObjC) &&
         "Class metatypes should not have ObjC representation without runtime");

  auto dynamicType = emitDynamicTypeOfHeapObject(IGF, instance, repr,
                                                 existentialTy,
                                                 /*allow artificial*/ false);
  out.add(dynamicType);

  // Get the witness tables.
  out.add(tablesAndValue.first);
}

void irgen::emitMetatypeOfMetatype(IRGenFunction &IGF, Explosion &value,
                                           SILType existentialTy,
                                           Explosion &out) {
  assert(existentialTy.is<ExistentialMetatypeType>());
  auto &baseTI = IGF.getTypeInfo(existentialTy).as<ExistentialMetatypeTypeInfo>();

  auto tablesAndValue = baseTI.getWitnessTablesAndValue(value);

  toolchain::Value *dynamicType = IGF.Builder.CreateCall(
      IGF.IGM.getGetMetatypeMetadataFunctionPointer(), tablesAndValue.second);
  out.add(dynamicType);
  out.add(tablesAndValue.first);
}

Address
irgen::emitClassExistentialValueAddress(IRGenFunction &IGF, Address existential,
                                        SILType baseTy) {
  assert(baseTy.isClassExistentialType());
  auto &baseTI = IGF.getTypeInfo(baseTy).as<ClassExistentialTypeInfo>();
  return baseTI.projectValue(IGF, existential);
}

/// Extract the instance pointer from a class existential value.
toolchain::Value *
irgen::emitClassExistentialProjection(IRGenFunction &IGF,
                                      Explosion &base,
                                      SILType baseTy,
                                      CanArchetypeType openedArchetype) {
  assert(baseTy.isClassExistentialType());
  auto &baseTI = IGF.getTypeInfo(baseTy).as<ClassExistentialTypeInfo>();

  if (!openedArchetype)
    return baseTI.getValue(IGF, base);

  // Capture the metadata and witness tables from this existential
  // into the given archetype.
  ArrayRef<toolchain::Value*> wtables;
  toolchain::Value *value;
  std::tie(wtables, value) = baseTI.getWitnessTablesAndValue(base);
  auto metadata = emitDynamicTypeOfHeapObject(IGF, value,
                                              MetatypeRepresentation::Thick,
                                              baseTy,
                                              /*allow artificial*/ false);
  bindArchetype(IGF, openedArchetype, metadata, MetadataState::Complete,
                wtables);

  return value;
}

/// Extract the metatype pointer from a class existential value.
toolchain::Value *
irgen::emitExistentialMetatypeProjection(IRGenFunction &IGF,
                                         Explosion &base,
                                         SILType baseTy,
                                         CanType openedTy) {
  assert(baseTy.is<ExistentialMetatypeType>());
  auto &baseTI = IGF.getTypeInfo(baseTy).as<ExistentialMetatypeTypeInfo>();

  if (!openedTy)
    return baseTI.getValue(IGF, base);

  // Capture the metadata and witness tables from this existential
  // into the given archetype.
  ArrayRef<toolchain::Value*> wtables;
  toolchain::Value *value;
  std::tie(wtables, value) = baseTI.getWitnessTablesAndValue(base);

  auto existentialType = baseTy.castTo<ExistentialMetatypeType>();
  auto targetType = cast<MetatypeType>(openedTy);

  // If we're starting with an ObjC representation, convert it to a
  // class type and let's go.
  toolchain::Value *metatype;
  if (existentialType->getRepresentation() == MetatypeRepresentation::ObjC) {
    metatype = emitObjCMetadataRefForMetadata(IGF, value);

  // Otherwise, we have type metadata.
  } else {
    assert(existentialType->getRepresentation()
             == MetatypeRepresentation::Thick);
    metatype = value;

    // The type we need to bind to the archetype is the one that's
    // deep in the type.
    while (!isa<ArchetypeType>(targetType.getInstanceType())) {
      targetType = cast<MetatypeType>(targetType.getInstanceType());
      existentialType =
        cast<ExistentialMetatypeType>(existentialType.getInstanceType());
      metatype = emitMetatypeInstanceType(IGF, metatype);
    }
  }

  auto openedArchetype = cast<ArchetypeType>(targetType.getInstanceType());
  bindArchetype(IGF, openedArchetype, metatype, MetadataState::Complete,
                wtables);

  return value;
}

static Address castToOpaquePtr(IRGenFunction &IGF, Address addr) {
  return Address(
      IGF.Builder.CreateBitCast(addr.getAddress(), IGF.IGM.OpaquePtrTy),
      IGF.IGM.OpaqueTy, addr.getAlignment());
}

static toolchain::Function *getAllocateBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout) {

  toolchain::Type *argTys[] = {IGM.getExistentialPtrTy(existLayout.getNumTables())};

  // __language_allocate_boxed_opaque_existential__N is the well-known function for
  // allocating buffers in existential containers of types with N witness
  // tables.
  toolchain::SmallString<40> fnName;
  toolchain::raw_svector_ostream(fnName)
      << "__language_allocate_boxed_opaque_existential_"
      << existLayout.getNumTables();

  return cast<toolchain::Function>(IGM.getOrCreateHelperFunction(
      fnName, IGM.OpaquePtrTy, argTys,
      [&](IRGenFunction &IGF) {
        auto it = IGF.CurFn->arg_begin();
        Address existentialContainer(
            &*(it++), IGF.IGM.getExistentialType(existLayout.getNumTables()),
            existLayout.getAlignment(IGM));

        // Dynamically check whether this type is inline or needs an allocation.
        auto *metadata = existLayout.loadMetadataRef(IGF, existentialContainer);
        toolchain::Value *isInline, *flags;
        std::tie(isInline, flags) = emitLoadOfIsInline(IGF, metadata);
        toolchain::BasicBlock *doneBB = IGF.createBasicBlock("done");
        toolchain::BasicBlock *allocateBB = IGF.createBasicBlock("allocateBox");
        toolchain::Value *addressInBox;
        Address existentialBuffer =
            existLayout.projectExistentialBuffer(IGF, existentialContainer);
        toolchain::Value *addressInline = IGF.Builder.CreateBitCast(
            existentialBuffer.getAddress(), IGF.IGM.OpaquePtrTy);
        IGF.Builder.CreateCondBr(isInline, doneBB, allocateBB);

        IGF.Builder.emitBlock(doneBB);
        IGF.Builder.CreateRet(addressInline);

        // Use the runtime to allocate a box of the appropriate size.
        {
          IGF.Builder.emitBlock(allocateBB);
          ConditionalDominanceScope allocateCondition(IGF);
          toolchain::Value *box, *address;
          IGF.emitAllocBoxCall(metadata, box, address);
          addressInBox =
              IGF.Builder.CreateBitCast(address, IGF.IGM.OpaquePtrTy);
          IGF.Builder.CreateStore(
              box,
              Address(IGF.Builder.CreateBitCast(existentialBuffer.getAddress(),
                                                box->getType()->getPointerTo()),
                      IGF.IGM.RefCountedPtrTy,
                      existLayout.getAlignment(IGF.IGM)));
          IGF.Builder.CreateRet(addressInBox);
        }
      },
      true /*noinline*/));
}

Address irgen::emitAllocateBoxedOpaqueExistentialBuffer(
    IRGenFunction &IGF, SILType existentialType, SILType valueType,
    Address existentialContainer, GenericEnvironment *genericEnv,
    bool isOutlined) {

  // Project to the existential buffer in the existential container.
  auto &existentialTI =
      IGF.getTypeInfo(existentialType).as<OpaqueExistentialTypeInfo>();
  OpaqueExistentialLayout existLayout = existentialTI.getLayout();
  Address existentialBuffer =
      existLayout.projectExistentialBuffer(IGF, existentialContainer);

  auto &valueTI = IGF.getTypeInfo(valueType);
  auto *valuePointerType = valueTI.getStorageType()->getPointerTo();

  // Check if the value is fixed size.
  if (auto *fixedTI = dyn_cast<FixedTypeInfo>(&valueTI)) {
    // Don't allocate an out-of-line buffer if the fixed buffer size is
    // sufficient.
    if (fixedTI->getFixedPacking(IGF.IGM) == FixedPacking::OffsetZero) {
      return valueTI.getAddressForPointer(IGF.Builder.CreateBitCast(
          existentialBuffer.getAddress(), valuePointerType));
    }
    // Otherwise, allocate a box with enough storage.
    Address addr = emitAllocateExistentialBoxInBuffer(
        IGF, valueType, existentialBuffer, genericEnv, "exist.box.addr",
        isOutlined);
    return addr;
  }
  /// Call a function to handle the non-fixed case.
  auto *allocateFun = getAllocateBoxedOpaqueExistentialBufferFunction(
      IGF.IGM, existLayout);
  auto *existentialAddr = IGF.Builder.CreateBitCast(
      existentialContainer.getAddress(),
      IGF.IGM.getExistentialPtrTy(existLayout.getNumTables()));
  auto *call = IGF.Builder.CreateCallWithoutDbgLoc(
      allocateFun->getFunctionType(), allocateFun, {existentialAddr});
  call->setCallingConv(IGF.IGM.DefaultCC);
  call->setDoesNotThrow();
  auto addressOfValue = IGF.Builder.CreateBitCast(call, valuePointerType);
  return valueTI.getAddressForPointer(addressOfValue);
}

static toolchain::Function *getDeallocateBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout) {

  toolchain::Type *argTys[] = {IGM.getExistentialPtrTy(existLayout.getNumTables())};

  // __language_deallocate_boxed_opaque_existential_N is the well-known function
  // for deallocating buffers in existential containers of types with N witness
  // tables.
  toolchain::SmallString<40> fnName;
  toolchain::raw_svector_ostream(fnName)
      << "__language_deallocate_boxed_opaque_existential_"
      << existLayout.getNumTables();

  return cast<toolchain::Function>(IGM.getOrCreateHelperFunction(
      fnName, IGM.VoidTy, argTys,
      [&](IRGenFunction &IGF) {
        auto &Builder = IGF.Builder;
        auto it = IGF.CurFn->arg_begin();
        Address existentialContainer(
            &*(it++), IGM.getExistentialType(existLayout.getNumTables()),
            existLayout.getAlignment(IGM));

        // Dynamically check whether this type is inline or needs a
        // deallocation.
        auto *metadata = existLayout.loadMetadataRef(IGF, existentialContainer);
        toolchain::Value *isInline, *flags;
        std::tie(isInline, flags) = emitLoadOfIsInline(IGF, metadata);
        toolchain::BasicBlock *doneBB = IGF.createBasicBlock("done");
        toolchain::BasicBlock *deallocateBB = IGF.createBasicBlock("deallocateBox");
        Builder.CreateCondBr(isInline, doneBB, deallocateBB);

        // We are done. Return.
        Builder.emitBlock(doneBB);
        Builder.CreateRetVoid();

        // We have an allocated uninitialized box. Deallocate the box.
        // No ConditionalDominanceScope because no code is executed that could
        // affect the caches.
        Builder.emitBlock(deallocateBB);

        // Project to the existential buffer address.
        auto existentialBuffer =
            existLayout.projectExistentialBuffer(IGF, existentialContainer);
        auto *boxReferenceAddr =
            Builder.CreateBitCast(existentialBuffer.getAddress(),
                                  IGM.RefCountedPtrTy->getPointerTo());
        // Load the reference.
        auto *boxReference =
            Builder.CreateLoad(Address(boxReferenceAddr, IGM.RefCountedPtrTy,
                                       existentialBuffer.getAlignment()));

        // Size and alignment requirements of the boxed value.
        auto *size = emitLoadOfSize(IGF, metadata);
        auto *alignmentMask = emitAlignMaskFromFlags(IGF, flags);

        //  Size = ((sizeof(HeapObject) + align) & ~align) + size
        auto *heapHeaderSize = toolchain::ConstantInt::get(
            IGF.IGM.SizeTy, IGM.RefCountedStructSize.getValue());
        auto *Add = Builder.CreateAdd(heapHeaderSize, alignmentMask);
        auto *Not = Builder.CreateNot(alignmentMask);
        size = Builder.CreateAdd(Builder.CreateAnd(Add, Not), size);

        // At least pointer aligned.
        //  AlignmentMask = alignmentMask | alignof(void*) - 1
        toolchain::Value *pointerAlignMask = toolchain::ConstantInt::get(
            IGF.IGM.SizeTy, IGF.IGM.getPointerAlignment().getValue() - 1);
        alignmentMask = Builder.CreateOr(alignmentMask, pointerAlignMask);
        IGF.emitDeallocRawCall(
            Builder.CreateBitCast(boxReference, IGF.IGM.Int8PtrTy), size,
            alignmentMask);
        // We are done. Return.
        Builder.CreateRetVoid();
      },
      true /*noinline*/));
}

void irgen::emitDeallocateBoxedOpaqueExistentialBuffer(
    IRGenFunction &IGF, SILType existentialType, Address existentialContainer) {

  // Project to the existential buffer in the existential container.
  auto &existentialTI =
      IGF.getTypeInfo(existentialType).as<OpaqueExistentialTypeInfo>();
  OpaqueExistentialLayout existLayout = existentialTI.getLayout();

  auto *deallocateFun = getDeallocateBoxedOpaqueExistentialBufferFunction(
      IGF.IGM, existLayout);
  auto *bufferAddr = IGF.Builder.CreateBitCast(
      existentialContainer.getAddress(),
      IGF.IGM.getExistentialPtrTy(existLayout.getNumTables()));
  auto *call = IGF.Builder.CreateCallWithoutDbgLoc(
      deallocateFun->getFunctionType(), deallocateFun, {bufferAddr});
  call->setCallingConv(IGF.IGM.DefaultCC);
  call->setDoesNotThrow();
  return;
}

void irgen::emitDestroyBoxedOpaqueExistentialBuffer(
    IRGenFunction &IGF, SILType existentialType, Address existentialContainer) {

  // Project to the existential buffer in the existential container.
  auto &existentialTI =
      IGF.getTypeInfo(existentialType).as<OpaqueExistentialTypeInfo>();
  OpaqueExistentialLayout existLayout = existentialTI.getLayout();

  auto *deallocateFun =
      getDestroyBoxedOpaqueExistentialBufferFunction(IGF.IGM, existLayout);
  auto *bufferAddr = IGF.Builder.CreateBitCast(
      existentialContainer.getAddress(),
      IGF.IGM.getExistentialPtrTy(existLayout.getNumTables()));
  auto *call = IGF.Builder.CreateCallWithoutDbgLoc(
    deallocateFun->getFunctionType(), deallocateFun, {bufferAddr});
  call->setCallingConv(IGF.IGM.DefaultCC);
  call->setDoesNotThrow();
  return;
}

static toolchain::Function *
getProjectBoxedOpaqueExistentialFunction(IRGenFunction &IGF,
                                         OpenedExistentialAccess accessKind,
                                         OpaqueExistentialLayout existLayout) {

  auto &IGM = IGF.IGM;
  auto *existentialBufferTy = IGM.getExistentialPtrTy(existLayout.getNumTables());
  toolchain::Type *argTys[] = {existentialBufferTy, IGM.TypeMetadataPtrTy};

  // __language_project_boxed_opaque_existential_N is the well-known function for
  // projecting buffers in existential containers of types with N witness
  // tables.
  toolchain::SmallString<40> fnName;
  toolchain::raw_svector_ostream(fnName)
      << (accessKind == OpenedExistentialAccess::Immutable
              ? "__language_project_boxed_opaque_existential_"
              : "__language_mutable_project_boxed_opaque_existential_")
      << existLayout.getNumTables();

  return cast<toolchain::Function>(IGM.getOrCreateHelperFunction(
      fnName, IGM.OpaquePtrTy, argTys,
      [&](IRGenFunction &IGF) {
        auto &Builder = IGF.Builder;
        auto &IGM = IGF.IGM;
        auto it = IGF.CurFn->arg_begin();
        Address existentialBuffer(
            &*(it++), IGM.getExistentialType(existLayout.getNumTables()),
            existLayout.getAlignment(IGM));
        auto *metadata = &*(it++);

        // Dynamically check whether this type is inline or needs a
        // deallocation.
        toolchain::Value *isInline, *flags;
        std::tie(isInline, flags) = emitLoadOfIsInline(IGF, metadata);
        toolchain::BasicBlock *doneBB = IGF.createBasicBlock("done");
        toolchain::BasicBlock *boxedBB = IGF.createBasicBlock("boxed");
        toolchain::Value *addressInline = Builder.CreateBitCast(
            existentialBuffer.getAddress(), IGM.OpaquePtrTy);
        Builder.CreateCondBr(isInline, doneBB, boxedBB);

        // We are done. Return the pointer to the address of the value.
        Builder.emitBlock(doneBB);
        IGF.Builder.CreateRet(addressInline);

        // We have a boxed representation.
        Builder.emitBlock(boxedBB);

        if (accessKind == OpenedExistentialAccess::Immutable) {
          // Project to the existential buffer address.
          auto *boxReferenceAddr =
              Builder.CreateBitCast(existentialBuffer.getAddress(),
                                    IGM.RefCountedPtrTy->getPointerTo());
          // Load the reference.
          auto *boxReference =
              Builder.CreateLoad(Address(boxReferenceAddr, IGM.RefCountedPtrTy,
                                         existentialBuffer.getAlignment()));

          // Size and alignment requirements of the boxed value.
          auto *alignmentMask = emitAlignMaskFromFlags(IGF, flags);

          //  StartOffset = ((sizeof(HeapObject) + align) & ~align)
          auto *heapHeaderSize = toolchain::ConstantInt::get(
              IGF.IGM.SizeTy, IGM.RefCountedStructSize.getValue());
          auto *Add = Builder.CreateAdd(heapHeaderSize, alignmentMask);
          auto *Not = Builder.CreateNot(alignmentMask);
          auto *startOffset = Builder.CreateAnd(Add, Not);
          auto *addressInBox =
              IGF.emitByteOffsetGEP(boxReference, startOffset, IGM.OpaqueTy);
          IGF.Builder.CreateRet(addressInBox);
          return;
        }
        // If we are opening this existential for mutating check the reference
        // count and copy if the boxed is not uniquely owned by this reference.
        assert(accessKind == OpenedExistentialAccess::Mutable);
        auto *alignmentMask = emitAlignMaskFromFlags(IGF, flags);

        toolchain::Value *box, *objectAddr;
        IGF.emitMakeBoxUniqueCall(
            Builder.CreateBitCast(existentialBuffer.getAddress(),
                                  IGM.OpaquePtrTy),
            metadata, alignmentMask, box, objectAddr);

        IGF.Builder.CreateRet(objectAddr);
      },
      true /*noinline*/));
}

Address irgen::emitOpaqueBoxedExistentialProjection(
    IRGenFunction &IGF, OpenedExistentialAccess accessKind, Address base,
    SILType existentialTy, CanArchetypeType openedArchetype) {

  assert(existentialTy.isExistentialType());
  if (existentialTy.isClassExistentialType()) {
    auto &baseTI =
        IGF.getTypeInfo(existentialTy).as<ClassExistentialTypeInfo>();
    auto valueAddr = baseTI.projectValue(IGF, base);
    auto value = IGF.Builder.CreateLoad(valueAddr);
    auto metadata = emitDynamicTypeOfHeapObject(IGF, value,
                                                MetatypeRepresentation::Thick,
                                                existentialTy,
                                                /*allow artificial*/ false);

    // If we are projecting into an opened archetype, capture the
    // witness tables.
    if (openedArchetype) {
      SmallVector<toolchain::Value *, 4> wtables;
      for (unsigned i = 0, n = baseTI.getNumStoredProtocols(); i != n; ++i) {
        auto wtableAddr = baseTI.projectWitnessTable(IGF, base, i);
        wtables.push_back(IGF.Builder.CreateLoad(wtableAddr));
      }

      bindArchetype(IGF, openedArchetype, metadata, MetadataState::Complete,
                    wtables);
    }

    return valueAddr;
  }

  auto &baseTI = IGF.getTypeInfo(existentialTy).as<OpaqueExistentialTypeInfo>();
  auto layout = baseTI.getLayout();

  toolchain::Value *metadata = layout.loadMetadataRef(IGF, base);

  // If we are projecting into an opened archetype, capture the
  // witness tables.
  if (openedArchetype) {
    SmallVector<toolchain::Value *, 4> wtables;
    for (unsigned i = 0, n = layout.getNumTables(); i != n; ++i) {
      wtables.push_back(layout.loadWitnessTable(IGF, base, i));
    }
    bindArchetype(IGF, openedArchetype, metadata, MetadataState::Complete,
                  wtables);
  }

  auto *projectFunc =
      getProjectBoxedOpaqueExistentialFunction(IGF, accessKind, layout);
  auto *bufferAddr = IGF.Builder.CreateBitCast(
      base.getAddress(),
      IGF.IGM.getExistentialPtrTy(layout.getNumTables()));
  auto *addrOfValue = IGF.Builder.CreateCallWithoutDbgLoc(
      projectFunc->getFunctionType(), projectFunc, {bufferAddr, metadata});
  addrOfValue->setCallingConv(IGF.IGM.DefaultCC);
  addrOfValue->setDoesNotThrow();

  return Address(addrOfValue, IGF.IGM.OpaqueTy, Alignment(1));
}

static void initBufferWithCopyOfReference(IRGenFunction &IGF,
                                          OpaqueExistentialLayout existLayout,
                                          Address destBuffer,
                                          Address srcBuffer) {
  auto &IGM = IGF.IGM;
  auto &Builder = IGF.Builder;

  auto *destReferenceAddr = Builder.CreateBitCast(
      destBuffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
  auto *srcReferenceAddr = Builder.CreateBitCast(
      srcBuffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
  auto *srcReference = Builder.CreateLoad(
      Address(srcReferenceAddr, IGM.RefCountedPtrTy, srcBuffer.getAlignment()));
  IGF.emitNativeStrongRetain(srcReference, IGF.getDefaultAtomicity());
  IGF.Builder.CreateStore(srcReference,
                          Address(destReferenceAddr, IGM.RefCountedPtrTy,
                                  existLayout.getAlignment(IGF.IGM)));
}

static toolchain::Function *getAssignBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout) {

  toolchain::Type *argTys[] = {IGM.getExistentialPtrTy(existLayout.getNumTables()),
                          IGM.getExistentialPtrTy(existLayout.getNumTables())};

  // __language_assign_box_in_existentials_N is the well-known function for
  // assigning buffers in existential containers of types with N witness
  // tables.
  toolchain::SmallString<40> fnName;
  toolchain::raw_svector_ostream(fnName)
      << "__language_assign_boxed_opaque_existential_"
      << existLayout.getNumTables();

  return cast<toolchain::Function>(IGM.getOrCreateHelperFunction(
      fnName, IGM.VoidTy, argTys,
      [&](IRGenFunction &IGF) {
        auto it = IGF.CurFn->arg_begin();
        auto boxTy = IGM.getExistentialType(existLayout.getNumTables());
        Address dest(&*(it++), boxTy, getFixedBufferAlignment(IGM));
        Address src(&*(it++), boxTy, getFixedBufferAlignment(IGM));
        auto &Builder = IGF.Builder;

        // If doing a self-assignment, we're done.
        toolchain::BasicBlock *doneBB = IGF.createBasicBlock("done");
        toolchain::BasicBlock *contBB = IGF.createBasicBlock("cont");
        toolchain::Value *isSelfAssign = Builder.CreateICmpEQ(
            dest.getAddress(), src.getAddress(), "isSelfAssign");
        Builder.CreateCondBr(isSelfAssign, doneBB, contBB);

        Builder.emitBlock(contBB);
        // We don't need a ConditionalDominanceScope here because (1) there's no
        // code in the other condition and (2) we immediately return.
        Address destBuffer = existLayout.projectExistentialBuffer(IGF, dest);
        Address srcBuffer = existLayout.projectExistentialBuffer(IGF, src);

        // Load the metadata tables.
        Address destMetadataSlot = existLayout.projectMetadataRef(IGF, dest);
        toolchain::Value *destMetadata = Builder.CreateLoad(destMetadataSlot);
        toolchain::Value *srcMetadata = existLayout.loadMetadataRef(IGF, src);

        // Check whether the metadata match.
        auto *matchBB = IGF.createBasicBlock("match");
        auto *noMatchBB = IGF.createBasicBlock("no-match");
        auto *sameMetadata =
            Builder.CreateICmpEQ(destMetadata, srcMetadata, "sameMetadata");
        Builder.CreateCondBr(sameMetadata, matchBB, noMatchBB);

        Builder.emitBlock(matchBB);
        {
          // Metadata pointers match.
          ConditionalDominanceScope matchCondition(IGF);
          toolchain::Value *isInline, *flags;
          auto *metadata = destMetadata;
          std::tie(isInline, flags) = emitLoadOfIsInline(IGF, metadata);
          auto *matchInlineBB = IGF.createBasicBlock("match-inline");
          auto *matchOutlineBB = IGF.createBasicBlock("match-outline");
          Builder.CreateCondBr(isInline, matchInlineBB, matchOutlineBB);

          // Inline.
          Builder.emitBlock(matchInlineBB);
          {
            ConditionalDominanceScope inlineCondition(IGF);
            auto dstAddress = castToOpaquePtr(IGF, destBuffer);
            auto srcAddress = castToOpaquePtr(IGF, srcBuffer);
            emitAssignWithCopyCall(IGF, metadata, dstAddress, srcAddress);
            Builder.CreateBr(doneBB);
          }

          // Outline.
          Builder.emitBlock(matchOutlineBB);
          {
            ConditionalDominanceScope outlineCondition(IGF);
            auto *destReferenceAddr = Builder.CreateBitCast(
                destBuffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
            auto *srcReferenceAddr = Builder.CreateBitCast(
                srcBuffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
            // Load the reference.
            auto *destReference = Builder.CreateLoad(
                Address(destReferenceAddr, IGM.RefCountedPtrTy,
                        destBuffer.getAlignment()));
            auto *srcReference = Builder.CreateLoad(
                Address(srcReferenceAddr, IGM.RefCountedPtrTy,
                        srcBuffer.getAlignment()));
            IGF.emitNativeStrongRetain(srcReference, IGF.getDefaultAtomicity());
            IGF.emitNativeStrongRelease(destReference,
                                        IGF.getDefaultAtomicity());
            IGF.Builder.CreateStore(
                srcReference, Address(destReferenceAddr, IGM.RefCountedPtrTy,
                                      existLayout.getAlignment(IGF.IGM)));
            Builder.CreateBr(doneBB);
          }
        }

        Builder.emitBlock(noMatchBB);
        {
          // Metadata pointers don't match.
          ConditionalDominanceScope noMatchCondition(IGF);
          // Store the metadata ref.
          IGF.Builder.CreateStore(srcMetadata, destMetadataSlot);

          // Store the protocol witness tables.
          unsigned numTables = existLayout.getNumTables();
          for (unsigned i = 0, e = numTables; i != e; ++i) {
            Address destTableSlot =
                existLayout.projectWitnessTable(IGF, dest, i);
            toolchain::Value *srcTable = existLayout.loadWitnessTable(IGF, src, i);

            // Overwrite the old witness table.
            IGF.Builder.CreateStore(srcTable, destTableSlot);
          }

          // Check whether buffers are inline.
          toolchain::Value *isDestInline, *destFlags;
          toolchain::Value *isSrcInline, *srcFlags;
          std::tie(isDestInline, destFlags) =
              emitLoadOfIsInline(IGF, destMetadata);
          std::tie(isSrcInline, srcFlags) =
              emitLoadOfIsInline(IGF, srcMetadata);
          Address tmpBuffer = IGF.createAlloca(IGM.getFixedBufferTy(),
                                               existLayout.getAlignment(IGM),
                                               "tmpInlineBuffer");
          auto *destInlineBB = IGF.createBasicBlock("dest-inline");
          auto *destOutlineBB = IGF.createBasicBlock("dest-outline");
          // Check whether the destination is inline.
          Builder.CreateCondBr(isDestInline, destInlineBB, destOutlineBB);

          Builder.emitBlock(destInlineBB);
          {
            ConditionalDominanceScope destInlineCondition(IGF);
            // Move aside so that we can destroy later.
            auto tmpAddress = castToOpaquePtr(IGF, tmpBuffer);
            auto destAddress = castToOpaquePtr(IGF, destBuffer);
            emitInitializeWithTakeCall(IGF, destMetadata, tmpAddress,
                                       destAddress);
            auto *srcInlineBB = IGF.createBasicBlock("dest-inline-src-inline");
            auto *srcOutlineBB =
                IGF.createBasicBlock("dest-inline-src-outline");
            auto *contBB2 = IGF.createBasicBlock("dest-inline-cont");
            // Check whether the source is inline.
            Builder.CreateCondBr(isSrcInline, srcInlineBB, srcOutlineBB);

            Builder.emitBlock(srcInlineBB);
            {
              // initializeWithCopy(dest, src)
              ConditionalDominanceScope domScope(IGF);
              auto destAddress = castToOpaquePtr(IGF, destBuffer);
              auto srcAddress = castToOpaquePtr(IGF, srcBuffer);
              emitInitializeWithCopyCall(IGF, srcMetadata, destAddress,
                                         srcAddress);
              Builder.CreateBr(contBB2);
            }

            Builder.emitBlock(srcOutlineBB);
            {
              // dest[0] = src[0]
              // language_retain(src[0])
              ConditionalDominanceScope domScope(IGF);
              initBufferWithCopyOfReference(IGF, existLayout, destBuffer,
                                            srcBuffer);
              Builder.CreateBr(contBB2);
            }

            Builder.emitBlock(contBB2);
            {
              ConditionalDominanceScope domScope(IGF);
              // destroy(tmpBuffer)
              emitDestroyCall(IGF, destMetadata,
                              castToOpaquePtr(IGF, tmpBuffer));
              Builder.CreateBr(doneBB);
            }
          }
          Builder.emitBlock(destOutlineBB);
          {
            ConditionalDominanceScope destOutlineCondition(IGF);
            // tmpRef = dest[0]
            auto *destReferenceAddr = Builder.CreateBitCast(
                destBuffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
            auto *destReference = Builder.CreateLoad(
                Address(destReferenceAddr, IGM.RefCountedPtrTy,
                        srcBuffer.getAlignment()));
            auto *srcInlineBB = IGF.createBasicBlock("dest-outline-src-inline");
            auto *srcOutlineBB =
                IGF.createBasicBlock("dest-outline-src-outline");
            auto *contBB2 = IGF.createBasicBlock("dest-outline-cont");
            // Check whether the source is inline.
            Builder.CreateCondBr(isSrcInline, srcInlineBB, srcOutlineBB);

            Builder.emitBlock(srcInlineBB);
            {
              // initializeWithCopy(dest, src)
              ConditionalDominanceScope domScope(IGF);
              auto destAddress = castToOpaquePtr(IGF, destBuffer);
              auto srcAddress = castToOpaquePtr(IGF, srcBuffer);
              emitInitializeWithCopyCall(IGF, srcMetadata, destAddress,
                                         srcAddress);
              Builder.CreateBr(contBB2);
            }

            Builder.emitBlock(srcOutlineBB);
            {
              // dest[0] = src[0]
              // language_retain(src[0])
              ConditionalDominanceScope domScope(IGF);
              initBufferWithCopyOfReference(IGF, existLayout, destBuffer,
                                            srcBuffer);
              Builder.CreateBr(contBB2);
            }
            Builder.emitBlock(contBB2);
            {
              ConditionalDominanceScope domScope(IGF);
              // language_release(tmpRef)
              IGF.emitNativeStrongRelease(destReference,
                                          IGF.getDefaultAtomicity());
              Builder.CreateBr(doneBB);
            }
          }
        }

        Builder.emitBlock(doneBB);
        Builder.CreateRetVoid();
      },
      true /*noinline*/));
}

static toolchain::Function *getDestroyBoxedOpaqueExistentialBufferFunction(
    IRGenModule &IGM, OpaqueExistentialLayout existLayout) {

  toolchain::Type *argTys[] = {IGM.getExistentialPtrTy(existLayout.getNumTables())};

  toolchain::SmallString<40> fnName;
  toolchain::raw_svector_ostream(fnName)
      << "__language_destroy_boxed_opaque_existential_"
      << existLayout.getNumTables();

  return cast<toolchain::Function>(IGM.getOrCreateHelperFunction(
      fnName, IGM.VoidTy, argTys,
      [&](IRGenFunction &IGF) {
        auto &Builder = IGF.Builder;
        auto it = IGF.CurFn->arg_begin();
        auto boxTy = IGM.getExistentialType(existLayout.getNumTables());
        Address existentialContainer(&*(it++), boxTy,
                                     existLayout.getAlignment(IGM));
        auto *metadata = existLayout.loadMetadataRef(IGF, existentialContainer);
        auto buffer =
            existLayout.projectExistentialBuffer(IGF, existentialContainer);

        // Is the value stored inline?
        toolchain::Value *isInline, *flags;
        std::tie(isInline, flags) = emitLoadOfIsInline(IGF, metadata);
        auto *inlineBB = IGF.createBasicBlock("inline");
        auto *outlineBB = IGF.createBasicBlock("outline");
        Builder.CreateCondBr(isInline, inlineBB, outlineBB);

        Builder.emitBlock(inlineBB);
        {
          ConditionalDominanceScope domScope(IGF);
          auto *opaquePtrToBuffer =
              Builder.CreateBitCast(buffer.getAddress(), IGM.OpaquePtrTy);
          emitDestroyCall(
              IGF, metadata,
              Address(opaquePtrToBuffer, IGM.OpaqueTy, buffer.getAlignment()));
          Builder.CreateRetVoid();
        }

        Builder.emitBlock(outlineBB);
        {
          ConditionalDominanceScope domScope(IGF);

          // language_release(buffer[0])
          auto *referenceAddr = Builder.CreateBitCast(
              buffer.getAddress(), IGM.RefCountedPtrTy->getPointerTo());
          auto *reference = Builder.CreateLoad(Address(
              referenceAddr, IGM.RefCountedPtrTy, buffer.getAlignment()));
          IGF.emitNativeStrongRelease(reference, IGF.getDefaultAtomicity());

          Builder.CreateRetVoid();
        }
      },
      true /*noinline*/));
}
