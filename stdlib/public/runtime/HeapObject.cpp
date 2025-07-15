//===--- HeapObject.cpp - Codira Language ABI Allocation Support -----------===//
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
// Allocation ABI Shims While the Language is Bootstrapped
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Lazy.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Heap.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/Once.h"
#include "language/ABI/System.h"
#include "MetadataCache.h"
#include "Private.h"
#include "RuntimeInvocationsTracking.h"
#include "WeakReference.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/CustomRRABI.h"
#include "language/Runtime/InstrumentsSupport.h"
#include "language/shims/GlobalObjects.h"
#include "language/shims/RuntimeShims.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <new>
#if LANGUAGE_OBJC_INTEROP
# include <objc/NSObject.h>
# include <objc/runtime.h>
# include <objc/message.h>
# include <objc/objc.h>
# include "language/Runtime/ObjCBridge.h"
# include <dlfcn.h>
#endif
#if LANGUAGE_STDLIB_HAS_MALLOC_TYPE
# include <malloc_type_private.h>
#endif
#include "Leaks.h"

using namespace language;

// Check to make sure the runtime is being built with a compiler that
// supports the Codira calling convention.
//
// If the Codira calling convention is not in use, functions such as
// language_allocBox and language_makeBoxUnique that rely on their return value
// being passed in a register to be compatible with Codira may miscompile on
// some platforms and silently fail.
#if !__has_attribute(languagecall)
#error "The runtime must be built with a compiler that supports languagecall."
#endif

/// Returns true if the pointer passed to a native retain or release is valid.
/// If false, the operation should immediately return.
LANGUAGE_ALWAYS_INLINE
static inline bool isValidPointerForNativeRetain(const void *p) {
#if defined(__arm64__) && (__POINTER_WIDTH__ == 32)
  // arm64_32 is special since it has 32-bit pointers but __arm64__ is true.
  // Catch it early since __POINTER_WIDTH__ is generally non-portable.
  return p != nullptr;
#elif defined(__ANDROID__) && defined(__aarch64__)
  // Check the top of the second byte instead, since Android AArch64 reserves
  // the top byte for its own pointer tagging since Android 11.
  return (intptr_t)((uintptr_t)p << 8) > 0;
#elif defined(__x86_64__) || defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64) || defined(__s390x__) || (defined(__riscv) && __riscv_xlen == 64) || (defined(__powerpc64__) && defined(__LITTLE_ENDIAN__))
  // On these platforms, except s390x, the upper half of address space is reserved for the
  // kernel, so we can assume that pointer values in this range are invalid.
  // On s390x it is theoretically possible to have high bit set but in practice
  // it is unlikely.
  return (intptr_t)p > 0;
#else
  return p != nullptr;
#endif
}

// Call the appropriate implementation of the `name` function, passing `args`
// to the call. This checks for an override in the function pointer. If an
// override is present, it calls that override. Otherwise it directly calls
// the default implementation. This allows the compiler to inline the default
// implementation and avoid the performance penalty of indirecting through
// the function pointer in the common case.
//
// NOTE: the memcpy and asm("") naming shenanigans are to convince the compiler
// not to emit a bunch of ptrauth instructions just to perform the comparison.
// We only want to authenticate the function pointer if we actually call it.
LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD
static HeapObject *_language_allocObject_(HeapMetadata const *metadata,
                                       size_t requiredSize,
                                       size_t requiredAlignmentMask)
                                       asm("__language_allocObject_");
static HeapObject *_language_retain_(HeapObject *object) asm("__language_retain_");
static HeapObject *_language_retain_n_(HeapObject *object, uint32_t n)
  asm("__language_retain_n_");
static void _language_release_(HeapObject *object) asm("__language_release_");
static void _language_release_n_(HeapObject *object, uint32_t n)
  asm("__language_release_n_");
static HeapObject *_language_tryRetain_(HeapObject *object)
  asm("__language_tryRetain_");

#ifdef LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE

#define CALL_IMPL(name, args) do { \
  if (LANGUAGE_UNLIKELY(_language_enableSwizzlingOfAllocationAndRefCountingFunctions_forInstrumentsOnly.load(std::memory_order_relaxed))) \
    return _ ## name args; \
  return _ ## name ## _ args; \
} while(0)

