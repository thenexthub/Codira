//===--- HeapObject.h -------------------------------------------*- C++ -*-===//
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
#ifndef LANGUAGE_STDLIB_SHIMS_HEAPOBJECT_H
#define LANGUAGE_STDLIB_SHIMS_HEAPOBJECT_H

#include "RefCount.h"
#include "CodiraStddef.h"
#include "System.h"
#include "Target.h"

#define LANGUAGE_ABI_HEAP_OBJECT_HEADER_SIZE_64 16
#define LANGUAGE_ABI_HEAP_OBJECT_HEADER_SIZE_32 8

#ifndef __language__
#include <type_traits>
#include "language/Basic/type_traits.h"

namespace language {

struct InProcess;

template <typename Target> struct TargetHeapMetadata;
using HeapMetadata = TargetHeapMetadata<InProcess>;
#else
typedef struct HeapMetadata HeapMetadata;
typedef struct HeapObject HeapObject;
#endif

#if !defined(__language__) && __has_feature(ptrauth_calls)
#include <ptrauth.h>
#endif
#ifndef __ptrauth_objc_isa_pointer
#define __ptrauth_objc_isa_pointer
#endif

// The members of the HeapObject header that are not shared by a
// standard Objective-C instance
#define LANGUAGE_HEAPOBJECT_NON_OBJC_MEMBERS       \
  InlineRefCounts refCounts

/// The Codira heap-object header.
/// This must match RefCountedStructTy in IRGen.
struct HeapObject {
  /// This is always a valid pointer to a metadata object.
  HeapMetadata const *__ptrauth_objc_isa_pointer metadata;

#if !LANGUAGE_RUNTIME_EMBEDDED

  LANGUAGE_HEAPOBJECT_NON_OBJC_MEMBERS;

#ifndef __language__
  HeapObject() = default;

  // Initialize a HeapObject header as appropriate for a newly-allocated object.
  constexpr HeapObject(HeapMetadata const *newMetadata) 
    : metadata(newMetadata)
    , refCounts(InlineRefCounts::Initialized)
  { }
  
  // Initialize a HeapObject header for an immortal object
  constexpr HeapObject(HeapMetadata const *newMetadata,
                       InlineRefCounts::Immortal_t immortal)
  : metadata(newMetadata)
  , refCounts(InlineRefCounts::Immortal)
  { }
#endif // __language__

#else // LANGUAGE_RUNTIME_EMBEDDED
  uintptr_t embeddedRefcount;

  // Note: The immortal refcount value is also hard-coded in IRGen in
  // `irgen::emitConstantObject`, and in EmbeddedRuntime.code.
#if __POINTER_WIDTH__ == 64
  static const uintptr_t EmbeddedImmortalRefCount = 0x7fffffffffffffffull;
#elif __POINTER_WIDTH__ == 32
  static const uintptr_t EmbeddedImmortalRefCount = 0x7fffffff;
#elif __POINTER_WIDTH__ == 16
  static const uintptr_t EmbeddedImmortalRefCount = 0x7fff;
#endif

#ifndef __language__
  HeapObject() = default;

  // Initialize a HeapObject header as appropriate for a newly-allocated object.
  constexpr HeapObject(HeapMetadata const *newMetadata)
      : metadata(newMetadata), embeddedRefcount(1) {}

  // Initialize a HeapObject header for an immortal object
  constexpr HeapObject(HeapMetadata const *newMetadata,
                       InlineRefCounts::Immortal_t immortal)
      : metadata(newMetadata), embeddedRefcount(EmbeddedImmortalRefCount) {}
#endif // __language__

#endif // LANGUAGE_RUNTIME_EMBEDDED

#ifndef __language__
#ifndef NDEBUG
  void dump() const LANGUAGE_USED;
#endif
#endif // __language__
};

#ifdef __cplusplus
extern "C" {
#endif

LANGUAGE_RUNTIME_STDLIB_API
void _language_instantiateInertHeapObject(void *address,
                                       const HeapMetadata *metadata);

LANGUAGE_RUNTIME_STDLIB_API
__language_size_t language_retainCount(HeapObject *obj);

LANGUAGE_RUNTIME_STDLIB_API
__language_size_t language_unownedRetainCount(HeapObject *obj);

LANGUAGE_RUNTIME_STDLIB_API
__language_size_t language_weakRetainCount(HeapObject *obj);

#ifdef __cplusplus
} // extern "C"
#endif

#ifndef __language__
static_assert(std::is_trivially_destructible<HeapObject>::value,
              "HeapObject must be trivially destructible");

static_assert(sizeof(HeapObject) == 2*sizeof(void*),
              "HeapObject must be two pointers long");

static_assert(alignof(HeapObject) == alignof(void*),
              "HeapObject must be pointer-aligned");

} // end namespace language
#endif // __language__

