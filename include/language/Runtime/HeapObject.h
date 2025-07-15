//===--- HeapObject.h - Codira Language Allocation ABI -----------*- C++ -*-===//
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
// Codira Allocation ABI
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_ALLOC_H
#define LANGUAGE_RUNTIME_ALLOC_H

#include <cstddef>
#include <cstdint>
#include "language/Runtime/Config.h"
#include "language/Runtime/Heap.h"

#if LANGUAGE_OBJC_INTEROP
#include <objc/objc.h>
#endif // LANGUAGE_OBJC_INTEROP

// Bring in the definition of HeapObject 
#include "language/shims/HeapObject.h"
#include "language/shims/Visibility.h"

namespace language {
  
struct InProcess;

template <typename Runtime> struct TargetMetadata;
using Metadata = TargetMetadata<InProcess>;
  
template <typename Runtime> struct TargetHeapMetadata;
using HeapMetadata = TargetHeapMetadata<InProcess>;

struct OpaqueValue;

/// Allocates a new heap object.  The returned memory is
/// uninitialized outside of the heap-object header.  The object
/// has an initial retain count of 1, and its metadata is set to
/// the given value.
///
/// At some point "soon after return", it will become an
/// invariant that metadata->getSize(returnValue) will equal
/// requiredSize.
///
/// Either aborts or throws a language exception if the allocation fails.
///
/// \param requiredSize - the required size of the allocation,
///   including the header
/// \param requiredAlignmentMask - the required alignment of the allocation;
///   always one less than a power of 2 that's at least alignof(void*)
/// \return never null
///
/// POSSIBILITIES: The argument order is fair game.  It may be useful
/// to have a variant which guarantees zero-initialized memory.
LANGUAGE_EXTERN_C LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD LANGUAGE_RUNTIME_EXPORT_ATTRIBUTE
HeapObject *language_allocObject(HeapMetadata const *metadata,
                              size_t requiredSize,
                              size_t requiredAlignmentMask);

/// Initializes the object header of a stack allocated object.
///
/// \param metadata - the object's metadata which is stored in the header
/// \param object - the pointer to the object's memory on the stack
/// \returns the passed object pointer.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_initStackObject(HeapMetadata const *metadata,
                                  HeapObject *object);

/// Initializes the object header of a static object which is statically
/// allocated in the data section.
///
/// \param metadata - the object's metadata which is stored in the header
/// \param object - the address of the object in the data section. It is assumed
///        that at offset -1 there is a language_once token allocated.
/// \returns the passed object pointer.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_initStaticObject(HeapMetadata const *metadata,
                                   HeapObject *object);

/// Performs verification that the lifetime of a stack allocated object has
/// ended. It aborts if the reference counts of the object indicate that the
/// object did escape to some other location.
LANGUAGE_RUNTIME_EXPORT
void language_verifyEndOfLifetime(HeapObject *object);

struct BoxPair {
  HeapObject *object;
  OpaqueValue *buffer;
};

/// Allocates a heap object that can contain a value of the given type.
/// Returns a Box structure containing a HeapObject* pointer to the
/// allocated object, and a pointer to the value inside the heap object.
/// The value pointer points to an uninitialized buffer of size and alignment
/// appropriate to store a value of the given type.
/// The heap object has an initial retain count of 1, and its metadata is set
/// such that destroying the heap object destroys the contained value.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_EXPORT
BoxPair language_allocBox(Metadata const *type);

/// Performs a uniqueness check on the pointer to a box structure. If the check
/// fails allocates a new box and stores the pointer in the buffer.
///
///  if (!isUnique(buffer[0]))
///    buffer[0] = language_allocBox(type)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_EXPORT
BoxPair language_makeBoxUnique(OpaqueValue *buffer, Metadata const *type,
                                    size_t alignMask);

/// Returns the address of a heap object representing all empty box types.
LANGUAGE_EXTERN_C LANGUAGE_RETURNS_NONNULL LANGUAGE_NODISCARD LANGUAGE_RUNTIME_EXPORT_ATTRIBUTE
HeapObject* language_allocEmptyBox();

/// Atomically increments the retain count of an object.
///
/// \param object - may be null, in which case this is a no-op
///
/// \return object - we return the object because this enables tail call
/// optimization and the argument register to be live through the call on
/// architectures whose argument and return register is the same register.
///
/// POSSIBILITIES: We may end up wanting a bunch of different variants:
///  - the general version which correctly handles null values, language
///     objects, and ObjC objects
///    - a variant that assumes that its operand is a language object
///      - a variant that can safely use non-atomic operations
///      - maybe a variant that can assume a non-null object
/// It may also prove worthwhile to have this use a custom CC
/// which preserves a larger set of registers.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_retain(HeapObject *object);

