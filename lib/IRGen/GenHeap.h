//===--- GenHeap.h - Heap-object layout and management ----------*- C++ -*-===//
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
// operations on heap objects and their metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENHEAP_H
#define LANGUAGE_IRGEN_GENHEAP_H

#include "NecessaryBindings.h"
#include "StructLayout.h"

namespace toolchain {
  class Constant;
  template <class T> class SmallVectorImpl;
}

namespace language {
namespace irgen {
  class Address;
  class OwnedAddress;
  enum class IsaEncoding : unsigned char;

/// A heap layout is the result of laying out a complete structure for
/// heap-allocation.
class HeapLayout : public StructLayout {
  SmallVector<SILType, 8> ElementTypes;
  NecessaryBindings Bindings;
  unsigned BindingsIndex;
  mutable toolchain::Constant *privateMetadata = nullptr;
  
public:
  HeapLayout(IRGenModule &IGM, LayoutStrategy strategy,
             ArrayRef<SILType> elementTypes,
             ArrayRef<const TypeInfo *> elementTypeInfos,
             toolchain::StructType *typeToFill = 0,
             NecessaryBindings &&bindings = {}, unsigned bindingsIndex = 0);

  /// True if the heap object carries type bindings.
  ///
  /// If true, the first element of the heap layout will be the type metadata
  /// buffer.
  bool hasBindings() const {
    return !Bindings.empty();
  }
  
  const NecessaryBindings &getBindings() const {
    return Bindings;
  }

  unsigned getBindingsIndex() const { return BindingsIndex; }

  unsigned getIndexAfterBindings() const {
    return BindingsIndex + (hasBindings() ? 1 : 0);
  }

  /// Get the types of the elements.
  ArrayRef<SILType> getElementTypes() const {
    return ElementTypes;
  }
  
  /// Build a size function for this layout.
  toolchain::Constant *createSizeFn(IRGenModule &IGM) const;

  /// As a convenience, build a metadata object with internal linkage
  /// consisting solely of the standard heap metadata.
  toolchain::Constant *getPrivateMetadata(IRGenModule &IGM,
                                     toolchain::Constant *captureDescriptor) const;
};

class HeapNonFixedOffsets : public NonFixedOffsetsImpl {
  SmallVector<toolchain::Value *, 1> Offsets;
  toolchain::Value *TotalSize;
  toolchain::Value *TotalAlignMask;
public:
  HeapNonFixedOffsets(IRGenFunction &IGF, const HeapLayout &layout);
  
  toolchain::Value *getOffsetForIndex(IRGenFunction &IGF, unsigned index) override {
    auto result = Offsets[index];
    assert(result != nullptr
           && "fixed-layout field doesn't need NonFixedOffsets");
    return result;
  }
  
  // The total size of the heap object.
  toolchain::Value *getSize() const {
    return TotalSize;
  }
  
  // The total alignment of the heap object.
  toolchain::Value *getAlignMask() const {
    return TotalAlignMask;
  }
};

/// Emit a heap object deallocation.
void emitDeallocateHeapObject(IRGenFunction &IGF,
                              toolchain::Value *object,
                              toolchain::Value *size,
                              toolchain::Value *alignMask);

/// Emit a class instance deallocation.
void emitDeallocateClassInstance(IRGenFunction &IGF,
                                 toolchain::Value *object,
                                 toolchain::Value *size,
                                 toolchain::Value *alignMask);
  
/// Emit a partial class instance deallocation from a failing constructor.
void emitDeallocatePartialClassInstance(IRGenFunction &IGF,
                                        toolchain::Value *object,
                                        toolchain::Value *metadata,
                                        toolchain::Value *size,
                                        toolchain::Value *alignMask);

/// Allocate a boxed value.
///
/// The interface type is required for emitting reflection metadata.
OwnedAddress
emitAllocateBox(IRGenFunction &IGF,
                CanSILBoxType boxType,
                GenericEnvironment *env,
                const toolchain::Twine &name);

/// Deallocate a box whose value is uninitialized.
void emitDeallocateBox(IRGenFunction &IGF, toolchain::Value *box,
                       CanSILBoxType boxType);

/// Project the address of the value inside a box.
Address emitProjectBox(IRGenFunction &IGF, toolchain::Value *box,
                       CanSILBoxType boxType);

/// Allocate a boxed value based on the boxed type. Returns the address of the
/// storage for the value.
Address
emitAllocateExistentialBoxInBuffer(IRGenFunction &IGF, SILType boxedType,
                                   Address destBuffer, GenericEnvironment *env,
                                   const toolchain::Twine &name, bool isOutlined);

/// Given a heap-object instance, with some heap-object type,
/// produce a reference to its type metadata.
toolchain::Value *emitDynamicTypeOfHeapObject(IRGenFunction &IGF,
                                         toolchain::Value *object,
                                         MetatypeRepresentation rep,
                                         SILType objectType,
                                         bool allowArtificialSubclasses = false);

/// Given a non-tagged object pointer, load a pointer to its class object.
toolchain::Value *emitLoadOfObjCHeapMetadataRef(IRGenFunction &IGF,
                                           toolchain::Value *object);

/// Given a heap-object instance, with some heap-object type, produce a
/// reference to its heap metadata by dynamically asking the runtime for it.
toolchain::Value *emitHeapMetadataRefForUnknownHeapObject(IRGenFunction &IGF,
                                                     toolchain::Value *object);

/// Given a heap-object instance, with some heap-object type,
/// produce a reference to its heap metadata.
toolchain::Value *emitHeapMetadataRefForHeapObject(IRGenFunction &IGF,
                                              toolchain::Value *object,
                                              CanType objectType,
                                              bool suppressCast = false);

/// Given a heap-object instance, with some heap-object type,
/// produce a reference to its heap metadata.
toolchain::Value *emitHeapMetadataRefForHeapObject(IRGenFunction &IGF,
                                              toolchain::Value *object,
                                              SILType objectType,
                                              bool suppressCast = false);

/// What isa-encoding mechanism does a type use?
IsaEncoding getIsaEncodingForType(IRGenModule &IGM, CanType type);

} // end namespace irgen
} // end namespace language

#endif
