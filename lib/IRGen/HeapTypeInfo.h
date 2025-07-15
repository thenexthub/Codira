//===--- HeapTypeInfo.h - Utilities for reference-counted types -*- C++ -*-===//
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
// This file defines some routines that are useful for emitting
// types that are single, reference-counted pointers.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_HEAPTYPEINFO_H
#define LANGUAGE_IRGEN_HEAPTYPEINFO_H

#include "language/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "toolchain/IR/DerivedTypes.h"

#include "ExtraInhabitants.h"
#include "ReferenceTypeInfo.h"
#include "ScalarTypeInfo.h"
#include "CodiraTargetInfo.h"
#include "GenType.h"

namespace language {
namespace irgen {

/// \group HeapTypeInfo
  
/// The kind of 'isa' encoding a heap object uses to reference its heap
/// metadata.
enum class IsaEncoding : uint8_t {
  /// The object stores a plain pointer to its heap metadata as its first word.
  Pointer,
  /// The object's isa is managed by the Objective-C runtime and must be
  /// accessed with object_getClass. This is a superset of "Pointer" because
  /// object_getClass is compatible with pointer isas.
  ObjC,
  /// The isa encoding is unknown and must be accessed in a maximally-compatible
  /// way.
  Unknown = ObjC,
};

/// HeapTypeInfo - A type designed for use implementing a type
/// which consists solely of something reference-counted.
///
/// Subclasses should implement the following method, returning true
/// if it's known to be OK to use Codira reference-counting on values
/// of this type:
///   ReferenceCounting getReferenceCounting() const;
template <class Impl>
class HeapTypeInfo
    : public SingleScalarTypeInfoWithTypeLayout<Impl, ReferenceTypeInfo> {
  using super = SingleScalarTypeInfoWithTypeLayout<Impl, ReferenceTypeInfo>;

  toolchain::Type *getOptionalIntType() const {
    return toolchain::IntegerType::get(this->getStorageType()->getContext(),
                                  this->getFixedSize().getValueInBits());
  }

protected:
  using super::asDerived;
public:
  HeapTypeInfo(ReferenceCounting refcounting, toolchain::PointerType *storage,
               Size size, SpareBitVector spareBits, Alignment align)
    : super(language::irgen::refcountingToScalarKind(refcounting), storage,
            size, spareBits, align) {}

  bool isSingleRetainablePointer(ResilienceExpansion expansion,
                                 ReferenceCounting *refcounting) const override {
    if (refcounting)
      *refcounting = asDerived().getReferenceCounting();
    return true;
  }
  
  bool canValueWitnessExtraInhabitantsUpTo(IRGenModule &IGM,
                                           unsigned index) const override {
    // Refcounting functions are no-ops when passed a null pointer, which is the
    // first extra inhabitant.
    return index == 0;
  }
  
  IsaEncoding getIsaEncoding(ResilienceExpansion expansion) const {
    switch (asDerived().getReferenceCounting()) {
    // We can access the isa of pure Codira heap objects directly.
    case ReferenceCounting::Native:
      return IsaEncoding::Pointer;
    // Use the ObjC runtime to access ObjC or mixed-heritage isas.
    case ReferenceCounting::ObjC:
    case ReferenceCounting::Block:
      return IsaEncoding::ObjC;
    case ReferenceCounting::Unknown:
      return IsaEncoding::Unknown;
    case ReferenceCounting::Error:
      toolchain_unreachable("errortype doesn't have an isa");
    }
  }

  static const bool IsScalarTriviallyDestroyable = false;

  // Emit the copy/destroy operations required by SingleScalarTypeInfo
  // using strong reference counting.
  virtual void emitScalarRelease(IRGenFunction &IGF, toolchain::Value *value,
                                 Atomicity atomicity) const {
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);
    IGF.emitStrongRelease(value, asDerived().getReferenceCounting(), atomicity);
  }

  void emitScalarFixLifetime(IRGenFunction &IGF, toolchain::Value *value) const {
    return IGF.emitFixLifetime(value);
  }

  virtual void emitScalarRetain(IRGenFunction &IGF, toolchain::Value *value,
                                Atomicity atomicity) const {
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);
    IGF.emitStrongRetain(value, asDerived().getReferenceCounting(), atomicity);
  }

  // Implement the primary retain/release operations of ReferenceTypeInfo
  // using basic reference counting.
  void strongRetain(IRGenFunction &IGF, Explosion &e,
                    Atomicity atomicity) const override {
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);
    toolchain::Value *value = e.claimNext();
    asDerived().emitScalarRetain(IGF, value, atomicity);
  }

  void strongRelease(IRGenFunction &IGF, Explosion &e,
                     Atomicity atomicity) const override {
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);
    toolchain::Value *value = e.claimNext();
    asDerived().emitScalarRelease(IGF, value, atomicity);
  }

#define REF_STORAGE_HELPER(Name) \
  const TypeInfo * \
  create##Name##StorageType(TypeConverter &TC, \
                            bool isOptional) const override { \
    return TC.create##Name##StorageType(this->getStorageType(), \
                                        asDerived().getReferenceCounting(), \
                                        isOptional); \
  }