LANGUAGE_RUNTIME_EXPORT
HeapObject *language_retain_n(HeapObject *object, uint32_t n);

LANGUAGE_RUNTIME_EXPORT
HeapObject *language_nonatomic_retain(HeapObject *object);

LANGUAGE_RUNTIME_EXPORT
HeapObject* language_nonatomic_retain_n(HeapObject *object, uint32_t n);

/// Atomically increments the reference count of an object, unless it has
/// already been destroyed. Returns nil if the object is dead.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_tryRetain(HeapObject *object);

/// Returns true if an object is in the process of being deallocated.
LANGUAGE_RUNTIME_EXPORT
bool language_isDeallocating(HeapObject *object);

/// Atomically decrements the retain count of an object.  If the
/// retain count reaches zero, the object is destroyed as follows:
///
///   size_t allocSize = object->metadata->destroy(object);
///   if (allocSize) language_deallocObject(object, allocSize);
///
/// \param object - may be null, in which case this is a no-op
///
/// POSSIBILITIES: We may end up wanting a bunch of different variants:
///  - the general version which correctly handles null values, language
///     objects, and ObjC objects
///    - a variant that assumes that its operand is a language object
///      - a variant that can safely use non-atomic operations
///      - maybe a variant that can assume a non-null object
/// It's unlikely that a custom CC would be beneficial here.
LANGUAGE_RUNTIME_EXPORT
void language_release(HeapObject *object);

LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_release(HeapObject *object);

/// Atomically decrements the retain count of an object n times. If the retain
/// count reaches zero, the object is destroyed
LANGUAGE_RUNTIME_EXPORT
void language_release_n(HeapObject *object, uint32_t n);

/// Sets the RC_DEALLOCATING_FLAG flag. This is done non-atomically.
/// The strong reference count of \p object must be 1 and no other thread may
/// retain the object during executing this function.
LANGUAGE_RUNTIME_EXPORT
void language_setDeallocating(HeapObject *object);

LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_release_n(HeapObject *object, uint32_t n);

// Refcounting observation hooks for memory tools. Don't use these.
LANGUAGE_RUNTIME_EXPORT
size_t language_retainCount(HeapObject *object);
LANGUAGE_RUNTIME_EXPORT
size_t language_unownedRetainCount(HeapObject *object);
LANGUAGE_RUNTIME_EXPORT
size_t language_weakRetainCount(HeapObject *object);

/// Is this pointer a non-null unique reference to an object?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferenced(const void *);

/// Is this non-null pointer a unique reference to an object?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferenced_nonNull(const void *);

/// Is this non-null BridgeObject a unique reference to an object?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferenced_nonNull_bridgeObject(uintptr_t bits);

/// Is this pointer a non-null unique reference to an object
/// that uses Codira reference counting?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferencedNonObjC(const void *);

/// Is this non-null pointer a unique reference to an object
/// that uses Codira reference counting?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferencedNonObjC_nonNull(const void *);

/// Is this non-null BridgeObject a unique reference to an object
/// that uses Codira reference counting?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferencedNonObjC_nonNull_bridgeObject(
  uintptr_t bits);

/// Is this native Codira pointer a non-null unique reference to
/// an object?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferenced_native(const struct HeapObject *);

/// Is this non-null native Codira pointer a unique reference to
/// an object?
LANGUAGE_RUNTIME_EXPORT
bool language_isUniquelyReferenced_nonNull_native(const struct HeapObject *);

/// Is this native Codira pointer non-null and has a reference count greater than
/// one.
/// This runtime call will print an error message with file name and location if
/// the closure is escaping but it will not abort.
///
/// \p type: 0 - withoutActuallyEscaping verification
///              Was the closure passed to a withoutActuallyEscaping block
///              escaped in the block?
///          1 - @objc closure sentinel verification
///              Was the closure passed to Objective-C escaped?
LANGUAGE_RUNTIME_EXPORT
bool language_isEscapingClosureAtFileLocation(const struct HeapObject *object,
                                           const unsigned char *filename,
                                           int32_t filenameLength,
                                           int32_t line,
                                           int32_t column,
                                           unsigned type);

