//===--- WeakReference.h - Codira weak references ----------------*- C++ -*-===//
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
// Codira weak reference implementation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_WEAKREFERENCE_H
#define LANGUAGE_RUNTIME_WEAKREFERENCE_H

#include "language/shims/Target.h"
#include "language/shims/Visibility.h"
#include "language/Runtime/Config.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"

#if LANGUAGE_OBJC_INTEROP
#include "language/Runtime/ObjCBridge.h"
#endif

#include "Private.h"

#include <cstdint>

namespace language {

// Note: This implementation of unknown weak makes several assumptions
// about ObjC's weak variables implementation:
// * Nil is stored verbatim.
// * Tagged pointer objects are stored verbatim with no side table entry.
// * Ordinary objects are stored with the LSB two bits (64-bit) or
//   one bit (32-bit) all clear. The stored value otherwise need not be
//   the pointed-to object.
//
// The Codira 3 implementation of unknown weak makes the following
// additional assumptions:
// * Ordinary objects are stored *verbatim* with the LSB *three* bits (64-bit)
//   or *two* bits (32-bit) all clear.

// Thread-safety:
// 
// Reading a weak reference must be thread-safe with respect to:
//  * concurrent readers
//  * concurrent weak reference zeroing due to deallocation of the
//    pointed-to object
//  * concurrent ObjC readers or zeroing (for non-native weak storage)
// 
// Reading a weak reference is NOT thread-safe with respect to:
//  * concurrent writes to the weak variable other than zeroing
//  * concurrent destruction of the weak variable
//
// Writing a weak reference must be thread-safe with respect to:
//  * concurrent weak reference zeroing due to deallocation of the
//    pointed-to object
//  * concurrent ObjC zeroing (for non-native weak storage)
//
// Writing a weak reference is NOT thread-safe with respect to:
//  * concurrent reads
//  * concurrent writes other than zeroing

class WeakReferenceBits {
  // On ObjC platforms, a weak variable may be controlled by the ObjC
  // runtime or by the Codira runtime. NativeMarkerMask and NativeMarkerValue
  // are used to distinguish them.
  //   if ((ptr & NativeMarkerMask) == NativeMarkerValue) it's Codira
  //   else it's ObjC
  // NativeMarkerMask incorporates the ObjC tagged pointer bits
  // plus one more bit that is set in Codira-controlled weak pointer values.
  // Non-ObjC platforms don't use any markers.
  enum : uintptr_t {
#if !LANGUAGE_OBJC_INTEROP
    NativeMarkerMask  = 0,
    NativeMarkerValue = 0
#elif defined(__x86_64__) && LANGUAGE_TARGET_OS_SIMULATOR
    NativeMarkerMask  = LANGUAGE_ABI_X86_64_SIMULATOR_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_X86_64_SIMULATOR_OBJC_WEAK_REFERENCE_MARKER_VALUE
#elif defined(__x86_64__)
    NativeMarkerMask  = LANGUAGE_ABI_X86_64_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_X86_64_OBJC_WEAK_REFERENCE_MARKER_VALUE
#elif defined(__i386__)
    NativeMarkerMask  = LANGUAGE_ABI_I386_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_I386_OBJC_WEAK_REFERENCE_MARKER_VALUE
#elif defined(__arm__) || defined(_M_ARM) || (__arm64__ && __ILP32__)
    NativeMarkerMask  = LANGUAGE_ABI_ARM_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_ARM_OBJC_WEAK_REFERENCE_MARKER_VALUE
#elif defined(__s390x__)
    NativeMarkerMask  = LANGUAGE_ABI_S390X_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_S390X_OBJC_WEAK_REFERENCE_MARKER_VALUE
#elif defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)
    NativeMarkerMask  = LANGUAGE_ABI_ARM64_OBJC_WEAK_REFERENCE_MARKER_MASK,
    NativeMarkerValue = LANGUAGE_ABI_ARM64_OBJC_WEAK_REFERENCE_MARKER_VALUE
#else
    #error unknown architecture
#endif
  };

  static_assert((NativeMarkerMask & NativeMarkerValue) == NativeMarkerValue,
                "native marker value must fall within native marker mask");
  static_assert((NativeMarkerMask & heap_object_abi::CodiraSpareBitsMask)
                == NativeMarkerMask,
                "native marker mask must fall within Codira spare bits");
#if LANGUAGE_OBJC_INTEROP
  static_assert((NativeMarkerMask & heap_object_abi::ObjCReservedBitsMask)
                == heap_object_abi::ObjCReservedBitsMask,
                "native marker mask must contain all ObjC tagged pointer bits");
  static_assert((NativeMarkerValue & heap_object_abi::ObjCReservedBitsMask)
                == 0,
                "native marker value must not interfere with ObjC bits");
#endif