#define NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  void name##LoadStrong(IRGenFunction &IGF, Address src, \
                         Explosion &out, bool isOptional) const override { \
    toolchain::Value *value = IGF.emit##Name##LoadStrong(src, \
                                          this->getStorageType(), \
                                          asDerived().getReferenceCounting()); \
    if (isOptional) { \
      out.add(IGF.Builder.CreatePtrToInt(value, getOptionalIntType())); \
    } else { \
      out.add(value); \
    } \
  } \
  void name##TakeStrong(IRGenFunction &IGF, Address src, \
                         Explosion &out, bool isOptional) const override { \
    toolchain::Value *value = IGF.emit##Name##TakeStrong(src, \
                                          this->getStorageType(), \
                                          asDerived().getReferenceCounting()); \
    if (isOptional) { \
      out.add(IGF.Builder.CreatePtrToInt(value, getOptionalIntType())); \
    } else { \
      out.add(value); \
    } \
  } \
  void name##Init(IRGenFunction &IGF, Explosion &in, \
                  Address dest, bool isOptional) const override { \
    toolchain::Value *value = in.claimNext(); \
    if (isOptional) { \
      assert(value->getType() == getOptionalIntType()); \
      value = IGF.Builder.CreateIntToPtr(value, this->getStorageType()); \
    } \
    IGF.emit##Name##Init(value, dest, asDerived().getReferenceCounting()); \
  } \
  void name##Assign(IRGenFunction &IGF, Explosion &in, \
                    Address dest, bool isOptional) const override { \
    toolchain::Value *value = in.claimNext(); \
    if (isOptional) { \
      assert(value->getType() == getOptionalIntType()); \
      value = IGF.Builder.CreateIntToPtr(value, this->getStorageType()); \
    } \
    IGF.emit##Name##Assign(value, dest, asDerived().getReferenceCounting()); \
  }
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name)                 \
  void strongRetain##Name(IRGenFunction &IGF, Explosion &e,                    \
                          Atomicity atomicity) const override {                \
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);   \
    toolchain::Value *value = e.claimNext();                                        \
    assert(asDerived().getReferenceCounting() == ReferenceCounting::Native);   \
    IGF.emitNativeStrongRetain##Name(value, atomicity);                        \
  }                                                                            \
  void strongRetain##Name##Release(IRGenFunction &IGF, Explosion &e,           \
                                   Atomicity atomicity) const override {       \
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);   \
    toolchain::Value *value = e.claimNext();                                        \
    assert(asDerived().getReferenceCounting() == ReferenceCounting::Native);   \
    IGF.emitNativeStrongRetainAnd##Name##Release(value, atomicity);            \
  }                                                                            \
  void name##Retain(IRGenFunction &IGF, Explosion &e, Atomicity atomicity)     \
      const override {                                                         \
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);   \
    toolchain::Value *value = e.claimNext();                                        \
    assert(asDerived().getReferenceCounting() == ReferenceCounting::Native);   \
    IGF.emitNative##Name##Retain(value, atomicity);                            \
  }                                                                            \
  void name##Release(IRGenFunction &IGF, Explosion &e, Atomicity atomicity)    \
      const override {                                                         \
    assert(asDerived().getReferenceCounting() != ReferenceCounting::Custom);   \
    toolchain::Value *value = e.claimNext();                                        \
    assert(asDerived().getReferenceCounting() == ReferenceCounting::Native);   \
    IGF.emitNative##Name##Release(value, atomicity);                           \
  }
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define UNCHECKED_REF_STORAGE(Name, ...) \
  REF_STORAGE_HELPER(Name)
#include "language/AST/ReferenceStorage.def"
#undef REF_STORAGE_HELPER
#undef NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER
#undef ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER

  LoadedRef loadRefcountedPtr(IRGenFunction &IGF, SourceLoc loc,
                              Address addr) const override {
    auto style = asDerived().getReferenceCounting();
    toolchain::Value *ptr =
      IGF.emitLoadRefcountedPtr(addr, style);
    return LoadedRef(ptr, true, style);
  }

  ReferenceCounting getReferenceCountingType() const override {
    return asDerived().getReferenceCounting();
  }

  // Extra inhabitants of heap object pointers.

  bool mayHaveExtraInhabitants(IRGenModule &IGM) const override {
    return true;
  }

  unsigned getFixedExtraInhabitantCount(IRGenModule &IGM) const override {
    return getHeapObjectExtraInhabitantCount(IGM);
  }

  APInt getFixedExtraInhabitantValue(IRGenModule &IGM,
                                     unsigned bits,
                                     unsigned index) const override {
    return getHeapObjectFixedExtraInhabitantValue(IGM, bits, index, 0);
  }

  toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF, Address src,
                                       SILType T, bool isOutlined)
  const override {
    return getHeapObjectExtraInhabitantIndex(IGF, src);
  }

  void storeExtraInhabitant(IRGenFunction &IGF, toolchain::Value *index,
                            Address dest, SILType T, bool isOutlined)
  const override {
    return storeHeapObjectExtraInhabitant(IGF, index, dest);
  }
};

}
}

#endif