/// Deallocate the given memory.
///
/// It must have been returned by language_allocObject and the strong reference
/// must have the RC_DEALLOCATING_FLAG flag set, but otherwise the object is
/// in an unknown state.
///
/// \param object - never null
/// \param allocatedSize - the allocated size of the object from the
///   program's perspective, i.e. the value
/// \param allocatedAlignMask - the alignment requirement that was passed
///   to allocObject
///
/// POSSIBILITIES: It may be useful to have a variant which
/// requires the object to have been fully zeroed from offsets
/// sizeof(CodiraHeapObject) to allocatedSize.
LANGUAGE_RUNTIME_EXPORT
void language_deallocObject(HeapObject *object, size_t allocatedSize,
                         size_t allocatedAlignMask);

/// Deallocate an uninitialized object with a strong reference count of +1.
///
/// It must have been returned by language_allocObject, but otherwise the object is
/// in an unknown state.
///
/// \param object - never null
/// \param allocatedSize - the allocated size of the object from the
///   program's perspective, i.e. the value
/// \param allocatedAlignMask - the alignment requirement that was passed
///   to allocObject
///
LANGUAGE_RUNTIME_EXPORT
void language_deallocUninitializedObject(HeapObject *object, size_t allocatedSize,
                                      size_t allocatedAlignMask);

/// Deallocate the given memory.
///
/// It must have been returned by language_allocObject, possibly used as an
/// Objective-C class instance, and the strong reference must have the
/// RC_DEALLOCATING_FLAG flag set, but otherwise the object is in an unknown
/// state.
///
/// \param object - never null
/// \param allocatedSize - the allocated size of the object from the
///   program's perspective, i.e. the value
/// \param allocatedAlignMask - the alignment requirement that was passed
///   to allocObject
///
/// POSSIBILITIES: It may be useful to have a variant which
/// requires the object to have been fully zeroed from offsets
/// sizeof(CodiraHeapObject) to allocatedSize.
LANGUAGE_RUNTIME_EXPORT
void language_deallocClassInstance(HeapObject *object,
                                 size_t allocatedSize,
                                 size_t allocatedAlignMask);

/// Deallocate the given memory after destroying instance variables.
///
/// Destroys instance variables in classes more derived than the given metatype.
///
/// It must have been returned by language_allocObject, possibly used as an
/// Objective-C class instance, and the strong reference must be equal to 1.
///
/// \param object - may be null
/// \param type - most derived class whose instance variables do not need to
///   be destroyed
/// \param allocatedSize - the allocated size of the object from the
///   program's perspective, i.e. the value
/// \param allocatedAlignMask - the alignment requirement that was passed
///   to allocObject
LANGUAGE_RUNTIME_EXPORT
void language_deallocPartialClassInstance(HeapObject *object,
                                       const HeapMetadata *type,
                                       size_t allocatedSize,
                                       size_t allocatedAlignMask);

/// Deallocate the given memory allocated by language_allocBox; it was returned
/// by language_allocBox but is otherwise in an unknown state. The given Metadata
/// pointer must be the same metadata pointer that was passed to language_allocBox
/// when the memory was allocated.
LANGUAGE_RUNTIME_EXPORT
void language_deallocBox(HeapObject *object);

/// Project the value out of a box. `object` must have been allocated
/// using `language_allocBox`, or by the compiler using a statically-emitted
/// box metadata object.
LANGUAGE_RUNTIME_EXPORT
OpaqueValue *language_projectBox(HeapObject *object);

/// RAII object that wraps a Codira heap object and releases it upon
/// destruction.
class CodiraRAII {
  HeapObject *object;

public:
  CodiraRAII(HeapObject *obj, bool AlreadyRetained) : object(obj) {
    if (!AlreadyRetained)
      language_retain(obj);
  }

  ~CodiraRAII() {
    if (object)
      language_release(object);
  }

  CodiraRAII(const CodiraRAII &other) {
    language_retain(*other);
    object = *other;
    ;
  }
  CodiraRAII(CodiraRAII &&other) : object(*other) {
    other.object = nullptr;
  }
  CodiraRAII &operator=(const CodiraRAII &other) {
    if (object)
      language_release(object);
    language_retain(*other);
    object = *other;
    return *this;
  }
  CodiraRAII &operator=(CodiraRAII &&other) {
    if (object)
      language_release(object);
    object = *other;
    other.object = nullptr;
    return *this;
  }