#define CALL_IMPL_CHECK(name, args) do { \
  void *fptr; \
  memcpy(&fptr, (void *)&_ ## name, sizeof(fptr)); \
  extern char _ ## name ## _as_char asm("__" #name "_"); \
  fptr = __ptrauth_language_runtime_function_entry_strip(fptr); \
  if (LANGUAGE_UNLIKELY(fptr != &_ ## name ## _as_char)) { \
    if (LANGUAGE_UNLIKELY(!_language_enableSwizzlingOfAllocationAndRefCountingFunctions_forInstrumentsOnly.load(std::memory_order_relaxed))) { \
      _language_enableSwizzlingOfAllocationAndRefCountingFunctions_forInstrumentsOnly.store(true, std::memory_order_relaxed); \
    } \
    return _ ## name args; \
  } \
  return _ ## name ## _ args; \
  } while(0)
#else

// If retain/release etc. aren't overridable, just call the real implementation.
#define CALL_IMPL(name, args) \
    return _ ## name ## _ args;

#define CALL_IMPL_CHECK(name, args) \
    return _ ## name ## _ args;

#endif

#if LANGUAGE_STDLIB_HAS_MALLOC_TYPE
static malloc_type_summary_t
computeMallocTypeSummary(const HeapMetadata *heapMetadata) {
  assert(isHeapMetadataKind(heapMetadata->getKind()));
  auto *classMetadata = heapMetadata->getClassObject();

  // Objc
  if (classMetadata && classMetadata->isPureObjC())
    return {.type_kind = MALLOC_TYPE_KIND_OBJC};

  return {.type_kind = MALLOC_TYPE_KIND_LANGUAGE};
}

static malloc_type_id_t getMallocTypeId(const HeapMetadata *heapMetadata) {
  uint64_t metadataPtrBits = reinterpret_cast<uint64_t>(heapMetadata);
  uint32_t hash = (metadataPtrBits >> 32) ^ (metadataPtrBits >> 0);

  malloc_type_descriptor_t desc = {
    .hash = hash,
    .summary = computeMallocTypeSummary(heapMetadata)
  };

  return desc.type_id;
}
#endif // LANGUAGE_STDLIB_HAS_MALLOC_TYPE

#ifdef LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE

LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_allocObject)(
    HeapMetadata const *metadata, size_t requiredSize,
    size_t requiredAlignmentMask) = _language_allocObject_;

LANGUAGE_RUNTIME_EXPORT
std::atomic<bool> _language_enableSwizzlingOfAllocationAndRefCountingFunctions_forInstrumentsOnly = false;

LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_retain)(HeapObject *object) =
    _language_retain_;

LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_retain_n)(
    HeapObject *object, uint32_t n) = _language_retain_n_;

LANGUAGE_RUNTIME_EXPORT
void (*LANGUAGE_RT_DECLARE_ENTRY _language_release)(HeapObject *object) =
    _language_release_;

LANGUAGE_RUNTIME_EXPORT
void (*LANGUAGE_RT_DECLARE_ENTRY _language_release_n)(HeapObject *object,
                                                uint32_t n) = _language_release_n_;

LANGUAGE_RUNTIME_EXPORT
HeapObject *(*LANGUAGE_RT_DECLARE_ENTRY _language_tryRetain)(HeapObject *object) =
    _language_tryRetain_;

#endif // LANGUAGE_STDLIB_OVERRIDABLE_RETAIN_RELEASE

static HeapObject *_language_allocObject_(HeapMetadata const *metadata,
                                       size_t requiredSize,
                                       size_t requiredAlignmentMask) {
  assert(isAlignmentMask(requiredAlignmentMask));
#if LANGUAGE_STDLIB_HAS_MALLOC_TYPE
  auto object = reinterpret_cast<HeapObject *>(language_slowAllocTyped(
      requiredSize, requiredAlignmentMask, getMallocTypeId(metadata)));
#else
  auto object = reinterpret_cast<HeapObject *>(
      language_slowAlloc(requiredSize, requiredAlignmentMask));
#endif

  // NOTE: this relies on the C++17 guaranteed semantics of no null-pointer
  // check on the placement new allocator which we have observed on Windows,
  // Linux, and macOS.
  ::new (object) HeapObject(metadata);

  // If leak tracking is enabled, start tracking this object.
  LANGUAGE_LEAKS_START_TRACKING_OBJECT(object);

  LANGUAGE_RT_TRACK_INVOCATION(object, language_allocObject);

  return object;
}

HeapObject *language::language_allocObject(HeapMetadata const *metadata,
                                     size_t requiredSize,
                                     size_t requiredAlignmentMask) {
  CALL_IMPL_CHECK(language_allocObject, (metadata, requiredSize, requiredAlignmentMask));
}

HeapObject *
language::language_initStackObject(HeapMetadata const *metadata,
                             HeapObject *object) {
  object->metadata = metadata;
  object->refCounts.initForNotFreeing();

  LANGUAGE_RT_TRACK_INVOCATION(object, language_initStackObject);
  return object;
}

struct InitStaticObjectContext {
  HeapObject *object;
  HeapMetadata const *metadata;
};