  uintptr_t bits;

 public:
   LANGUAGE_ALWAYS_INLINE
   WeakReferenceBits() {}

   LANGUAGE_ALWAYS_INLINE
   WeakReferenceBits(HeapObjectSideTableEntry *newValue) {
     setNativeOrNull(newValue);
   }

   LANGUAGE_ALWAYS_INLINE
   bool isNativeOrNull() const {
     return bits == 0 || (bits & NativeMarkerMask) == NativeMarkerValue;
   }

   LANGUAGE_ALWAYS_INLINE
   HeapObjectSideTableEntry *getNativeOrNull() const {
     assert(isNativeOrNull());
     if (bits == 0)
       return nullptr;
     return reinterpret_cast<HeapObjectSideTableEntry *>(bits &
                                                         ~NativeMarkerMask);
   }

   LANGUAGE_ALWAYS_INLINE
   void setNativeOrNull(HeapObjectSideTableEntry *newValue) {
     assert((uintptr_t(newValue) & NativeMarkerMask) == 0);
     if (newValue)
       bits = uintptr_t(newValue) | NativeMarkerValue;
     else
       bits = 0;
   }
};


class WeakReference {
  union {
    std::atomic<WeakReferenceBits> nativeValue;
#if LANGUAGE_OBJC_INTEROP
    id nonnativeValue;
#endif
  };

  void destroyOldNativeBits(WeakReferenceBits oldBits) {
    auto oldSide = oldBits.getNativeOrNull();
    if (oldSide)
      oldSide->decrementWeak();
  }
  
  HeapObject *nativeLoadStrongFromBits(WeakReferenceBits bits) {
    auto side = bits.getNativeOrNull();
    return side ? side->tryRetain() : nullptr;
  }
  
  HeapObject *nativeTakeStrongFromBits(WeakReferenceBits bits) {
    auto side = bits.getNativeOrNull();
    if (side) {
      auto obj = side->tryRetain();
      side->decrementWeak();
      return obj;
    } else {
      return nullptr;
    }
  }
  
  void nativeCopyInitFromBits(WeakReferenceBits srcBits) {
    auto side = srcBits.getNativeOrNull();
    if (side)
      side = side->incrementWeak();

    nativeValue.store(WeakReferenceBits(side), std::memory_order_relaxed);
  }

 public:
  
  WeakReference() : nativeValue() {}

  WeakReference(std::nullptr_t)
    : nativeValue(WeakReferenceBits(nullptr)) { }

  WeakReference(const WeakReference& rhs) = delete;


  void nativeInit(HeapObject *object) {
    auto side = object ? object->refCounts.formWeakReference() : nullptr;
    nativeValue.store(WeakReferenceBits(side), std::memory_order_relaxed);
  }
  
  void nativeDestroy() {
    auto oldBits = nativeValue.load(std::memory_order_relaxed);
    nativeValue.store(nullptr, std::memory_order_relaxed);
    destroyOldNativeBits(oldBits);
  }

  void nativeAssign(HeapObject *newObject) {
    if (newObject) {
      assert(objectUsesNativeCodiraReferenceCounting(newObject) &&
             "weak assign native with non-native new object");
    }
    
    auto newSide =
      newObject ? newObject->refCounts.formWeakReference() : nullptr;
    auto newBits = WeakReferenceBits(newSide);

    auto oldBits = nativeValue.load(std::memory_order_relaxed);
    nativeValue.store(newBits, std::memory_order_relaxed);

    assert(oldBits.isNativeOrNull() &&
           "weak assign native with non-native old object");
    destroyOldNativeBits(oldBits);
  }

  HeapObject *nativeLoadStrong() {
    auto bits = nativeValue.load(std::memory_order_relaxed);
    return nativeLoadStrongFromBits(bits);
  }

  HeapObject *nativeTakeStrong() {
    auto bits = nativeValue.load(std::memory_order_relaxed);
    nativeValue.store(nullptr, std::memory_order_relaxed);
    return nativeTakeStrongFromBits(bits);
  }

  void nativeCopyInit(WeakReference *src) {
    auto srcBits = src->nativeValue.load(std::memory_order_relaxed);
    return nativeCopyInitFromBits(srcBits);
  }