  HeapObject *operator *() const { return object; }
};

/// RAII object that wraps a Codira object and optionally performs a single
/// retain on that object. Multiple requests to retain the object only perform a
/// single retain, and if that retain has been done then it's automatically
/// released when leaving the scope. This helps implement a defensive retain
/// pattern where you may need to retain an object in some circumstances. This
/// helper makes it easy to retain the object only once even when loops are
/// involved, and do a release to balance the retain on all paths out of the
/// scope.
class CodiraDefensiveRetainRAII {
  HeapObject *object;
  bool didRetain;

public:
  // Noncopyable.
  CodiraDefensiveRetainRAII(const CodiraDefensiveRetainRAII &) = delete;
  CodiraDefensiveRetainRAII &operator=(const CodiraDefensiveRetainRAII &) = delete;

  /// Create a new helper with the given object. The object is not retained
  /// initially.
  CodiraDefensiveRetainRAII(HeapObject *object)
      : object(object), didRetain(false) {}

  ~CodiraDefensiveRetainRAII() {
    if (didRetain)
      language_release(object);
  }

  /// Perform a defensive retain of the object. If a defensive retain has
  /// already been performed, this is a no-op.
  void defensiveRetain() {
    if (!didRetain) {
      language_retain(object);
      didRetain = true;
    }
  }

  /// Take the retain from the helper. This is an optimization for code paths
  /// that want to retain the object long-term, and avoids doing a redundant
  /// retain/release pair. If a defensive retain has not been done, then this
  /// will retain the object, so the caller always gets a +1 on the object.
  void takeRetain() {
    if (!didRetain)
      language_retain(object);
    didRetain = false;
  }

  /// Returns true if the object was defensively retained (and takeRetain not
  /// called). Intended for use in asserts.
  bool isRetained() { return didRetain; }
};

/*****************************************************************************/
/**************************** UNOWNED REFERENCES *****************************/
/*****************************************************************************/

/// An unowned reference in memory.  This is ABI.
struct UnownedReference {
  HeapObject *Value;
};

/// Increment the unowned retain count.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_unownedRetain(HeapObject *value);

/// Decrement the unowned retain count.
LANGUAGE_RUNTIME_EXPORT
void language_unownedRelease(HeapObject *value);

/// Increment the unowned retain count.
LANGUAGE_RUNTIME_EXPORT
void *language_nonatomic_unownedRetain(HeapObject *value);

/// Decrement the unowned retain count.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_unownedRelease(HeapObject *value);

/// Increment the unowned retain count by n.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_unownedRetain_n(HeapObject *value, int n);

/// Decrement the unowned retain count by n.
LANGUAGE_RUNTIME_EXPORT
void language_unownedRelease_n(HeapObject *value, int n);

/// Increment the unowned retain count by n.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_nonatomic_unownedRetain_n(HeapObject *value, int n);

/// Decrement the unowned retain count by n.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_unownedRelease_n(HeapObject *value, int n);

/// Increment the strong retain count of an object, aborting if it has
/// been deallocated.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_unownedRetainStrong(HeapObject *value);

/// Increment the strong retain count of an object, aborting if it has
/// been deallocated.
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_nonatomic_unownedRetainStrong(HeapObject *value);

/// Increment the strong retain count of an object which may have been
/// deallocated, aborting if it has been deallocated, and decrement its
/// unowned reference count.
LANGUAGE_RUNTIME_EXPORT
void language_unownedRetainStrongAndRelease(HeapObject *value);

/// Increment the strong retain count of an object which may have been
/// deallocated, aborting if it has been deallocated, and decrement its
/// unowned reference count.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_unownedRetainStrongAndRelease(HeapObject *value);

/// Aborts if the object has been deallocated.
LANGUAGE_RUNTIME_EXPORT
void language_unownedCheck(HeapObject *value);

static inline void language_unownedInit(UnownedReference *ref, HeapObject *value) {
  ref->Value = value;
  language_unownedRetain(value);
}

static inline void language_unownedAssign(UnownedReference *ref,
                                       HeapObject *value) {
  auto oldValue = ref->Value;
  if (value != oldValue) {
    language_unownedRetain(value);
    ref->Value = value;
    language_unownedRelease(oldValue);
  }
}

static inline HeapObject *language_unownedLoadStrong(UnownedReference *ref) {
  auto value = ref->Value;
  language_unownedRetainStrong(value);
  return value;
}