// TODO: We could generate inline code for the fast-path, i.e. the metadata
// pointer is already set. That would be a performance/codesize tradeoff.
HeapObject *
language::language_initStaticObject(HeapMetadata const *metadata,
                              HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_initStaticObject);
  // The token is located at a negative offset from the object header.
  language_once_t *token = ((language_once_t *)object) - 1;

  // We have to initialize the header atomically. Otherwise we could reset the
  // refcount to 1 while another thread already incremented it - and would
  // decrement it to 0 afterwards.
  InitStaticObjectContext Ctx = { object, metadata };
  language::once(
      *token,
      [](void *OpaqueCtx) {
        InitStaticObjectContext *Ctx = (InitStaticObjectContext *)OpaqueCtx;
        Ctx->object->metadata = Ctx->metadata;
        Ctx->object->refCounts.initImmortal();
      },
      &Ctx);

  return object;
}

void
language::language_verifyEndOfLifetime(HeapObject *object) {
  if (object->refCounts.getCount() != 0)
    language::fatalError(/* flags = */ 0,
                      "Fatal error: Stack object escaped\n");

  if (object->refCounts.getUnownedCount() != 1)
    language::fatalError(/* flags = */ 0,
                      "Fatal error: Unowned reference to stack object\n");

  if (object->refCounts.getWeakCount() != 0)
    language::fatalError(/* flags = */ 0,
                      "Fatal error: Weak reference to stack object\n");
}

/// Allocate a reference-counted object on the heap that
/// occupies <size> bytes of maximally-aligned storage.  The object is
/// uninitialized except for its header.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
HeapObject* language_bufferAllocate(
  HeapMetadata const* bufferType, size_t size, size_t alignMask)
{
  return language::language_allocObject(bufferType, size, alignMask);
}

namespace {
/// Heap object destructor for a generic box allocated with language_allocBox.
static LANGUAGE_CC(language) void destroyGenericBox(LANGUAGE_CONTEXT HeapObject *o) {
  auto metadata = static_cast<const GenericBoxHeapMetadata *>(o->metadata);
  // Destroy the object inside.
  auto *value = metadata->project(o);
  metadata->BoxedType->vw_destroy(value);

  // Deallocate the box.
  language_deallocObject(o, metadata->getAllocSize(),
                      metadata->getAllocAlignMask());
}

class BoxCacheEntry {
public:
  FullMetadata<GenericBoxHeapMetadata> Data;

  BoxCacheEntry(const Metadata *type)
    : Data{HeapMetadataHeader{ {/*type layout*/nullptr}, {destroyGenericBox},
                               {/*vwtable*/ nullptr}},
           GenericBoxHeapMetadata{MetadataKind::HeapGenericLocalVariable,
                                  GenericBoxHeapMetadata::getHeaderOffset(type),
                                  type}} {
  }

  intptr_t getKeyIntValueForDump() {
    return reinterpret_cast<intptr_t>(Data.BoxedType);
  }

  bool matchesKey(const Metadata *type) const { return type == Data.BoxedType; }

  friend toolchain::hash_code hash_value(const BoxCacheEntry &value) {
    return toolchain::hash_value(value.Data.BoxedType);
  }

  static size_t getExtraAllocationSize(const Metadata *key) {
    return 0;
  }
  size_t getExtraAllocationSize() const {
    return 0;
  }
};

} // end anonymous namespace

static SimpleGlobalCache<BoxCacheEntry, BoxesTag> Boxes;

BoxPair language::language_makeBoxUnique(OpaqueValue *buffer, const Metadata *type,
                                    size_t alignMask) {
  auto *inlineBuffer = reinterpret_cast<ValueBuffer*>(buffer);
  HeapObject *box = reinterpret_cast<HeapObject *>(inlineBuffer->PrivateData[0]);

  if (!language_isUniquelyReferenced_nonNull_native(box)) {
    auto refAndObjectAddr = BoxPair(language_allocBox(type));
    // Compute the address of the old object.
    auto headerOffset = sizeof(HeapObject) + alignMask & ~alignMask;
    auto *oldObjectAddr = reinterpret_cast<OpaqueValue *>(
        reinterpret_cast<char *>(box) + headerOffset);
    // Copy the data.
    type->vw_initializeWithCopy(refAndObjectAddr.buffer, oldObjectAddr);
    inlineBuffer->PrivateData[0] = refAndObjectAddr.object;
    // Release ownership of the old box.
    language_release(box);
    return refAndObjectAddr;
  } else {
    auto headerOffset = sizeof(HeapObject) + alignMask & ~alignMask;
    auto *objectAddr = reinterpret_cast<OpaqueValue *>(
        reinterpret_cast<char *>(box) + headerOffset);
    return BoxPair{box, objectAddr};
  }
}