/// Global bit masks

// TODO(<rdar://problem/34837179>): Convert each macro below to static consts
// when static consts are visible to SIL.

// The extra inhabitants and spare bits of heap object pointers.
// These must align with the values in IRGen's CodiraTargetInfo.cpp.
#if defined(__x86_64__)

#ifdef __APPLE__
#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DARWIN_X86_64_LEAST_VALID_POINTER
#else
#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER
#endif
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_X86_64_LANGUAGE_SPARE_BITS_MASK
#if LANGUAGE_TARGET_OS_SIMULATOR
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_X86_64_SIMULATOR_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_X86_64_SIMULATOR_OBJC_NUM_RESERVED_LOW_BITS
#else
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_X86_64_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_X86_64_OBJC_NUM_RESERVED_LOW_BITS
#endif

#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64

#elif (defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)) &&     \
      (__POINTER_WIDTH__ == 64)

#ifdef __APPLE__
#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DARWIN_ARM64_LEAST_VALID_POINTER
#else
#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER
#endif
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_ARM64_LANGUAGE_SPARE_BITS_MASK
#if defined(__ANDROID__)
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_ANDROID_ARM64_OBJC_RESERVED_BITS_MASK
#else
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_ARM64_OBJC_RESERVED_BITS_MASK
#endif
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_ARM64_OBJC_NUM_RESERVED_LOW_BITS
#if defined(__ANDROID__)
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64 >> 8
#else
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64
#endif

#elif defined(__powerpc64__)

#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_POWERPC64_LANGUAGE_SPARE_BITS_MASK
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_DEFAULT_OBJC_NUM_RESERVED_LOW_BITS
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64

#elif defined(__s390x__)

#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_S390X_LANGUAGE_SPARE_BITS_MASK
#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_S390X_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_S390X_OBJC_NUM_RESERVED_LOW_BITS
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64

#elif defined(__wasm32__)

#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_WASM32_LEAST_VALID_POINTER

#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LANGUAGE_SPARE_BITS_MASK

#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_DEFAULT_OBJC_NUM_RESERVED_LOW_BITS

#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_32

#else

#define _language_abi_LeastValidPointerValue                                      \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LEAST_VALID_POINTER

#if defined(__i386__)
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_I386_LANGUAGE_SPARE_BITS_MASK
#elif defined(__arm__) || defined(_M_ARM) ||                                   \
      (defined(__arm64__) && (__POINTER_WIDTH__ == 32))
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_ARM_LANGUAGE_SPARE_BITS_MASK
#elif defined(__ppc__)
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_POWERPC_LANGUAGE_SPARE_BITS_MASK
#else
#define _language_abi_CodiraSpareBitsMask                                          \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_LANGUAGE_SPARE_BITS_MASK
#endif

#define _language_abi_ObjCReservedBitsMask                                        \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_OBJC_RESERVED_BITS_MASK
#define _language_abi_ObjCReservedLowBits                                         \
  (unsigned) LANGUAGE_ABI_DEFAULT_OBJC_NUM_RESERVED_LOW_BITS

#if __POINTER_WIDTH__ == 64
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_64
#else
#define _language_BridgeObject_TaggedPointerBits                                  \
  (__language_uintptr_t) LANGUAGE_ABI_DEFAULT_BRIDGEOBJECT_TAG_32
#endif

#endif

/// Corresponding namespaced decls
#ifdef __cplusplus
namespace heap_object_abi {
static const __language_uintptr_t LeastValidPointerValue =
    _language_abi_LeastValidPointerValue;
static const __language_uintptr_t CodiraSpareBitsMask =
    _language_abi_CodiraSpareBitsMask;
static const __language_uintptr_t ObjCReservedBitsMask =
    _language_abi_ObjCReservedBitsMask;
static const unsigned ObjCReservedLowBits =
    _language_abi_ObjCReservedLowBits;
static const __language_uintptr_t BridgeObjectTagBitsMask =
    _language_BridgeObject_TaggedPointerBits;
} // heap_object_abi
#endif // __cplusplus

#endif // LANGUAGE_STDLIB_SHIMS_HEAPOBJECT_H