static inline void *language_unownedTakeStrong(UnownedReference *ref) {
  auto value = ref->Value;
  language_unownedRetainStrongAndRelease(value);
  return value;
}

static inline void language_unownedDestroy(UnownedReference *ref) {
  language_unownedRelease(ref->Value);
}

static inline void language_unownedCopyInit(UnownedReference *dest,
                                         UnownedReference *src) {
  dest->Value = src->Value;
  language_unownedRetain(dest->Value);
}

static inline void language_unownedTakeInit(UnownedReference *dest,
                                         UnownedReference *src) {
  dest->Value = src->Value;
}

static inline void language_unownedCopyAssign(UnownedReference *dest,
                                           UnownedReference *src) {
  auto newValue = src->Value;
  auto oldValue = dest->Value;
  if (newValue != oldValue) {
    dest->Value = newValue;
    language_unownedRetain(newValue);
    language_unownedRelease(oldValue);
  }
}

static inline void language_unownedTakeAssign(UnownedReference *dest,
                                           UnownedReference *src) {
  auto newValue = src->Value;
  auto oldValue = dest->Value;
  dest->Value = newValue;
  language_unownedRelease(oldValue);
}

static inline bool language_unownedIsEqual(UnownedReference *ref,
                                        HeapObject *value) {
  bool isEqual = ref->Value == value;
  if (isEqual)
    language_unownedCheck(value);
  return isEqual;
}

/*****************************************************************************/
/****************************** WEAK REFERENCES ******************************/
/*****************************************************************************/

// Defined in Runtime/WeakReference.h
class WeakReference;

/// Initialize a weak reference.
///
/// \param ref - never null
/// \param value - can be null
/// \return ref
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakInit(WeakReference *ref, HeapObject *value);

/// Assign a new value to a weak reference.
///
/// \param ref - never null
/// \param value - can be null
/// \return ref
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakAssign(WeakReference *ref, HeapObject *value);

/// Load a value from a weak reference.  If the current value is a
/// non-null object that has begun deallocation, returns null;
/// otherwise, retains the object before returning.
///
/// \param ref - never null
/// \return can be null
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_weakLoadStrong(WeakReference *ref);

/// Load a value from a weak reference as if by language_weakLoadStrong,
/// but leaving the reference in an uninitialized state.
///
/// \param ref - never null
/// \return can be null
LANGUAGE_RUNTIME_EXPORT
HeapObject *language_weakTakeStrong(WeakReference *ref);

/// Destroy a weak reference.
///
/// \param ref - never null, but can refer to a null object
LANGUAGE_RUNTIME_EXPORT
void language_weakDestroy(WeakReference *ref);

/// Copy initialize a weak reference.
///
/// \param dest - never null, but can refer to a null object
/// \param src - never null, but can refer to a null object
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakCopyInit(WeakReference *dest, WeakReference *src);

/// Take initialize a weak reference.
///
/// \param dest - never null, but can refer to a null object
/// \param src - never null, but can refer to a null object
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakTakeInit(WeakReference *dest, WeakReference *src);

/// Copy assign a weak reference.
///
/// \param dest - never null, but can refer to a null object
/// \param src - never null, but can refer to a null object
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakCopyAssign(WeakReference *dest, WeakReference *src);

/// Take assign a weak reference.
///
/// \param dest - never null, but can refer to a null object
/// \param src - never null, but can refer to a null object
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_weakTakeAssign(WeakReference *dest, WeakReference *src);

/*****************************************************************************/
/************************* OTHER REFERENCE-COUNTING **************************/
/*****************************************************************************/

LANGUAGE_RUNTIME_EXPORT
void *language_bridgeObjectRetain(void *value);

/// Increment the strong retain count of a bridged object by n.
LANGUAGE_RUNTIME_EXPORT
void *language_bridgeObjectRetain_n(void *value, int n);


LANGUAGE_RUNTIME_EXPORT
void *language_nonatomic_bridgeObjectRetain(void *value);


/// Increment the strong retain count of a bridged object by n.
LANGUAGE_RUNTIME_EXPORT
void *language_nonatomic_bridgeObjectRetain_n(void *value, int n);


/*****************************************************************************/
/************************ UNKNOWN REFERENCE-COUNTING *************************/
/*****************************************************************************/

#if LANGUAGE_OBJC_INTEROP

/// Increment the strong retain count of an object which might not be a native
/// Codira object.
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectRetain(void *value);