BoxPair language::language_allocBox(const Metadata *type) {
  // Get the heap metadata for the box.
  auto metadata = &Boxes.getOrInsert(type).first->Data;

  // Allocate and project the box.
  auto allocation = language_allocObject(metadata, metadata->getAllocSize(),
                                      metadata->getAllocAlignMask());
  auto projection = metadata->project(allocation);

  return BoxPair{allocation, projection};
}

void language::language_deallocBox(HeapObject *o) {
  auto metadata = static_cast<const GenericBoxHeapMetadata *>(o->metadata);
  // Move the object to the deallocating state (+1 -> +0).
  o->refCounts.decrementFromOneNonAtomic();
  language_deallocObject(o, metadata->getAllocSize(),
                      metadata->getAllocAlignMask());
}

OpaqueValue *language::language_projectBox(HeapObject *o) {
  // The compiler will use a nil reference as a way to avoid allocating memory
  // for boxes of empty type. The address of an empty value is always undefined,
  // so we can just return nil back in this case.
  if (!o)
    return nullptr;
  auto metadata = static_cast<const GenericBoxHeapMetadata *>(o->metadata);
  return metadata->project(o);
}

namespace { // Begin anonymous namespace.

struct _CodiraEmptyBoxStorage {
  HeapObject header;
};

language::HeapLocalVariableMetadata _emptyBoxStorageMetadata;

/// The singleton empty box storage object.
_CodiraEmptyBoxStorage _EmptyBoxStorage = {
 // HeapObject header;
  {
    &_emptyBoxStorageMetadata,
  }
};

} // End anonymous namespace.

HeapObject *language::language_allocEmptyBox() {
  auto heapObject = reinterpret_cast<HeapObject*>(&_EmptyBoxStorage);
  language_retain(heapObject);
  return heapObject;
}

// Forward-declare this, but define it after language_release.
extern "C" LANGUAGE_LIBRARY_VISIBILITY LANGUAGE_NOINLINE LANGUAGE_USED void
_language_release_dealloc(HeapObject *object);

LANGUAGE_ALWAYS_INLINE
static HeapObject *_language_retain_(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_retain);
  if (isValidPointerForNativeRetain(object)) {
    // Return the result of increment() to make the eventual call to
    // incrementSlow a tail call, which avoids pushing a stack frame on the fast
    // path on ARM64.
    return object->refCounts.increment(1);
  }
  return object;
}

HeapObject *language::language_retain(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  return language_nonatomic_retain(object);
#else
  CALL_IMPL(language_retain, (object));
#endif
}

CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(language_retain)

HeapObject *language::language_nonatomic_retain(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_retain);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.incrementNonAtomic(1);
  return object;
}

LANGUAGE_ALWAYS_INLINE
static HeapObject *_language_retain_n_(HeapObject *object, uint32_t n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_retain_n);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.increment(n);
  return object;
}

HeapObject *language::language_retain_n(HeapObject *object, uint32_t n) {
#ifdef LANGUAGE_THREADING_NONE
  return language_nonatomic_retain_n(object, n);
#else
  CALL_IMPL(language_retain_n, (object, n));
#endif
}

HeapObject *language::language_nonatomic_retain_n(HeapObject *object, uint32_t n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_retain_n);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.incrementNonAtomic(n);
  return object;
}

LANGUAGE_ALWAYS_INLINE
static void _language_release_(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_release);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.decrementAndMaybeDeinit(1);
}

void language::language_release(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  language_nonatomic_release(object);
#else
  CALL_IMPL(language_release, (object));
#endif
}

CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(language_release)

void language::language_nonatomic_release(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_release);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.decrementAndMaybeDeinitNonAtomic(1);
}

LANGUAGE_ALWAYS_INLINE
static void _language_release_n_(HeapObject *object, uint32_t n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_release_n);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.decrementAndMaybeDeinit(n);
}

void language::language_release_n(HeapObject *object, uint32_t n) {
#ifdef LANGUAGE_THREADING_NONE
  language_nonatomic_release_n(object, n);
#else
  CALL_IMPL(language_release_n, (object, n));
#endif
}

void language::language_nonatomic_release_n(HeapObject *object, uint32_t n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_release_n);
  if (isValidPointerForNativeRetain(object))
    object->refCounts.decrementAndMaybeDeinitNonAtomic(n);
}

size_t language::language_retainCount(HeapObject *object) {
  if (isValidPointerForNativeRetain(object))
    return object->refCounts.getCount();
  return 0;
}

size_t language::language_unownedRetainCount(HeapObject *object) {
  return object->refCounts.getUnownedCount();
}

size_t language::language_weakRetainCount(HeapObject *object) {
  return object->refCounts.getWeakCount();
}

HeapObject *language::language_unownedRetain(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  return static_cast<HeapObject *>(language_nonatomic_unownedRetain(object));
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRetain);
  if (!isValidPointerForNativeRetain(object))
    return object;

  object->refCounts.incrementUnowned(1);
  return object;