  void nativeTakeInit(WeakReference *src) {
    auto srcBits = src->nativeValue.load(std::memory_order_relaxed);
    assert(srcBits.isNativeOrNull());
    src->nativeValue.store(nullptr, std::memory_order_relaxed);
    nativeValue.store(srcBits, std::memory_order_relaxed);
  }

  void nativeCopyAssign(WeakReference *src) {
    if (this == src) return;
    nativeDestroy();
    nativeCopyInit(src);
  }

  void nativeTakeAssign(WeakReference *src) {
    if (this == src) return;
    nativeDestroy();
    nativeTakeInit(src);
  }

#if LANGUAGE_OBJC_INTEROP
 private:
  void nonnativeInit(id object) {
    objc_initWeak(&nonnativeValue, object);
  }

  void initWithNativeness(void *object, bool isNative) {
    if (isNative)
      nativeInit(static_cast<HeapObject *>(object));
    else
      nonnativeInit(static_cast<id>(object));
  }

  void nonnativeDestroy() {
    objc_destroyWeak(&nonnativeValue);
  }
  
  void destroyWithNativeness(bool isNative) {
    if (isNative)
      nativeDestroy();
    else
      nonnativeDestroy();
  }

 public:
  
  void unknownInit(void *object) {
    if (isObjCTaggedPointerOrNull(object)) {
      nonnativeValue = static_cast<id>(object);
    } else {
      bool isNative = objectUsesNativeCodiraReferenceCounting(object);
      initWithNativeness(object, isNative);
    }
  }

  void unknownDestroy() {
    auto oldBits = nativeValue.load(std::memory_order_relaxed);
    destroyWithNativeness(oldBits.isNativeOrNull());
  }

  void unknownAssign(void *newObject) {
    // If the new value is not allocated, simply destroy any old value.
    if (isObjCTaggedPointerOrNull(newObject)) {
      unknownDestroy();
      nonnativeValue = static_cast<id>(newObject);
      return;
    }

    bool newIsNative = objectUsesNativeCodiraReferenceCounting(newObject);
    
    auto oldBits = nativeValue.load(std::memory_order_relaxed);
    bool oldIsNative = oldBits.isNativeOrNull();

    // If they're both native, use the native function.
    if (oldIsNative && newIsNative)
      return nativeAssign(static_cast<HeapObject *>(newObject));

    // If neither is native, use ObjC.
    if (!oldIsNative && !newIsNative)
      return (void) objc_storeWeak(&nonnativeValue, static_cast<id>(newObject));

    // They don't match. Destroy and re-initialize.
    destroyWithNativeness(oldIsNative);
    initWithNativeness(newObject, newIsNative);
  }

  void *unknownLoadStrong() {
    auto bits = nativeValue.load(std::memory_order_relaxed);
    if (bits.isNativeOrNull())
      return nativeLoadStrongFromBits(bits);
    else
      return objc_loadWeakRetained(&nonnativeValue);
  }

  void *unknownTakeStrong() {
    auto bits = nativeValue.load(std::memory_order_relaxed);
    if (bits.isNativeOrNull()) {
      nativeValue.store(nullptr, std::memory_order_relaxed);
      return nativeTakeStrongFromBits(bits);
    }
    else {
      id result = objc_loadWeakRetained(&nonnativeValue);
      objc_destroyWeak(&nonnativeValue);
      return result;
    }
  }

  void unknownCopyInit(WeakReference *src) {
    auto srcBits = src->nativeValue.load(std::memory_order_relaxed);
    if (srcBits.isNativeOrNull())
      nativeCopyInitFromBits(srcBits);
    else
      objc_copyWeak(&nonnativeValue, &src->nonnativeValue);
  }

  void unknownTakeInit(WeakReference *src) {
    auto srcBits = src->nativeValue.load(std::memory_order_relaxed);
    if (srcBits.isNativeOrNull())
      nativeTakeInit(src);
    else
      objc_moveWeak(&nonnativeValue, &src->nonnativeValue);
  }

  void unknownCopyAssign(WeakReference *src) {
    if (this == src) return;
    unknownDestroy();
    unknownCopyInit(src);
  }

  void unknownTakeAssign(WeakReference *src) {
    if (this == src) return;
    unknownDestroy();
    unknownTakeInit(src);
  }

// LANGUAGE_OBJC_INTEROP
#endif

};

static_assert(sizeof(WeakReference) == sizeof(void*),
              "incorrect WeakReference size");
static_assert(alignof(WeakReference) == alignof(void*),
              "incorrect WeakReference alignment");

// namespace language
}

#endif