/// Increment the strong retain count of an object which might not be a native
/// Codira object by n.
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectRetain_n(void *value, int n);

/// Increment the strong retain count of an object which might not be a native
/// Codira object.
LANGUAGE_RUNTIME_EXPORT
void *language_nonatomic_unknownObjectRetain(void *value);

/// Increment the strong retain count of an object which might not be a native
/// Codira object by n.
LANGUAGE_RUNTIME_EXPORT
void *language_nonatomic_unknownObjectRetain_n(void *value, int n);

#else

static inline void *language_unknownObjectRetain(void *value) {
  return language_retain(static_cast<HeapObject *>(value));
}

static inline void *language_unknownObjectRetain_n(void *value, int n) {
  return language_retain_n(static_cast<HeapObject *>(value), n);
}

static inline void *language_nonatomic_unknownObjectRetain(void *value) {
  return language_nonatomic_retain(static_cast<HeapObject *>(value));
}

static inline void *language_nonatomic_unknownObjectRetain_n(void *value, int n) {
  return language_nonatomic_retain_n(static_cast<HeapObject *>(value), n);
}


#endif // LANGUAGE_OBJC_INTEROP

LANGUAGE_RUNTIME_EXPORT
void language_bridgeObjectRelease(void *value);

/// Decrement the strong retain count of a bridged object by n.
LANGUAGE_RUNTIME_EXPORT
void language_bridgeObjectRelease_n(void *value, int n);

LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_bridgeObjectRelease(void *value);

/// Decrement the strong retain count of a bridged object by n.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_bridgeObjectRelease_n(void *value, int n);

#if LANGUAGE_OBJC_INTEROP

/// Decrement the strong retain count of an object which might not be a native
/// Codira object.
LANGUAGE_RUNTIME_EXPORT
void language_unknownObjectRelease(void *value);

/// Decrement the strong retain count of an object which might not be a native
/// Codira object by n.
LANGUAGE_RUNTIME_EXPORT
void language_unknownObjectRelease_n(void *value, int n);

/// Decrement the strong retain count of an object which might not be a native
/// Codira object.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_unknownObjectRelease(void *value);

/// Decrement the strong retain count of an object which might not be a native
/// Codira object by n.
LANGUAGE_RUNTIME_EXPORT
void language_nonatomic_unknownObjectRelease_n(void *value, int n);

#else

static inline void language_unknownObjectRelease(void *value) {
  language_release(static_cast<HeapObject *>(value));
}

static inline void language_unknownObjectRelease_n(void *value, int n) {
  language_release_n(static_cast<HeapObject *>(value), n);
}

static inline void language_nonatomic_unknownObjectRelease(void *value) {
  language_nonatomic_release(static_cast<HeapObject *>(value));
}

static inline void language_nonatomic_unknownObjectRelease_n(void *value, int n) {
  language_nonatomic_release_n(static_cast<HeapObject *>(value), n);
}

#endif // LANGUAGE_OBJC_INTEROP

/*****************************************************************************/
/************************** UNKNOWN WEAK REFERENCES **************************/
/*****************************************************************************/

#if LANGUAGE_OBJC_INTEROP

/// Initialize a weak reference.
///
/// \param ref - never null
/// \param value - not necessarily a native Codira object; can be null
/// \return ref
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakInit(WeakReference *ref, void *value);

#else