#endif
}

// Assert that the metadata is a class or ErrorObject, for unowned operations.
// Other types of metadata are not supposed to be used with unowned.
static void checkMetadataForUnownedRR(HeapObject *object) {
  assert(object->metadata->isClassObject() ||
         object->metadata->getKind() == MetadataKind::ErrorObject);
  if (object->metadata->isClassObject())
    assert(
        static_cast<const ClassMetadata *>(object->metadata)->isTypeMetadata());
}

void language::language_unownedRelease(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  language_nonatomic_unownedRelease(object);
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRelease);
  if (!isValidPointerForNativeRetain(object))
    return;

  checkMetadataForUnownedRR(object);

  if (object->refCounts.decrementUnownedShouldFree(1)) {
    auto classMetadata = static_cast<const ClassMetadata*>(object->metadata);

    language_slowDealloc(object, classMetadata->getInstanceSize(),
                      classMetadata->getInstanceAlignMask());
  }
#endif
}

void *language::language_nonatomic_unownedRetain(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_unownedRetain);
  if (!isValidPointerForNativeRetain(object))
    return object;

  object->refCounts.incrementUnownedNonAtomic(1);
  return object;
}

void language::language_nonatomic_unownedRelease(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_unownedRelease);
  if (!isValidPointerForNativeRetain(object))
    return;

  checkMetadataForUnownedRR(object);

  if (object->refCounts.decrementUnownedShouldFreeNonAtomic(1)) {
    auto classMetadata = static_cast<const ClassMetadata*>(object->metadata);

    language_slowDealloc(object, classMetadata->getInstanceSize(),
                       classMetadata->getInstanceAlignMask());
  }
}

HeapObject *language::language_unownedRetain_n(HeapObject *object, int n) {
#ifdef LANGUAGE_THREADING_NONE
  return language_nonatomic_unownedRetain_n(object, n);
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRetain_n);
  if (!isValidPointerForNativeRetain(object))
    return object;

  object->refCounts.incrementUnowned(n);
  return object;
#endif
}

void language::language_unownedRelease_n(HeapObject *object, int n) {
#ifdef LANGUAGE_THREADING_NONE
  language_nonatomic_unownedRelease_n(object, n);
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRelease_n);
  if (!isValidPointerForNativeRetain(object))
    return;

  checkMetadataForUnownedRR(object);

  if (object->refCounts.decrementUnownedShouldFree(n)) {
    auto classMetadata = static_cast<const ClassMetadata*>(object->metadata);
    language_slowDealloc(object, classMetadata->getInstanceSize(),
                      classMetadata->getInstanceAlignMask());
  }
#endif
}

HeapObject *language::language_nonatomic_unownedRetain_n(HeapObject *object, int n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_unownedRetain_n);
  if (!isValidPointerForNativeRetain(object))
    return object;

  object->refCounts.incrementUnownedNonAtomic(n);
  return object;
}

void language::language_nonatomic_unownedRelease_n(HeapObject *object, int n) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRelease_n);
  if (!isValidPointerForNativeRetain(object))
    return;

  checkMetadataForUnownedRR(object);

  if (object->refCounts.decrementUnownedShouldFreeNonAtomic(n)) {
    auto classMetadata = static_cast<const ClassMetadata*>(object->metadata);
    language_slowDealloc(object, classMetadata->getInstanceSize(),
                      classMetadata->getInstanceAlignMask());
  }
}

LANGUAGE_ALWAYS_INLINE
static HeapObject *_language_tryRetain_(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_tryRetain);
  if (!isValidPointerForNativeRetain(object))
    return nullptr;

#ifdef LANGUAGE_THREADING_NONE
  if (object->refCounts.tryIncrementNonAtomic()) return object;
  else return nullptr;
#else
  if (object->refCounts.tryIncrement()) return object;
  else return nullptr;
#endif
}

HeapObject *language::language_tryRetain(HeapObject *object) {
  CALL_IMPL(language_tryRetain, (object));
}

bool language::language_isDeallocating(HeapObject *object) {
  if (!isValidPointerForNativeRetain(object))
    return false;
  return object->refCounts.isDeiniting();
}

void language::language_setDeallocating(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_setDeallocating);
  object->refCounts.decrementFromOneNonAtomic();
}

HeapObject *language::language_unownedRetainStrong(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  return language_nonatomic_unownedRetainStrong(object);
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRetainStrong);
  if (!isValidPointerForNativeRetain(object))
    return object;
  assert(object->refCounts.getUnownedCount() &&
         "object is not currently unowned-retained");

  if (! object->refCounts.tryIncrement())
    language::language_abortRetainUnowned(object);
  return object;
#endif
}