static inline WeakReference *language_unknownObjectWeakInit(WeakReference *ref,
                                                         void *value) {
  return language_weakInit(ref, static_cast<HeapObject *>(value));
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Assign a new value to a weak reference.
///
/// \param ref - never null
/// \param value - not necessarily a native Codira object; can be null
/// \return ref
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakAssign(WeakReference *ref, void *value);

#else

static inline WeakReference *language_unknownObjectWeakAssign(WeakReference *ref,
                                                           void *value) {
  return language_weakAssign(ref, static_cast<HeapObject *>(value));
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Load a value from a weak reference, much like language_weakLoadStrong
/// but without requiring the variable to refer to a native Codira object.
///
/// \param ref - never null
/// \return can be null
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectWeakLoadStrong(WeakReference *ref);

#else

static inline void *language_unknownObjectWeakLoadStrong(WeakReference *ref) {
  return static_cast<void *>(language_weakLoadStrong(ref));
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Load a value from a weak reference as if by
/// language_unknownObjectWeakLoadStrong, but leaving the reference in an
/// uninitialized state.
///
/// \param ref - never null
/// \return can be null
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectWeakTakeStrong(WeakReference *ref);

#else

static inline void *language_unknownObjectWeakTakeStrong(WeakReference *ref) {
  return static_cast<void *>(language_weakTakeStrong(ref));
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Destroy a weak reference variable that might not refer to a native
/// Codira object.
LANGUAGE_RUNTIME_EXPORT
void language_unknownObjectWeakDestroy(WeakReference *object);

#else

static inline void language_unknownObjectWeakDestroy(WeakReference *object) {
  language_weakDestroy(object);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Copy-initialize a weak reference variable from one that might not
/// refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakCopyInit(WeakReference *dest,
                                               WeakReference *src);

#else

static inline WeakReference *
language_unknownObjectWeakCopyInit(WeakReference *dest, WeakReference *src) {
  return language_weakCopyInit(dest, src);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Take-initialize a weak reference variable from one that might not
/// refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakTakeInit(WeakReference *dest,
                                               WeakReference *src);

#else

static inline WeakReference *
language_unknownObjectWeakTakeInit(WeakReference *dest, WeakReference *src) {
  return language_weakTakeInit(dest, src);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Copy-assign a weak reference variable from another when either
/// or both variables might not refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakCopyAssign(WeakReference *dest,
                                                 WeakReference *src);

#else

static inline WeakReference *
language_unknownObjectWeakCopyAssign(WeakReference *dest, WeakReference *src) {
  return language_weakCopyAssign(dest, src);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Take-assign a weak reference variable from another when either
/// or both variables might not refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
WeakReference *language_unknownObjectWeakTakeAssign(WeakReference *dest,
                                                 WeakReference *src);

#else

static inline WeakReference *
language_unknownObjectWeakTakeAssign(WeakReference *dest, WeakReference *src) {
  return language_weakTakeAssign(dest, src);
}

#endif // LANGUAGE_OBJC_INTEROP

/*****************************************************************************/
/************************ UNKNOWN UNOWNED REFERENCES *************************/
/*****************************************************************************/

#if LANGUAGE_OBJC_INTEROP

/// Initialize an unowned reference to an object with unknown reference
/// counting.
/// \return ref
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedInit(UnownedReference *ref,
                                                 void *value);

#else

static inline UnownedReference *
language_unknownObjectUnownedInit(UnownedReference *ref, void *value) {
  language_unownedInit(ref, static_cast<HeapObject*>(value));
  return ref;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Assign to an unowned reference holding an object with unknown reference
/// counting.
/// \return ref
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedAssign(UnownedReference *ref,
                                                   void *value);

#else

static inline UnownedReference *
language_unknownObjectUnownedAssign(UnownedReference *ref, void *value) {
  language_unownedAssign(ref, static_cast<HeapObject*>(value));
  return ref;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Load from an unowned reference to an object with unknown reference
/// counting.
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectUnownedLoadStrong(UnownedReference *ref);

#else

static inline void *
language_unknownObjectUnownedLoadStrong(UnownedReference *ref) {
  return language_unownedLoadStrong(ref);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Take from an unowned reference to an object with unknown reference
/// counting.
LANGUAGE_RUNTIME_EXPORT
void *language_unknownObjectUnownedTakeStrong(UnownedReference *ref);

#else

static inline void *
language_unknownObjectUnownedTakeStrong(UnownedReference *ref) {
  return language_unownedTakeStrong(ref);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP
  
/// Destroy an unowned reference to an object with unknown reference counting.
LANGUAGE_RUNTIME_EXPORT
void language_unknownObjectUnownedDestroy(UnownedReference *ref);

#else

static inline void language_unknownObjectUnownedDestroy(UnownedReference *ref) {
  language_unownedDestroy(ref);
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Copy-initialize an unowned reference variable from one that might not
/// refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedCopyInit(UnownedReference *dest,
                                                     UnownedReference *src);

#else

static inline UnownedReference *
language_unknownObjectUnownedCopyInit(UnownedReference *dest,
                                   UnownedReference *src) {
  language_unownedCopyInit(dest, src);
  return dest;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Take-initialize an unowned reference variable from one that might not
/// refer to a native Codira object.
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedTakeInit(UnownedReference *dest,
                                                     UnownedReference *src);

#else

static inline UnownedReference *
language_unknownObjectUnownedTakeInit(UnownedReference *dest,
                                   UnownedReference *src) {
  language_unownedTakeInit(dest, src);
  return dest;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Copy-assign an unowned reference variable from another when either
/// or both variables might not refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedCopyAssign(UnownedReference *dest,
                                                       UnownedReference *src);

#else

static inline UnownedReference *
language_unknownObjectUnownedCopyAssign(UnownedReference *dest,
                                     UnownedReference *src) {
  language_unownedCopyAssign(dest, src);
  return dest;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Take-assign an unowned reference variable from another when either
/// or both variables might not refer to a native Codira object.
/// \return dest
LANGUAGE_RUNTIME_EXPORT
UnownedReference *language_unknownObjectUnownedTakeAssign(UnownedReference *dest,
                                                       UnownedReference *src);

#else

static inline UnownedReference *
language_unknownObjectUnownedTakeAssign(UnownedReference *dest,
                                     UnownedReference *src) {
  language_unownedTakeAssign(dest, src);
  return dest;
}

#endif // LANGUAGE_OBJC_INTEROP

#if LANGUAGE_OBJC_INTEROP

/// Return `*ref == value` when ref might not refer to a native Codira object.
/// Does not halt when *ref is a dead object as long as *ref != value.
LANGUAGE_RUNTIME_EXPORT
bool language_unknownObjectUnownedIsEqual(UnownedReference *ref, void *value);

#else

static inline bool language_unknownObjectUnownedIsEqual(UnownedReference *ref,
                                                     void *value) {
  return language_unownedIsEqual(ref, static_cast<HeapObject *>(value));
}

#endif // LANGUAGE_OBJC_INTEROP

struct TypeNamePair {
  const char *data;
  uintptr_t length;
};

/// Return the name of a Codira type represented by a metadata object.
/// fn _getTypeName(_ type: Any.Type, qualified: Bool)
///   -> (UnsafePointer<UInt8>, Int)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
TypeNamePair
language_getTypeName(const Metadata *type, bool qualified);

/// Return the mangled name of a Codira type represented by a metadata object.
/// fn _getMangledTypeName(_ type: Any.Type)
///   -> (UnsafePointer<UInt8>, Int)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
TypeNamePair
language_getFunctionFullNameFromMangledName(
        const char *mangledNameStart, uintptr_t mangledNameLength);

/// Return the human-readable full name of the mangled function name passed in.
/// fn _getMangledTypeName(_ mangledName: UnsafePointer<UInt8>,
///                          mangledNameLength: UInt)
///   -> (UnsafePointer<UInt8>, Int)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
TypeNamePair
language_getMangledTypeName(const Metadata *type);

} // end namespace language

#if LANGUAGE_OBJC_INTEROP
/// Standard ObjC lifecycle methods for Codira objects
#define STANDARD_OBJC_METHOD_IMPLS_FOR_LANGUAGE_OBJECTS \
- (id)retain { \
  auto SELF = reinterpret_cast<HeapObject *>(self); \
  return reinterpret_cast<id>(language_retain(SELF)); \
} \
- (oneway void)release { \
  auto SELF = reinterpret_cast<HeapObject *>(self); \
  language_release(SELF); \
} \
- (id)autorelease { \
  return _objc_rootAutorelease(self); \
} \
- (NSUInteger)retainCount { \
  return language::language_retainCount(reinterpret_cast<HeapObject *>(self)); \
} \
- (BOOL)_isDeallocating { \
  return language_isDeallocating(reinterpret_cast<HeapObject *>(self)); \
} \
- (BOOL)_tryRetain { \
  return language_tryRetain(reinterpret_cast<HeapObject*>(self)) != nullptr; \
} \
- (BOOL)allowsWeakReference { \
  return !language_isDeallocating(reinterpret_cast<HeapObject *>(self)); \
} \
- (BOOL)retainWeakReference { \
  return language_tryRetain(reinterpret_cast<HeapObject*>(self)) != nullptr; \
} \
- (void)_setWeaklyReferenced { \
  auto heapObj = reinterpret_cast<HeapObject *>(self); \
  heapObj->refCounts.setPureCodiraDeallocation(false); \
} \
- (void)_noteAssociatedObjects { \
  auto heapObj = reinterpret_cast<HeapObject *>(self); \
  heapObj->refCounts.setPureCodiraDeallocation(false); \
} \
- (void)dealloc { \
  language_rootObjCDealloc(reinterpret_cast<HeapObject *>(self)); \
}

#endif // LANGUAGE_OBJC_INTEROP


#endif // LANGUAGE_RUNTIME_ALLOC_H