HeapObject *language::language_nonatomic_unownedRetainStrong(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_unownedRetainStrong);
  if (!isValidPointerForNativeRetain(object))
    return object;
  assert(object->refCounts.getUnownedCount() &&
         "object is not currently unowned-retained");

  if (! object->refCounts.tryIncrementNonAtomic())
    language::language_abortRetainUnowned(object);
  return object;
}

void language::language_unownedRetainStrongAndRelease(HeapObject *object) {
#ifdef LANGUAGE_THREADING_NONE
  language_nonatomic_unownedRetainStrongAndRelease(object);
#else
  LANGUAGE_RT_TRACK_INVOCATION(object, language_unownedRetainStrongAndRelease);
  if (!isValidPointerForNativeRetain(object))
    return;
  assert(object->refCounts.getUnownedCount() &&
         "object is not currently unowned-retained");

  if (! object->refCounts.tryIncrement())
    language::language_abortRetainUnowned(object);

  // This should never cause a deallocation.
  bool dealloc = object->refCounts.decrementUnownedShouldFree(1);
  assert(!dealloc && "retain-strong-and-release caused dealloc?");
  (void) dealloc;
#endif
}

void language::language_nonatomic_unownedRetainStrongAndRelease(HeapObject *object) {
  LANGUAGE_RT_TRACK_INVOCATION(object, language_nonatomic_unownedRetainStrongAndRelease);
  if (!isValidPointerForNativeRetain(object))
    return;
  assert(object->refCounts.getUnownedCount() &&
         "object is not currently unowned-retained");

  if (! object->refCounts.tryIncrementNonAtomic())
    language::language_abortRetainUnowned(object);

  // This should never cause a deallocation.
  bool dealloc = object->refCounts.decrementUnownedShouldFreeNonAtomic(1);
  assert(!dealloc && "retain-strong-and-release caused dealloc?");
  (void) dealloc;
}

void language::language_unownedCheck(HeapObject *object) {
  if (!isValidPointerForNativeRetain(object)) return;
  assert(object->refCounts.getUnownedCount() &&
         "object is not currently unowned-retained");

  if (object->refCounts.isDeiniting())
    language::language_abortRetainUnowned(object);
}

void _language_release_dealloc(HeapObject *object) {
  asFullMetadata(object->metadata)->destroy(object);
}

#if LANGUAGE_OBJC_INTEROP
/// Perform the root -dealloc operation for a class instance.
void language::language_rootObjCDealloc(HeapObject *self) {
  auto metadata = self->metadata;
  assert(metadata->isClassObject());
  auto classMetadata = static_cast<const ClassMetadata*>(metadata);
  assert(classMetadata->isTypeMetadata());
  language_deallocClassInstance(self, classMetadata->getInstanceSize(),
                             classMetadata->getInstanceAlignMask());
}
#endif

void language::language_deallocClassInstance(HeapObject *object,
                                       size_t allocatedSize,
                                       size_t allocatedAlignMask) {
  size_t retainCount = language_retainCount(object);
  if (LANGUAGE_UNLIKELY(retainCount > 1)) {
    auto descriptor = object->metadata->getTypeContextDescriptor();

    language::fatalError(0,
                      "Object %p of class %s deallocated with non-zero retain "
                      "count %zd. This object's deinit, or something called "
                      "from it, may have created a strong reference to self "
                      "which outlived deinit, resulting in a dangling "
                      "reference.\n",
                      object,
                      descriptor ? descriptor->Name.get() : "<unknown>",
                      retainCount);
  }

#if LANGUAGE_OBJC_INTEROP
  // We need to let the ObjC runtime clean up any associated objects or weak
  // references associated with this object.
#if TARGET_OS_SIMULATOR && (__x86_64__ || __i386__)
  const bool fastDeallocSupported = false;
#else
  const bool fastDeallocSupported = true;
#endif

  if (!fastDeallocSupported || !object->refCounts.getPureCodiraDeallocation()) {
    objc_destructInstance((id)object);
  }
#endif

  language_deallocObject(object, allocatedSize, allocatedAlignMask);
}

/// Variant of the above used in constructor failure paths.
void language::language_deallocPartialClassInstance(HeapObject *object,
                                              HeapMetadata const *metadata,
                                              size_t allocatedSize,
                                              size_t allocatedAlignMask) {
  if (!object)
    return;

  // Destroy ivars
  auto *classMetadata = _language_getClassOfAllocated(object)->getClassObject();
  assert(classMetadata && "Not a class?");

#if LANGUAGE_OBJC_INTEROP
  // If the object's class is already pure ObjC class, just release it and move
  // on. There are no ivar destroyers. This avoids attempting to mutate
  // placeholder objects statically created in read-only memory.
  if (classMetadata->isPureObjC()) {
    objc_release((id)object);
    return;
  }
#endif

  while (classMetadata != metadata) {
#if LANGUAGE_OBJC_INTEROP
    // If we have hit a pure Objective-C class, we won't see another ivar
    // destroyer.
    if (classMetadata->isPureObjC()) {
      // Set the class to the pure Objective-C superclass, so that when dealloc
      // runs, it starts at that superclass.
      object_setClass((id)object, class_const_cast(classMetadata));

      // Release the object.
      objc_release((id)object);
      return;
    }
#endif

    if (classMetadata->IVarDestroyer)
      classMetadata->IVarDestroyer(object);

    classMetadata = classMetadata->Superclass->getClassObject();
    assert(classMetadata && "Given metatype not a superclass of object type?");
  }

#if LANGUAGE_OBJC_INTEROP
  // If this class doesn't use Codira-native reference counting, use
  // objc_release instead.
  if (!usesNativeCodiraReferenceCounting(classMetadata)) {
    // Find the pure Objective-C superclass.
    while (!classMetadata->isPureObjC())
      classMetadata = classMetadata->Superclass->getClassObject();

    // Set the class to the pure Objective-C superclass, so that when dealloc
    // runs, it starts at that superclass.
    object_setClass((id)object, class_const_cast(classMetadata));

    // Release the object.
    objc_release((id)object);
    return;
  }
#endif

  // The strong reference count should be +1 -- tear down the object
  bool shouldDeallocate = object->refCounts.decrementShouldDeinit(1);
  assert(shouldDeallocate);
  (void) shouldDeallocate;
  language_deallocClassInstance(object, allocatedSize, allocatedAlignMask);
}

#if !defined(__APPLE__) && LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS
static inline void memset_pattern8(void *b, const void *pattern8, size_t len) {
  char *ptr = static_cast<char *>(b);
  while (len >= 8) {
    memcpy(ptr, pattern8, 8);
    ptr += 8;
    len -= 8;
  }
  memcpy(ptr, pattern8, len);
}
#endif

static inline void language_deallocObjectImpl(HeapObject *object,
                                           size_t allocatedSize,
                                           size_t allocatedAlignMask,
                                           bool isDeiniting) {
  assert(isAlignmentMask(allocatedAlignMask));
  if (!isDeiniting) {
    assert(object->refCounts.isUniquelyReferenced());
    object->refCounts.decrementFromOneNonAtomic();
  }
  assert(object->refCounts.isDeiniting());
  LANGUAGE_RT_TRACK_INVOCATION(object, language_deallocObject);
#if LANGUAGE_RUNTIME_CLOBBER_FREED_OBJECTS
  memset_pattern8((uint8_t *)object + sizeof(HeapObject),
                  "\xF0\xEF\xBE\xAD\xDE\xED\xFE\x0F", // 0x0ffeeddeadbeeff0
                  allocatedSize - sizeof(HeapObject));
#endif

  // If we are tracking leaks, stop tracking this object.
  LANGUAGE_LEAKS_STOP_TRACKING_OBJECT(object);


  // Drop the initial weak retain of the object.
  //
  // If the outstanding weak retain count is 1 (i.e. only the initial
  // weak retain), we can immediately call language_slowDealloc.  This is
  // useful both as a way to eliminate an unnecessary atomic
  // operation, and as a way to avoid calling language_unownedRelease on an
  // object that might be a class object, which simplifies the logic
  // required in language_unownedRelease for determining the size of the
  // object.
  //
  // If we see that there is an outstanding weak retain of the object,
  // we need to fall back on language_release, because it's possible for
  // us to race against a weak retain or a weak release.  But if the
  // outstanding weak retain count is 1, then anyone attempting to
  // increase the weak reference count is inherently racing against
  // deallocation and thus in undefined-behavior territory.  And
  // we can even do this with a normal load!  Here's why:
  //
  // 1. There is an invariant that, if the strong reference count
  // is > 0, then the weak reference count is > 1.
  //
  // 2. The above lets us say simply that, in the absence of
  // races, once a reference count reaches 0, there are no points
  // which happen-after where the reference count is > 0.
  //
  // 3. To not race, a strong retain must happen-before a point
  // where the strong reference count is > 0, and a weak retain
  // must happen-before a point where the weak reference count
  // is > 0.
  //
  // 4. Changes to either the strong and weak reference counts occur
  // in a total order with respect to each other.  This can
  // potentially be done with a weaker memory ordering than
  // sequentially consistent if the architecture provides stronger
  // ordering for memory guaranteed to be co-allocated on a cache
  // line (which the reference count fields are).
  //
  // 5. This function happens-after a point where the strong
  // reference count was 0.
  //
  // 6. Therefore, if a normal load in this function sees a weak
  // reference count of 1, it cannot be racing with a weak retain
  // that is not racing with deallocation:
  //
  //   - A weak retain must happen-before a point where the weak
  //     reference count is > 0.
  //
  //   - This function logically decrements the weak reference
  //     count.  If it is possible for it to see a weak reference
  //     count of 1, then at the end of this function, the
  //     weak reference count will logically be 0.
  //
  //   - There can be no points after that point where the
  //     weak reference count will be > 0.
  //
  //   - Therefore either the weak retain must happen-before this
  //     function, or this function cannot see a weak reference
  //     count of 1, or there is a race.
  //
  // Note that it is okay for there to be a race involving a weak
  // *release* which happens after the strong reference count drops to
  // 0.  However, this is harmless: if our load fails to see the
  // release, we will fall back on language_unownedRelease, which does an
  // atomic decrement (and has the ability to reconstruct
  // allocatedSize and allocatedAlignMask).
  //
  // Note: This shortcut is NOT an optimization.
  // Some allocations passed to language_deallocObject() are not compatible
  // with language_unownedRelease() because they do not have ClassMetadata.

  if (object->refCounts.canBeFreedNow()) {
    // object state DEINITING -> DEAD
    language_slowDealloc(object, allocatedSize, allocatedAlignMask);
  } else {
    // object state DEINITING -> DEINITED
    language_unownedRelease(object);
  }
}

void language::language_deallocObject(HeapObject *object, size_t allocatedSize,
                                size_t allocatedAlignMask) {
  language_deallocObjectImpl(object, allocatedSize, allocatedAlignMask, true);
}

void language::language_deallocUninitializedObject(HeapObject *object,
                                             size_t allocatedSize,
                                             size_t allocatedAlignMask) {
  language_deallocObjectImpl(object, allocatedSize, allocatedAlignMask, false);
}

WeakReference *language::language_weakInit(WeakReference *ref, HeapObject *value) {
  ref->nativeInit(value);
  return ref;
}

WeakReference *language::language_weakAssign(WeakReference *ref, HeapObject *value) {
  ref->nativeAssign(value);
  return ref;
}

HeapObject *language::language_weakLoadStrong(WeakReference *ref) {
  return ref->nativeLoadStrong();
}

HeapObject *language::language_weakTakeStrong(WeakReference *ref) {
  return ref->nativeTakeStrong();
}

void language::language_weakDestroy(WeakReference *ref) {
  ref->nativeDestroy();
}

WeakReference *language::language_weakCopyInit(WeakReference *dest,
                                         WeakReference *src) {
  dest->nativeCopyInit(src);
  return dest;
}

WeakReference *language::language_weakTakeInit(WeakReference *dest,
                                         WeakReference *src) {
  dest->nativeTakeInit(src);
  return dest;
}

WeakReference *language::language_weakCopyAssign(WeakReference *dest,
                                           WeakReference *src) {
  dest->nativeCopyAssign(src);
  return dest;
}

WeakReference *language::language_weakTakeAssign(WeakReference *dest,
                                           WeakReference *src) {
  dest->nativeTakeAssign(src);
  return dest;
}

#ifndef NDEBUG // "not not debug", or "debug-able configurations"

/// Returns true if the "immutable" flag is set on \p object.
///
/// Used for runtime consistency checking of COW buffers.
LANGUAGE_RUNTIME_EXPORT
bool _language_isImmutableCOWBuffer(HeapObject *object) {
  return object->refCounts.isImmutableCOWBuffer();
}

/// Sets the "immutable" flag on \p object to \p immutable and returns the old
/// value of the flag.
///
/// Used for runtime consistency checking of COW buffers.
LANGUAGE_RUNTIME_EXPORT
bool _language_setImmutableCOWBuffer(HeapObject *object, bool immutable) {
  return object->refCounts.setIsImmutableCOWBuffer(immutable);
}

void HeapObject::dump() const {
  auto *Self = const_cast<HeapObject *>(this);
  printf("HeapObject: %p\n", Self);
  printf("HeapMetadata Pointer: %p.\n", Self->metadata);
  printf("Strong Ref Count: %d.\n", Self->refCounts.getCount());
  printf("Unowned Ref Count: %d.\n", Self->refCounts.getUnownedCount());
  printf("Weak Ref Count: %d.\n", Self->refCounts.getWeakCount());
  if (Self->metadata->getKind() == MetadataKind::Class) {
    printf("Uses Native Retain: %s.\n",
           (objectUsesNativeCodiraReferenceCounting(Self) ? "true" : "false"));
  } else {
    printf("Uses Native Retain: Not a class. N/A.\n");
  }
  printf("RefCount Side Table: %p.\n", Self->refCounts.getSideTable());
  printf("Is Deiniting: %s.\n",
         (Self->refCounts.isDeiniting() ? "true" : "false"));
}

#endif
