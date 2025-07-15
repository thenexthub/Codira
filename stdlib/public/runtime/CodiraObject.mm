//===--- CodiraObject.mm - Native Codira Object root class ------------------===//
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
// This implements the Objective-C root class that provides basic `id`-
// compatibility and `NSObject` protocol conformance for pure Codira classes.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#include <objc/NSObject.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/objc.h>
#if __has_include(<objc/objc-internal.h>)
#include <objc/objc-internal.h>
#endif
#endif
#include "toolchain/ADT/StringRef.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/Bincompat.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/CustomRRABI.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/EnvironmentVariables.h"
#include "language/Runtime/Heap.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/ObjCBridge.h"
#include "language/Runtime/Portability.h"
#include "language/Strings.h"
#include "language/Threading/Mutex.h"
#include "language/shims/RuntimeShims.h"
#include "language/shims/AssertionReporting.h"
#include "../CompatibilityOverride/CompatibilityOverride.h"
#include "ErrorObject.h"
#include "Private.h"
#include "CodiraEquatableSupport.h"
#include "CodiraObject.h"
#include "CodiraValue.h"
#include "WeakReference.h"
#if LANGUAGE_OBJC_INTEROP
#include <dlfcn.h>
#endif
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#if LANGUAGE_OBJC_INTEROP
# import <CoreFoundation/CFBase.h> // for CFTypeID
# import <Foundation/Foundation.h>
# include <malloc/malloc.h>
# include <dispatch/dispatch.h>
#endif

using namespace language;
using namespace hashable_support;

#if LANGUAGE_HAS_ISA_MASKING
OBJC_EXPORT __attribute__((__weak_import__))
const uintptr_t objc_debug_isa_class_mask;

uintptr_t language::language_isaMask = LANGUAGE_ISA_MASK;
#endif

const ClassMetadata *language::_language_getClass(const void *object) {
#if LANGUAGE_OBJC_INTEROP
  if (!isObjCTaggedPointer(object))
    return _language_getClassOfAllocated(object);
  return reinterpret_cast<const ClassMetadata*>(
    object_getClass(id_const_cast(object)));
#else
  return _language_getClassOfAllocated(object);
#endif
}

#if LANGUAGE_OBJC_INTEROP
/// Replacement for ObjC object_isClass(), which is unavailable on
/// deployment targets macOS 10.9 and iOS 7.
static bool objcObjectIsClass(id object) {
  // same as object_isClass(object)
  return class_isMetaClass(object_getClass(object));
}

/// Same as _language_getClassOfAllocated() but returns type Class.
static Class _language_getObjCClassOfAllocated(const void *object) {
  return class_const_cast(_language_getClassOfAllocated(object));
}

/// Fetch the ObjC class object associated with the formal dynamic
/// type of the given (possibly Objective-C) object.  The formal
/// dynamic type ignores dynamic subclasses such as those introduced
/// by KVO.
///
/// The object pointer may be a tagged pointer, but cannot be null.
const ClassMetadata *language::language_getObjCClassFromObject(HeapObject *object) {
  auto classAsMetadata = _language_getClass(object);

  // Walk up the superclass chain skipping over artificial Codira classes.
  // If we find a non-Codira class use the result of [object class] instead.

  while (classAsMetadata && classAsMetadata->isTypeMetadata()) {
    if (!classAsMetadata->isArtificialSubclass())
      return classAsMetadata;
    classAsMetadata = classAsMetadata->Superclass;
  }

  id objcObject = reinterpret_cast<id>(object);
  Class objcClass = [objcObject class];
  if (objcObjectIsClass(objcObject)) {
    // Original object is a class. We want a
    // metaclass but +class doesn't give that to us.
    objcClass = object_getClass(objcClass);
  }
  classAsMetadata = reinterpret_cast<const ClassMetadata *>(objcClass);
  return classAsMetadata;
}
#endif

/// Fetch the type metadata associated with the formal dynamic
/// type of the given (possibly Objective-C) object.  The formal
/// dynamic type ignores dynamic subclasses such as those introduced
/// by KVO.
///
/// The object pointer may be a tagged pointer, but cannot be null.
const Metadata *language::language_getObjectType(HeapObject *object) {
  auto classAsMetadata = _language_getClass(object);

#if LANGUAGE_OBJC_INTEROP
  // Walk up the superclass chain skipping over artificial Codira classes.
  // If we find a non-Codira class use the result of [object class] instead.

  while (classAsMetadata && classAsMetadata->isTypeMetadata()) {
    if (!classAsMetadata->isArtificialSubclass())
      return classAsMetadata;
    classAsMetadata = classAsMetadata->Superclass;
  }

  id objcObject = reinterpret_cast<id>(object);
  Class objcClass = [objcObject class];
  if (objcObjectIsClass(objcObject)) {
    // Original object is a class. We want a
    // metaclass but +class doesn't give that to us.
    objcClass = object_getClass(objcClass);
  }
  classAsMetadata = reinterpret_cast<const ClassMetadata *>(objcClass);
  return language_getObjCClassMetadata(classAsMetadata);
#else
  assert(classAsMetadata &&
         classAsMetadata->isTypeMetadata() &&
         !classAsMetadata->isArtificialSubclass());
  return classAsMetadata;
#endif
}

#if LANGUAGE_OBJC_INTEROP
static CodiraObject *_allocHelper(Class cls) {
  // XXX FIXME
  // When we have layout information, do precise alignment rounding
  // For now, assume someone is using hardware vector types
#if defined(__x86_64__) || defined(__i386__)
  const size_t mask = 32 - 1;
#else
  const size_t mask = 16 - 1;
#endif
  return reinterpret_cast<CodiraObject *>(language::language_allocObject(
    reinterpret_cast<HeapMetadata const *>(cls),
    class_getInstanceSize(cls), mask));
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
Class _language_classOfObjCHeapObject(OpaqueValue *value) {
  return _language_getObjCClassOfAllocated(value);
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
id language_stdlib_getDescription(OpaqueValue *value,
                                      const Metadata *type);

id language::getDescription(OpaqueValue *value, const Metadata *type) {
  id result = language_stdlib_getDescription(value, type);
  type->vw_destroy(value);
  return [result autorelease];
}

static id _getObjectDescription(CodiraObject *obj) {
  language_retain((HeapObject*)obj);
  return getDescription((OpaqueValue*)&obj,
                        _language_getClassOfAllocated(obj));
}

static id _getClassDescription(Class cls) {
  const char *name = class_getName(cls);
  int len = strlen(name);
  return [language_stdlib_NSStringFromUTF8(name, len) autorelease];
}

@implementation CodiraObject
+ (void)initialize {
#if LANGUAGE_HAS_ISA_MASKING && !TARGET_OS_SIMULATOR && !NDEBUG
  uintptr_t libObjCMask = (uintptr_t)&objc_absolute_packed_isa_class_mask;
  assert(libObjCMask);

#  if __arm64__ && !__has_feature(ptrauth_calls)
  // When we're built ARM64 but running on ARM64e hardware, we will get an
  // ARM64e libobjc with an ARM64e ISA mask. This mismatch is harmless and we
  // shouldn't assert.
  assert(libObjCMask == LANGUAGE_ISA_MASK || libObjCMask == LANGUAGE_ISA_MASK_PTRAUTH);
#  else
  assert(libObjCMask == LANGUAGE_ISA_MASK);
#  endif
#endif
}

+ (instancetype)allocWithZone:(struct _NSZone *)zone {
  assert(zone == nullptr);
  return _allocHelper(self);
}

+ (instancetype)alloc {
  // we do not support "placement new" or zones,
  // so there is no need to call allocWithZone
  return _allocHelper(self);
}

+ (Class)class {
  return self;
}
- (Class)class {
  return _language_getObjCClassOfAllocated(self);
}
+ (Class)superclass {
  return (Class)((const ClassMetadata*) self)->Superclass;
}
- (Class)superclass {
  return (Class)_language_getClassOfAllocated(self)->Superclass;
}

+ (BOOL)isMemberOfClass:(Class)cls {
  return cls == _language_getObjCClassOfAllocated(self);
}

- (BOOL)isMemberOfClass:(Class)cls {
  return cls == _language_getObjCClassOfAllocated(self);
}

- (instancetype)self {
  return self;
}
- (BOOL)isProxy {
  return NO;
}

- (struct _NSZone *)zone {
  auto zone = malloc_zone_from_ptr(self);
  return (struct _NSZone *)(zone ? zone : malloc_default_zone());
}

- (void)doesNotRecognizeSelector: (SEL) sel {
  Class cls = _language_getObjCClassOfAllocated(self);
  fatalError(/* flags = */ 0,
             "Unrecognized selector %c[%s %s]\n",
             class_isMetaClass(cls) ? '+' : '-',
             class_getName(cls), sel_getName(sel));
}

STANDARD_OBJC_METHOD_IMPLS_FOR_LANGUAGE_OBJECTS

// Retaining the class object itself is a no-op.
+ (id)retain {
  return self;
}
+ (void)release {
  /* empty */
}
+ (id)autorelease {
  return self;
}
+ (NSUInteger)retainCount {
  return ULONG_MAX;
}
+ (BOOL)_isDeallocating {
  return NO;
}
+ (BOOL)_tryRetain {
  return YES;
}
+ (BOOL)allowsWeakReference {
  return YES;
}
+ (BOOL)retainWeakReference {
  return YES;
}

- (BOOL)isKindOfClass:(Class)someClass {
  for (auto cls = _language_getClassOfAllocated(self); cls != nullptr;
       cls = cls->Superclass)
    if (cls == (const ClassMetadata*) someClass)
      return YES;

  return NO;
}

+ (BOOL)isSubclassOfClass:(Class)someClass {
  for (auto cls = (const ClassMetadata*) self; cls != nullptr;
       cls = cls->Superclass)
    if (cls == (const ClassMetadata*) someClass)
      return YES;

  return NO;
}

+ (BOOL)respondsToSelector:(SEL)sel {
  if (!sel) return NO;
  return class_respondsToSelector(_language_getObjCClassOfAllocated(self), sel);
}

- (BOOL)respondsToSelector:(SEL)sel {
  if (!sel) return NO;
  return class_respondsToSelector(_language_getObjCClassOfAllocated(self), sel);
}

+ (BOOL)instancesRespondToSelector:(SEL)sel {
  if (!sel) return NO;
  return class_respondsToSelector(self, sel);
}


+ (IMP)methodForSelector:(SEL)sel {
  return class_getMethodImplementation(object_getClass((id)self), sel);
}

- (IMP)methodForSelector:(SEL)sel {
  return class_getMethodImplementation(object_getClass(self), sel);
}

+ (IMP)instanceMethodForSelector:(SEL)sel {
  return class_getMethodImplementation(self, sel);
}


- (BOOL)conformsToProtocol:(Protocol*)proto {
  if (!proto) return NO;
  auto selfClass = _language_getObjCClassOfAllocated(self);

  // Walk the superclass chain.
  while (selfClass) {
    if (class_conformsToProtocol(selfClass, proto))
      return YES;
    selfClass = class_getSuperclass(selfClass);
  }

  return NO;
}

+ (BOOL)conformsToProtocol:(Protocol*)proto {
  if (!proto) return NO;

  // Walk the superclass chain.
  Class selfClass = self;
  while (selfClass) {
    if (class_conformsToProtocol(selfClass, proto))
      return YES;
    selfClass = class_getSuperclass(selfClass);
  }

  return NO;
}

- (NSUInteger)hash {
  if (runtime::bincompat::useLegacyCodiraObjCHashing()) {
    // Legacy behavior: Don't proxy to Codira Hashable
    return (NSUInteger)self;
  }

  auto selfMetadata = _language_getClassOfAllocated(self);

  // If it's Hashable, use that
  auto hashableConformance =
    reinterpret_cast<const hashable_support::HashableWitnessTable *>(
      language_conformsToProtocolCommon(
	selfMetadata, &hashable_support::HashableProtocolDescriptor));
  if (hashableConformance != NULL) {
    return _language_stdlib_Hashable_hashValue_indirect(
      &self, selfMetadata, hashableConformance);
  }

  // If a type is Equatable (but not Hashable), we
  // have to return something here that is compatible
  // with the `isEqual:` below.
  auto equatableConformance =
    reinterpret_cast<const equatable_support::EquatableWitnessTable *>(
      language_conformsToProtocolCommon(
	selfMetadata, &equatable_support::EquatableProtocolDescriptor));
  if (equatableConformance != nullptr) {
    // Warn once per class about this
    auto selfClass = [self class];
    static Lazy<std::unordered_set<Class>> warned;
    static LazyMutex warnedLock;
    LazyMutex::ScopedLock guard(warnedLock);
    auto result = warned.get().insert(selfClass);
    auto inserted = std::get<1>(result);
    if (inserted) {
      const char *clsName = class_getName([self class]);
      warning(0,
	      "Obj-C `-hash` method was invoked on a Codira object of type `%s` "
	      "that is Equatable but not Hashable; "
	      "this can lead to severe performance problems.\n",
	      clsName);
    }
    // Constant value (yuck!) is the only choice here
    return (NSUInteger)1;
  }

  // Legacy default for types that are neither Hashable nor Equatable.
  return (NSUInteger)self;
}

- (BOOL)isEqual:(id)other {
  if (self == other) {
    return YES;
  }
  if (other == nil) {
    return NO;
  }
  if (runtime::bincompat::useLegacyCodiraObjCHashing()) {
    // Legacy behavior: Don't proxy to Codira Hashable or Equatable
    return NO; // We know the ids are different
  }


  // Get Codira type for self and other
  auto selfMetadata = _language_getClassOfAllocated(self);

  // We use Equatable conformance, which will also work for types that implement
  // Hashable.  If the type implements Equatable but not Hashable, there is a
  // risk that `-hash` and `-isEqual:` might be incompatible.  See notes above
  // for `-hash`
  auto equatableConformance =
    language_conformsToProtocolCommon(
      selfMetadata, &equatable_support::EquatableProtocolDescriptor);
  if (equatableConformance == NULL) {
    return NO;
  }

  // Is the other object a subclass of the parent that
  // actually defined this conformance?
  auto conformingParent =
    findConformingSuperclass(selfMetadata, equatableConformance->getDescription());
  auto otherMetadata = _language_getClassOfAllocated(other);
  if (_language_class_isSubclass(otherMetadata, conformingParent)) {
    // We now have an equatable conformance of a common parent
    // of both object types:
    return _language_stdlib_Equatable_isEqual_indirect(
      &self,
      &other,
      conformingParent,
      reinterpret_cast<const equatable_support::EquatableWitnessTable *>(
	equatableConformance)
    );
  }

  return NO;
}

- (id)performSelector:(SEL)aSelector {
  return ((id(*)(id, SEL))objc_msgSend)(self, aSelector);
}

- (id)performSelector:(SEL)aSelector withObject:(id)object {
  return ((id(*)(id, SEL, id))objc_msgSend)(self, aSelector, object);
}

- (id)performSelector:(SEL)aSelector withObject:(id)object1
                                     withObject:(id)object2 {
  return ((id(*)(id, SEL, id, id))objc_msgSend)(self, aSelector, object1,
                                                                 object2);
}

- (id /* NSString */)description {
  return _getObjectDescription(self);
}
- (id /* NSString */)debugDescription {
  return _getObjectDescription(self);
}

+ (id /* NSString */)description {
  return _getClassDescription(self);
}
+ (id /* NSString */)debugDescription {
  return _getClassDescription(self);
}

- (id /* NSString */)_copyDescription {
  // The NSObject version of this pushes an autoreleasepool in case -description
  // autoreleases, but we're OK with leaking things if we're at the top level
  // of the main thread with no autorelease pool.
  return [[self description] retain];
}

- (CFTypeID)_cfTypeID {
  return (CFTypeID)1; //NSObject's CFTypeID is constant
}

// Foundation collections expect these to be implemented.
- (BOOL)isNSArray__      { return NO; }
- (BOOL)isNSCFConstantString__  { return NO; }
- (BOOL)isNSData__       { return NO; }
- (BOOL)isNSDate__       { return NO; }
- (BOOL)isNSDictionary__ { return NO; }
- (BOOL)isNSObject__     { return NO; }
- (BOOL)isNSOrderedSet__ { return NO; }
- (BOOL)isNSNumber__     { return NO; }
- (BOOL)isNSSet__        { return NO; }
- (BOOL)isNSString__     { return NO; }
- (BOOL)isNSTimeZone__   { return NO; }
- (BOOL)isNSValue__      { return NO; }

@end

#endif

/// Decide dynamically whether the given class uses native Codira
/// reference-counting.
bool language::usesNativeCodiraReferenceCounting(const ClassMetadata *theClass) {
#if LANGUAGE_OBJC_INTEROP
  if (!theClass->isTypeMetadata()) return false;
  return (theClass->getFlags() & ClassFlags::UsesCodiraRefcounting);
#else
  return true;
#endif
}

/// Decide dynamically whether the given type metadata uses native Codira
/// reference-counting.  The metadata is known to correspond to a class
/// type, but note that does not imply being known to be a ClassMetadata
/// due to the existence of ObjCClassWrapper.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
bool
_language_objcClassUsesNativeCodiraReferenceCounting(const Metadata *theClass) {
#if LANGUAGE_OBJC_INTEROP
  // If this is ObjC wrapper metadata, the class is definitely not using
  // Codira ref-counting.
  if (isa<ObjCClassWrapperMetadata>(theClass)) return false;

  // Otherwise, it's class metadata.
  return usesNativeCodiraReferenceCounting(cast<ClassMetadata>(theClass));
#else
  return true;
#endif
}

// The non-pointer bits, excluding the tag bits.
static auto const unTaggedNonNativeBridgeObjectBits
  = heap_object_abi::CodiraSpareBitsMask
  & ~heap_object_abi::ObjCReservedBitsMask
  & ~heap_object_abi::BridgeObjectTagBitsMask;

#if LANGUAGE_OBJC_INTEROP

#if defined(__x86_64__)
static uintptr_t const objectPointerIsObjCBit = 0x4000000000000000ULL;
#elif defined(__LP64__)
static uintptr_t const objectPointerIsObjCBit = 0x4000000000000000ULL;
#else
static uintptr_t const objectPointerIsObjCBit = 0x00000002U;
#endif

void *language::language_unknownObjectRetain_n(void *object, int n) {
  if (isObjCTaggedPointerOrNull(object)) return object;
  if (objectUsesNativeCodiraReferenceCounting(object)) {
    return language_retain_n(static_cast<HeapObject *>(object), n);
  }
  for (int i = 0; i < n; ++i)
    objc_retain(static_cast<id>(object));

  return object;
}

void language::language_unknownObjectRelease_n(void *object, int n) {
  if (isObjCTaggedPointerOrNull(object)) return;
  if (objectUsesNativeCodiraReferenceCounting(object))
    return language_release_n(static_cast<HeapObject *>(object), n);
  for (int i = 0; i < n; ++i)
    objc_release(static_cast<id>(object));
}

void *language::language_unknownObjectRetain(void *object) {
  if (isObjCTaggedPointerOrNull(object)) return object;
  if (objectUsesNativeCodiraReferenceCounting(object)) {
    return language_retain(static_cast<HeapObject *>(object));
  }
  return objc_retain(static_cast<id>(object));
}

void language::language_unknownObjectRelease(void *object) {
  if (isObjCTaggedPointerOrNull(object)) return;
  if (objectUsesNativeCodiraReferenceCounting(object))
    return language_release(static_cast<HeapObject *>(object));
  return objc_release(static_cast<id>(object));
}

void *language::language_nonatomic_unknownObjectRetain_n(void *object, int n) {
  if (isObjCTaggedPointerOrNull(object)) return object;
  if (objectUsesNativeCodiraReferenceCounting(object)) {
    return language_nonatomic_retain_n(static_cast<HeapObject *>(object), n);
  }
  for (int i = 0; i < n; ++i)
    objc_retain(static_cast<id>(object));
  return object;
}

void language::language_nonatomic_unknownObjectRelease_n(void *object, int n) {
  if (isObjCTaggedPointerOrNull(object)) return;
  if (objectUsesNativeCodiraReferenceCounting(object))
    return language_nonatomic_release_n(static_cast<HeapObject *>(object), n);
  for (int i = 0; i < n; ++i)
    objc_release(static_cast<id>(object));
}

void *language::language_nonatomic_unknownObjectRetain(void *object) {
  if (isObjCTaggedPointerOrNull(object)) return object;
  if (objectUsesNativeCodiraReferenceCounting(object)) {
    return language_nonatomic_retain(static_cast<HeapObject *>(object));
  }
  return objc_retain(static_cast<id>(object));
}

void language::language_nonatomic_unknownObjectRelease(void *object) {
  if (isObjCTaggedPointerOrNull(object)) return;
  if (objectUsesNativeCodiraReferenceCounting(object))
    return language_release(static_cast<HeapObject *>(object));
  return objc_release(static_cast<id>(object));
}

/// Return true iff the given BridgeObject is not known to use native
/// reference-counting.
///
/// Precondition: object does not encode a tagged pointer
static bool isNonNative_unTagged_bridgeObject(void *object) {
  static_assert((heap_object_abi::CodiraSpareBitsMask & objectPointerIsObjCBit) ==
                objectPointerIsObjCBit,
                "isObjC bit not within spare bits");
  return (uintptr_t(object) & objectPointerIsObjCBit) != 0
      && (uintptr_t(object) & heap_object_abi::BridgeObjectTagBitsMask) == 0;
}

/// Return true iff the given BridgeObject is a tagged value.
static bool isBridgeObjectTaggedPointer(void *object) {
  return (uintptr_t(object) & heap_object_abi::BridgeObjectTagBitsMask) != 0;
}

#endif

// Mask out the spare bits in a bridgeObject, returning the object it
// encodes.
///
/// Precondition: object does not encode a tagged pointer
static void* toPlainObject_unTagged_bridgeObject(void *object) {
  return (void*)(uintptr_t(object) & ~unTaggedNonNativeBridgeObjectBits);
}

#if LANGUAGE_OBJC_INTEROP
#if __arm64__
// Marking this as noinline allows language_bridgeObjectRetain to avoid emitting
// a stack frame for the language_retain path on ARM64. It makes for worse codegen
// on x86-64, though, so limit it to ARM64.
LANGUAGE_NOINLINE
#endif
static void *objcRetainAndReturn(void *object) {
  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);
  objc_retain(static_cast<id>(objectRef));
  return object;
}
#endif

void *language::language_bridgeObjectRetain(void *object) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return object;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object)) {
    return language_retain(static_cast<HeapObject *>(objectRef));
  }

  // Put the call to objc_retain in a separate function, tail-called here. This
  // allows the fast path of language_bridgeObjectRetain to avoid creating a stack
  // frame on ARM64. We can't directly tail-call objc_retain, because
  // language_bridgeObjectRetain returns the pointer with objectPointerIsObjCBit
  // set, so we have to make a non-tail call and then return the value with the
  // bit set.
  LANGUAGE_MUSTTAIL return objcRetainAndReturn(object);
#else
  // No tail call here. When !LANGUAGE_OBJC_INTEROP, the value of objectRef may be
  // different from that of object, e.g. on Linux ARM64.
  language_retain(static_cast<HeapObject *>(objectRef));
  return object;
#endif
}

CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(language_bridgeObjectRetain)

LANGUAGE_RUNTIME_EXPORT
void *language::language_nonatomic_bridgeObjectRetain(void *object) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return object;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object)) {
    language_nonatomic_retain(static_cast<HeapObject *>(objectRef));
    return object;
  }
  objc_retain(static_cast<id>(objectRef));
  return object;
#else
  language_nonatomic_retain(static_cast<HeapObject *>(objectRef));
  return object;
#endif
}

LANGUAGE_RUNTIME_EXPORT
void language::language_bridgeObjectRelease(void *object) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object))
    return language_release(static_cast<HeapObject *>(objectRef));
  return objc_release(static_cast<id>(objectRef));
#else
  language_release(static_cast<HeapObject *>(objectRef));
#endif
}

CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(language_bridgeObjectRelease)

void language::language_nonatomic_bridgeObjectRelease(void *object) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object))
    return language_nonatomic_release(static_cast<HeapObject *>(objectRef));
  return objc_release(static_cast<id>(objectRef));
#else
  language_nonatomic_release(static_cast<HeapObject *>(objectRef));
#endif
}

void *language::language_bridgeObjectRetain_n(void *object, int n) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return object;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object)) {
    language_retain_n(static_cast<HeapObject *>(objectRef), n);
    return object;
  }
  for (int i = 0;i < n; ++i)
    objc_retain(static_cast<id>(objectRef));
  return object;
#else
  language_retain_n(static_cast<HeapObject *>(objectRef), n);
  return object;
#endif
}

void language::language_bridgeObjectRelease_n(void *object, int n) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object))
    return language_release_n(static_cast<HeapObject *>(objectRef), n);
  for (int i = 0; i < n; ++i)
    objc_release(static_cast<id>(objectRef));
#else
  language_release_n(static_cast<HeapObject *>(objectRef), n);
#endif
}

void *language::language_nonatomic_bridgeObjectRetain_n(void *object, int n) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return object;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object)) {
    language_nonatomic_retain_n(static_cast<HeapObject *>(objectRef), n);
    return object;
  }
  for (int i = 0;i < n; ++i)
    objc_retain(static_cast<id>(objectRef));
  return object;
#else
  language_nonatomic_retain_n(static_cast<HeapObject *>(objectRef), n);
  return object;
#endif
}

void language::language_nonatomic_bridgeObjectRelease_n(void *object, int n) {
#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object) || isBridgeObjectTaggedPointer(object))
    return;
#endif

  auto const objectRef = toPlainObject_unTagged_bridgeObject(object);

#if LANGUAGE_OBJC_INTEROP
  if (!isNonNative_unTagged_bridgeObject(object))
    return language_nonatomic_release_n(static_cast<HeapObject *>(objectRef), n);
  for (int i = 0; i < n; ++i)
    objc_release(static_cast<id>(objectRef));
#else
  language_nonatomic_release_n(static_cast<HeapObject *>(objectRef), n);
#endif
}


#if LANGUAGE_OBJC_INTEROP

/*****************************************************************************/
/************************ UNKNOWN UNOWNED REFERENCES *************************/
/*****************************************************************************/

// Codira's native unowned references are implemented purely with
// reference-counting: as long as an unowned reference is held to an object,
// it can be destroyed but never deallocated, being that it remains fully safe
// to pass around a pointer and perform further reference-counting operations.
//
// For imported class types (meaning ObjC, for now, but in principle any
// type which supports ObjC-style weak references but not directly Codira-style
// unowned references), we have to implement this on top of the weak-reference
// support, at least for now.  But we'd like to be able to statically take
// advantage of Codira's representational advantages when we know that all the
// objects involved are Codira-native.  That means that whatever scheme we use
// for unowned references needs to interoperate with code just doing naive
// loads and stores, at least when the ObjC case isn't triggered.
//
// We have to be sensitive about making unreasonable assumptions about the
// implementation of ObjC weak references, and we definitely cannot modify
// memory owned by the ObjC runtime.  In the long run, direct support from
// the ObjC runtime can allow an efficient implementation that doesn't violate
// those requirements, both by allowing us to directly check whether a weak
// reference was cleared by deallocation vs. just initialized to nil and by
// guaranteeing a bit pattern that distinguishes Codira references.  In the
// meantime, out-of-band allocation is inefficient but not ridiculously so.
//
// Note that unowned references need not provide guaranteed behavior in
// the presence of read/write or write/write races on the reference itself.
// Furthermore, and unlike weak references, they also do not need to be
// safe against races with the deallocation of the object.  It is the user's
// responsibility to ensure that the reference remains valid at the time
// that the unowned reference is read.

namespace {
  /// An Objective-C unowned reference.  Given an unknown unowned reference
  /// in memory, it is an ObjC unowned reference if the IsObjCFlag bit
  /// is set; if so, the pointer stored in the reference actually points
  /// to out-of-line storage containing an ObjC weak reference.
  ///
  /// It is an invariant that this out-of-line storage is only ever
  /// allocated and constructed for non-null object references, so if the
  /// weak load yields null, it can only be because the object was deallocated.
  struct ObjCUnownedReference : UnownedReference {
    // Pretending that there's a subclass relationship here means that
    // accesses to objects formally constructed as UnownedReferences will
    // technically be aliasing violations.  However, the language runtime
    // will generally not see any such objects.

    enum : uintptr_t { IsObjCMask = 0x1, IsObjCFlag = 0x1 };

    /// The out-of-line storage of an ObjC unowned reference.
    struct Storage {
      /// A weak reference registered with the ObjC runtime.
      mutable id WeakRef;

      Storage(id ref) {
        assert(ref && "creating storage for null reference?");
        objc_initWeak(&WeakRef, ref);
      }

      Storage(const Storage &other) {
        objc_copyWeak(&WeakRef, &other.WeakRef);
      }

      Storage &operator=(const Storage &other) = delete;

      Storage &operator=(id ref) {
        objc_storeWeak(&WeakRef, ref);
        return *this;
      }

      ~Storage() {
        objc_destroyWeak(&WeakRef);
      }

      // Don't use the C++ allocator.
      void *operator new(size_t size) { return malloc(size); }
      void operator delete(void *ptr) { free(ptr); }
    };

    Storage *storage() {
      assert(isa<ObjCUnownedReference>(this));
      return reinterpret_cast<Storage*>(
               reinterpret_cast<uintptr_t>(Value) & ~IsObjCMask);
    }

    static void initialize(UnownedReference *dest, id value) {
      initializeWithStorage(dest, new Storage(value));
    }

    static void initializeWithCopy(UnownedReference *dest, Storage *src) {
      initializeWithStorage(dest, new Storage(*src));
    }

    static void initializeWithStorage(UnownedReference *dest,
                                      Storage *storage) {
      dest->Value = (HeapObject*) (uintptr_t(storage) | IsObjCFlag);
    }

    static bool classof(const UnownedReference *ref) {
      return (uintptr_t(ref->Value) & IsObjCMask) == IsObjCFlag;
    }
  };
}

static bool isObjCForUnownedReference(void *value) {
  return (isObjCTaggedPointer(value) ||
          !objectUsesNativeCodiraReferenceCounting(value));
}

UnownedReference *language::language_unknownObjectUnownedInit(UnownedReference *dest,
                                                        void *value) {
  // Note that LLDB also needs to know about the memory layout of unowned
  // references. The implementation here needs to be kept in sync with
  // lldb_private::CodiraLanguageRuntime.
  if (!value) {
    dest->Value = nullptr;
  } else if (isObjCForUnownedReference(value)) {
    ObjCUnownedReference::initialize(dest, (id) value);
  } else {
    language_unownedInit(dest, (HeapObject*) value);
  }
  return dest;
}

UnownedReference *
language::language_unknownObjectUnownedAssign(UnownedReference *dest, void *value) {
  if (!value) {
    language_unknownObjectUnownedDestroy(dest);
    dest->Value = nullptr;
  } else if (isObjCForUnownedReference(value)) {
    if (auto objcDest = dyn_cast<ObjCUnownedReference>(dest)) {
      objc_storeWeak(&objcDest->storage()->WeakRef, (id) value);
    } else {
      language_unownedDestroy(dest);
      ObjCUnownedReference::initialize(dest, (id) value);
    }
  } else {
    if (auto objcDest = dyn_cast<ObjCUnownedReference>(dest)) {
      delete objcDest->storage();
      language_unownedInit(dest, (HeapObject*) value);
    } else {
      language_unownedAssign(dest, (HeapObject*) value);
    }
  }
  return dest;
}

void *language::language_unknownObjectUnownedLoadStrong(UnownedReference *ref) {
  if (!ref->Value) {
    return nullptr;
  } else if (auto objcRef = dyn_cast<ObjCUnownedReference>(ref)) {
    auto result = (void*) objc_loadWeakRetained(&objcRef->storage()->WeakRef);
    if (result == nullptr) {
      language::language_abortRetainUnowned(nullptr);
    }
    return result;
  } else {
    return language_unownedLoadStrong(ref);
  }
}

void *language::language_unknownObjectUnownedTakeStrong(UnownedReference *ref) {
  if (!ref->Value) {
    return nullptr;
  } else if (auto objcRef = dyn_cast<ObjCUnownedReference>(ref)) {
    auto storage = objcRef->storage();
    auto result = (void*) objc_loadWeakRetained(&objcRef->storage()->WeakRef);
    if (result == nullptr) {
      language::language_abortRetainUnowned(nullptr);
    }
    delete storage;
    return result;
  } else {
    return language_unownedTakeStrong(ref);
  }
}

void language::language_unknownObjectUnownedDestroy(UnownedReference *ref) {
  if (!ref->Value) {
    // Nothing to do.
    return;
  } else if (auto objcRef = dyn_cast<ObjCUnownedReference>(ref)) {
    delete objcRef->storage();
  } else {
    language_unownedDestroy(ref);
  }
}

UnownedReference *
language::language_unknownObjectUnownedCopyInit(UnownedReference *dest,
                                          UnownedReference *src) {
  assert(dest != src);
  if (!src->Value) {
    dest->Value = nullptr;
  } else if (auto objcSrc = dyn_cast<ObjCUnownedReference>(src)) {
    ObjCUnownedReference::initializeWithCopy(dest, objcSrc->storage());
  } else {
    language_unownedCopyInit(dest, src);
  }
  return dest;
}

UnownedReference *
language::language_unknownObjectUnownedTakeInit(UnownedReference *dest,
                                          UnownedReference *src) {
  assert(dest != src);
  dest->Value = src->Value;
  return dest;
}

UnownedReference *
language::language_unknownObjectUnownedCopyAssign(UnownedReference *dest,
                                            UnownedReference *src) {
  if (dest == src) return dest;

  if (auto objcSrc = dyn_cast<ObjCUnownedReference>(src)) {
    if (auto objcDest = dyn_cast<ObjCUnownedReference>(dest)) {
      // ObjC unfortunately doesn't expose a copy-assign operation.
      objc_destroyWeak(&objcDest->storage()->WeakRef);
      objc_copyWeak(&objcDest->storage()->WeakRef,
                    &objcSrc->storage()->WeakRef);
      return dest;
    }

    language_unownedDestroy(dest);
    ObjCUnownedReference::initializeWithCopy(dest, objcSrc->storage());
  } else {
    if (auto objcDest = dyn_cast<ObjCUnownedReference>(dest)) {
      delete objcDest->storage();
      language_unownedCopyInit(dest, src);
    } else {
      language_unownedCopyAssign(dest, src);
    }
  }
  return dest;
}

UnownedReference *
language::language_unknownObjectUnownedTakeAssign(UnownedReference *dest,
                                            UnownedReference *src) {
  assert(dest != src);

  // There's not really anything more efficient to do here than this.
  language_unknownObjectUnownedDestroy(dest);
  dest->Value = src->Value;
  return dest;
}

bool language::language_unknownObjectUnownedIsEqual(UnownedReference *ref,
                                              void *value) {
  if (!ref->Value) {
    return value == nullptr;
  } else if (auto objcRef = dyn_cast<ObjCUnownedReference>(ref)) {
    id refValue = objc_loadWeakRetained(&objcRef->storage()->WeakRef);
    bool isEqual = (void*)refValue == value;
    // This ObjC case has no deliberate unowned check here,
    // unlike the Codira case.
    [refValue release];
    return isEqual;
  } else {
    return language_unownedIsEqual(ref, (HeapObject *)value);
  }
}

/*****************************************************************************/
/************************** UNKNOWN WEAK REFERENCES **************************/
/*****************************************************************************/

WeakReference *language::language_unknownObjectWeakInit(WeakReference *ref,
                                                  void *value) {
  ref->unknownInit(value);
  return ref;
}

WeakReference *language::language_unknownObjectWeakAssign(WeakReference *ref,
                                                    void *value) {
  ref->unknownAssign(value);
  return ref;
}

void *language::language_unknownObjectWeakLoadStrong(WeakReference *ref) {
  return ref->unknownLoadStrong();
}

void *language::language_unknownObjectWeakTakeStrong(WeakReference *ref) {
  return ref->unknownTakeStrong();
}

void language::language_unknownObjectWeakDestroy(WeakReference *ref) {
  ref->unknownDestroy();
}

WeakReference *language::language_unknownObjectWeakCopyInit(WeakReference *dest,
                                                      WeakReference *src) {
  dest->unknownCopyInit(src);
  return dest;
}
WeakReference *language::language_unknownObjectWeakTakeInit(WeakReference *dest,
                                                      WeakReference *src) {
  dest->unknownTakeInit(src);
  return dest;
}
WeakReference *language::language_unknownObjectWeakCopyAssign(WeakReference *dest,
                                                        WeakReference *src) {
  dest->unknownCopyAssign(src);
  return dest;
}
WeakReference *language::language_unknownObjectWeakTakeAssign(WeakReference *dest,
                                                        WeakReference *src) {
  dest->unknownTakeAssign(src);
  return dest;
}

// LANGUAGE_OBJC_INTEROP
#endif

/*****************************************************************************/
/******************************* DYNAMIC CASTS *******************************/
/*****************************************************************************/

#if LANGUAGE_OBJC_INTEROP
static const void *
language_dynamicCastObjCClassImpl(const void *object,
                               const ClassMetadata *targetType) {
  // FIXME: We need to decide if this is really how we want to treat 'nil'.
  if (object == nullptr)
    return nullptr;

  if ([id_const_cast(object) isKindOfClass:class_const_cast(targetType)]) {
    return object;
  }

  // For casts to NSError or NSObject, we might need to bridge via the Error
  // protocol. Try it now.
  if (targetType == reinterpret_cast<const ClassMetadata*>(getNSErrorClass()) ||
      targetType == reinterpret_cast<const ClassMetadata*>([NSObject class])) {
    auto srcType = language_getObjCClassMetadata(
        reinterpret_cast<const ClassMetadata*>(
          object_getClass(id_const_cast(object))));
    if (auto srcErrorWitness = findErrorWitness(srcType)) {
      return dynamicCastValueToNSError((OpaqueValue*)&object, srcType,
                                       srcErrorWitness,
                                       DynamicCastFlags::TakeOnSuccess);
    }
  }

  return nullptr;
}

static const void *
language_dynamicCastObjCClassUnconditionalImpl(const void *object,
                                            const ClassMetadata *targetType,
                                            const char *filename,
                                            unsigned line, unsigned column) {
  // FIXME: We need to decide if this is really how we want to treat 'nil'.
  if (object == nullptr)
    return nullptr;

  if ([id_const_cast(object) isKindOfClass:class_const_cast(targetType)]) {
    return object;
  }

  // For casts to NSError or NSObject, we might need to bridge via the Error
  // protocol. Try it now.
  if (targetType == reinterpret_cast<const ClassMetadata*>(getNSErrorClass()) ||
      targetType == reinterpret_cast<const ClassMetadata*>([NSObject class])) {
    auto srcType = language_getObjCClassMetadata(
        reinterpret_cast<const ClassMetadata*>(
          object_getClass(id_const_cast(object))));
    if (auto srcErrorWitness = findErrorWitness(srcType)) {
      return dynamicCastValueToNSError((OpaqueValue*)&object, srcType,
                                       srcErrorWitness,
                                       DynamicCastFlags::TakeOnSuccess);
    }
  }

  Class sourceType = object_getClass(id_const_cast(object));
  language_dynamicCastFailure(reinterpret_cast<const Metadata *>(sourceType),
                           targetType);
}

static const void *
language_dynamicCastForeignClassImpl(const void *object,
                                  const ForeignClassMetadata *targetType) {
  // FIXME: Actually compare CFTypeIDs, once they are available in the metadata.
  return object;
}

static const void *
language_dynamicCastForeignClassUnconditionalImpl(
         const void *object,
         const ForeignClassMetadata *targetType,
         const char *filename,
         unsigned line, unsigned column) {
  // FIXME: Actual compare CFTypeIDs, once they are available in the metadata.
  return object;
}

bool language::objectConformsToObjCProtocol(const void *theObject,
                                         ProtocolDescriptorRef protocol) {
  return [id_const_cast(theObject)
          conformsToProtocol: protocol.getObjCProtocol()];
}


bool language::classConformsToObjCProtocol(const void *theClass,
                                        ProtocolDescriptorRef protocol) {
  return [class_const_cast(theClass)
          conformsToProtocol: protocol.getObjCProtocol()];
}

LANGUAGE_RUNTIME_EXPORT
const Metadata *language_dynamicCastTypeToObjCProtocolUnconditional(
                                               const Metadata *type,
                                               size_t numProtocols,
                                               Protocol * const *protocols,
                                               const char *filename,
                                               unsigned line, unsigned column) {
  Class classObject;

  switch (type->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
    // Native class metadata is also the class object.
    // ObjC class wrappers get unwrapped.
    classObject = type->getObjCClassObject();
    break;

  // Other kinds of type can never conform to ObjC protocols.
  default:
    language_dynamicCastFailure(type, nameForMetadata(type).c_str(),
                             protocols[0], protocol_getName(protocols[0]));

  case MetadataKind::HeapLocalVariable:
  case MetadataKind::HeapGenericLocalVariable:
  case MetadataKind::ErrorObject:
    assert(false && "not type metadata");
    break;
  }

  for (size_t i = 0; i < numProtocols; ++i) {
    if (![classObject conformsToProtocol:protocols[i]]) {
      language_dynamicCastFailure(type, nameForMetadata(type).c_str(),
                               protocols[i], protocol_getName(protocols[i]));
    }
  }

  return type;
}

LANGUAGE_RUNTIME_EXPORT
const Metadata *language_dynamicCastTypeToObjCProtocolConditional(
                                                const Metadata *type,
                                                size_t numProtocols,
                                                Protocol * const *protocols) {
  Class classObject;

  switch (type->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
    // Native class metadata is also the class object.
    // ObjC class wrappers get unwrapped.
    classObject = type->getObjCClassObject();
    break;

  // Other kinds of type can never conform to ObjC protocols.
  default:
    return nullptr;

  case MetadataKind::HeapLocalVariable:
  case MetadataKind::HeapGenericLocalVariable:
  case MetadataKind::ErrorObject:
    assert(false && "not type metadata");
    break;
  }

  for (size_t i = 0; i < numProtocols; ++i) {
    if (![classObject conformsToProtocol:protocols[i]]) {
      return nullptr;
    }
  }

  return type;
}

LANGUAGE_RUNTIME_EXPORT
id language_dynamicCastObjCProtocolUnconditional(id object,
                                              size_t numProtocols,
                                              Protocol * const *protocols,
                                              const char *filename,
                                              unsigned line, unsigned column) {
  for (size_t i = 0; i < numProtocols; ++i) {
    if (![object conformsToProtocol:protocols[i]]) {
      Class sourceType = object_getClass(object);
      language_dynamicCastFailure(sourceType, class_getName(sourceType),
                               protocols[i], protocol_getName(protocols[i]));
    }
  }

  return object;
}

LANGUAGE_RUNTIME_EXPORT
id language_dynamicCastObjCProtocolConditional(id object,
                                            size_t numProtocols,
                                            Protocol * const *protocols) {
  if (!runtime::bincompat::useLegacyCodiraValueUnboxingInCasting()) {
    if (getAsCodiraValue(object) != nil) {
      // CodiraValue wrapper never holds a class object
      return nil;
    }
  }
  for (size_t i = 0; i < numProtocols; ++i) {
    if (![object conformsToProtocol:protocols[i]]) {
      return nil;
    }
  }

  return object;
}

#if OBJC_SUPPORTSLAZYREALIZATION_DEFINED
static bool checkObjCSupportsLazyRealization() {
  if (!LANGUAGE_RUNTIME_WEAK_CHECK(_objc_supportsLazyRealization))
    return false;
  return LANGUAGE_RUNTIME_WEAK_USE(_objc_supportsLazyRealization());
}
#endif

// Check whether the current ObjC runtime supports lazy realization. If it does,
// then we can avoid forcing realization of classes before we use them.
static bool objcSupportsLazyRealization() {
#if OBJC_SUPPORTSLAZYREALIZATION_DEFINED
  return LANGUAGE_LAZY_CONSTANT(checkObjCSupportsLazyRealization());
#else
  return false;
#endif
}

void language::language_instantiateObjCClass(const ClassMetadata *_c) {
  static const objc_image_info ImageInfo = {0, 0};

  Class c = class_const_cast(_c);

  if (!objcSupportsLazyRealization()) {
    // Ensure the superclass is realized.
    [class_getSuperclass(c) class];
  }

  // Register the class.
  Class registered = objc_readClassPair(c, &ImageInfo);
  assert(registered == c
         && "objc_readClassPair failed to instantiate the class in-place");
  (void)registered;
}

Class language::language_getInitializedObjCClass(Class c) {
  if (!objcSupportsLazyRealization()) {
    // Used when we have class metadata and we want to ensure a class has been
    // initialized by the Objective-C runtime. We need to do this because the
    // class "c" might be valid metadata, but it hasn't been initialized yet.
    // Send a message that's likely not to be overridden to minimize potential
    // side effects. Ignore the return value in case it is overridden to
    // return something different. See
    // https://github.com/apple/language/issues/52863 for an example.
    [c self];
  }
  return c;
}

static const ClassMetadata *
language_dynamicCastObjCClassMetatypeImpl(const ClassMetadata *source,
                                       const ClassMetadata *dest) {
  if ([class_const_cast(source) isSubclassOfClass:class_const_cast(dest)])
    return source;
  return nil;
}

static const ClassMetadata *
language_dynamicCastObjCClassMetatypeUnconditionalImpl(const ClassMetadata *source,
                                                    const ClassMetadata *dest,
                                                    const char *filename,
                                                    unsigned line, unsigned column) {
  if ([class_const_cast(source) isSubclassOfClass:class_const_cast(dest)])
    return source;

  language_dynamicCastFailure(source, dest);
}

#endif

static const ClassMetadata *
language_dynamicCastForeignClassMetatypeImpl(const ClassMetadata *sourceType,
                                          const ClassMetadata *targetType) {
  // FIXME: Actually compare CFTypeIDs, once they are available in
  // the metadata.
  return sourceType;
}

static const ClassMetadata *
language_dynamicCastForeignClassMetatypeUnconditionalImpl(
  const ClassMetadata *sourceType,
  const ClassMetadata *targetType,
  const char *filename,
  unsigned line, unsigned column)
{
  // FIXME: Actually compare CFTypeIDs, once they arae available in
  // the metadata.
  return sourceType;
}

#if LANGUAGE_OBJC_INTEROP
// Given a non-nil object reference, return true iff the object uses
// native language reference counting.
static bool usesNativeCodiraReferenceCounting_nonNull(
  const void* object
) {
  assert(object != nullptr);
  return !isObjCTaggedPointer(object) &&
    objectUsesNativeCodiraReferenceCounting(object);
}
#endif

bool language::language_isUniquelyReferenced_nonNull_native(const HeapObject *object){
  assert(object != nullptr);
  assert(!object->refCounts.isDeiniting());
  return object->refCounts.isUniquelyReferenced();
}

bool language::language_isUniquelyReferenced_native(const HeapObject* object) {
  return object != nullptr
    && language::language_isUniquelyReferenced_nonNull_native(object);
}

bool language::language_isUniquelyReferencedNonObjC_nonNull(const void* object) {
  assert(object != nullptr);
  return
#if LANGUAGE_OBJC_INTEROP
    usesNativeCodiraReferenceCounting_nonNull(object) &&
#endif
    language_isUniquelyReferenced_nonNull_native((const HeapObject*)object);
}

#if LANGUAGE_OBJC_INTEROP
static bool isUniquelyReferenced(id object) {
#if OBJC_ISUNIQUELYREFERENCED_DEFINED
  if (!LANGUAGE_RUNTIME_WEAK_CHECK(objc_isUniquelyReferenced))
    return false;
  return LANGUAGE_RUNTIME_WEAK_USE(objc_isUniquelyReferenced(object));
#else
  auto objcIsUniquelyRefd = LANGUAGE_LAZY_CONSTANT(reinterpret_cast<bool (*)(id)>(
      dlsym(RTLD_NEXT, "objc_isUniquelyReferenced")));

  return objcIsUniquelyRefd && objcIsUniquelyRefd(object);
#endif /* OBJC_ISUNIQUELYREFERENCED_DEFINED */
}
#endif

bool language::language_isUniquelyReferenced_nonNull(const void *object) {
  assert(object != nullptr);

#if LANGUAGE_OBJC_INTEROP
  if (isObjCTaggedPointer(object))
    return false;

  if (!usesNativeCodiraReferenceCounting_nonNull(object)) {
    return isUniquelyReferenced(id_const_cast(object));
  }
#endif
  return language_isUniquelyReferenced_nonNull_native(
      static_cast<const HeapObject *>(object));
}

// Given an object reference, return true iff it is non-nil and refers
// to a native language object with strong reference count of 1.
bool language::language_isUniquelyReferencedNonObjC(
  const void* object
) {
  return object != nullptr
    && language_isUniquelyReferencedNonObjC_nonNull(object);
}

// Given an object reference, return true if it is non-nil and refers
// to an ObjC or native language object with a strong reference count of 1.
bool language::language_isUniquelyReferenced(const void *object) {
  return object != nullptr && language_isUniquelyReferenced_nonNull(object);
}

/// Return true if the given bits of a Builtin.BridgeObject refer to a
/// native language object whose strong reference count is 1.
bool language::language_isUniquelyReferencedNonObjC_nonNull_bridgeObject(
  uintptr_t bits
) {
  auto bridgeObject = (void*)bits;

  if (isObjCTaggedPointer(bridgeObject))
    return false;

  const auto object = toPlainObject_unTagged_bridgeObject(bridgeObject);

  // Note: we could just return false if all spare bits are set,
  // but in that case the cost of a deeper check for a unique native
  // object is going to be a negligible cost for a possible big win.
#if LANGUAGE_OBJC_INTEROP
  return !isNonNative_unTagged_bridgeObject(bridgeObject)
             ? language_isUniquelyReferenced_nonNull_native(
                   (const HeapObject *)object)
             : language_isUniquelyReferencedNonObjC_nonNull(object);
#else
  return language_isUniquelyReferenced_nonNull_native((const HeapObject *)object);
#endif
}

/// Return true if the given bits of a Builtin.BridgeObject refer to
/// an object whose strong reference count is 1.
bool language::language_isUniquelyReferenced_nonNull_bridgeObject(uintptr_t bits) {
  auto bridgeObject = reinterpret_cast<void *>(bits);

  if (isObjCTaggedPointer(bridgeObject))
    return false;

  const auto object = toPlainObject_unTagged_bridgeObject(bridgeObject);

#if LANGUAGE_OBJC_INTEROP
  return !isNonNative_unTagged_bridgeObject(bridgeObject)
             ? language_isUniquelyReferenced_nonNull_native(
                   (const HeapObject *)object)
             : language_isUniquelyReferenced_nonNull(object);
#else
  return language_isUniquelyReferenced_nonNull_native((const HeapObject *)object);
#endif
}

// Given a non-@objc object reference, return true iff the
// object is non-nil and has a strong reference count greater than 1
bool language::language_isEscapingClosureAtFileLocation(const HeapObject *object,
                                                  const unsigned char *filename,
                                                  int32_t filenameLength,
                                                  int32_t line, int32_t column,
                                                  unsigned verificationType) {
  assert((verificationType == 0 || verificationType == 1) &&
         "Unknown verification type");

  bool isEscaping =
      object != nullptr && !object->refCounts.isUniquelyReferenced();

  // Print a message if the closure escaped.
  if (isEscaping) {
    auto *message = (verificationType == 0)
                        ? "closure argument was escaped in "
                          "withoutActuallyEscaping block"
                        : "closure argument passed as @noescape "
                          "to Objective-C has escaped";
    auto messageLength = strlen(message);

    char *log;
    language_asprintf(
        &log, "%.*s: file %.*s, line %" PRIu32 ", column %" PRIu32 " \n",
        (int)messageLength, message, filenameLength, filename, line, column);

    printCurrentBacktrace(2/*framesToSkip*/);

    if (_language_shouldReportFatalErrorsToDebugger()) {
      RuntimeErrorDetails details = {
          .version = RuntimeErrorDetails::currentVersion,
          .errorType = "escaping-closure-violation",
          .currentStackDescription = "Closure has escaped",
          .framesToSkip = 1,
          .memoryAddress = nullptr,
          .numExtraThreads = 0,
          .threads = nullptr,
          .numFixIts = 0,
          .fixIts = nullptr,
          .numNotes = 0,
          .notes = nullptr,
      };
      _language_reportToDebugger(RuntimeErrorFlagFatal, log, &details);
    }

    language_reportError(RuntimeErrorFlagFatal, log);
    free(log);
  }
  return isEscaping;
}

struct ClassExtents {
  size_t negative;
  size_t positive; 
};

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
ClassExtents
_language_getCodiraClassInstanceExtents(const Metadata *c) {
  assert(c && c->isClassObject());
  auto metaData = c->getClassObject();
  return ClassExtents{
    metaData->getInstanceAddressPoint(),
    metaData->getInstanceSize() - metaData->getInstanceAddressPoint()
  };
}

#if LANGUAGE_OBJC_INTEROP

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
ClassExtents
_language_getObjCClassInstanceExtents(const ClassMetadata* c) {
  // Pure ObjC classes never have negative extents.
  if (c->isPureObjC())
    return ClassExtents{0, class_getInstanceSize(class_const_cast(c))};

  return _language_getCodiraClassInstanceExtents(c);
}

LANGUAGE_CC(language)
LANGUAGE_RUNTIME_EXPORT
void language_objc_language3ImplicitObjCEntrypoint(id self, SEL selector,
                                             const char *filename,
                                             size_t filenameLength,
                                             size_t line, size_t column,
                                             std::atomic<bool> *didLog) {
  // Only log once. We should have been given a unique zero-initialized
  // atomic flag for each entry point.
  if (didLog->exchange(true))
    return;
  
  // Figure out how much reporting we want by querying the environment
  // variable LANGUAGE_DEBUG_IMPLICIT_OBJC_ENTRYPOINT. We have four meaningful
  // levels:
  //
  //   0: Don't report anything
  //   1: Complain about uses of implicit @objc entrypoints.
  //   2: Complain about uses of implicit @objc entrypoints, with backtraces
  //      if possible.
  //   3: Complain about uses of implicit @objc entrypoints, then abort().
  //
  // The default, if LANGUAGE_DEBUG_IMPLICIT_OBJC_ENTRYPOINT is not set, is 2.
  uint8_t reportLevel =
    runtime::environment::LANGUAGE_DEBUG_IMPLICIT_OBJC_ENTRYPOINT();
  if (reportLevel < 1) return;

  // Report the error.
  uint32_t flags = 0;
  if (reportLevel >= 2)
    flags |= 1 << 0; // Backtrace
  bool isInstanceMethod = !class_isMetaClass(object_getClass(self));
  void (*reporter)(uint32_t, const char *, ...) =
    reportLevel > 2 ? language::fatalError : language::warning;
  
  if (filenameLength > INT_MAX)
    filenameLength = INT_MAX;

  char *message, *nullTerminatedFilename;
  language_asprintf(&message,
           "implicit Objective-C entrypoint %c[%s %s] is deprecated and will "
           "be removed in Codira 4",
           isInstanceMethod ? '-' : '+',
           class_getName([self class]),
           sel_getName(selector));
  language_asprintf(&nullTerminatedFilename, "%*s", (int)filenameLength, filename);

  RuntimeErrorDetails::FixIt fixit = {
    .filename = nullTerminatedFilename,
    .startLine = line,
    .startColumn = column,
    .endLine = line,
    .endColumn = column,
    .replacementText = "@objc "
  };
  RuntimeErrorDetails::Note note = {
    .description = "add '@objc' to expose this Codira declaration to Objective-C",
    .numFixIts = 1,
    .fixIts = &fixit
  };
  RuntimeErrorDetails details = {
    .version = RuntimeErrorDetails::currentVersion,
    .errorType = "implicit-objc-entrypoint",
    .currentStackDescription = nullptr,
    .framesToSkip = 1,
    .memoryAddress = nullptr,
    .numExtraThreads = 0,
    .threads = nullptr,
    .numFixIts = 0,
    .fixIts = nullptr,
    .numNotes = 1,
    .notes = &note
  };
  uintptr_t runtime_error_flags = RuntimeErrorFlagNone;
  if (reporter == language::fatalError)
    runtime_error_flags = RuntimeErrorFlagFatal;
  _language_reportToDebugger(runtime_error_flags, message, &details);

  reporter(flags,
           "*** %s:%zu:%zu: %s; add explicit '@objc' to the declaration to "
           "emit the Objective-C entrypoint in Codira 4 and suppress this "
           "message\n",
           nullTerminatedFilename, line, column, message);
  free(message);
  free(nullTerminatedFilename);
}

const Metadata *language::getNSObjectMetadata() {
  return LANGUAGE_LAZY_CONSTANT(
      language_getObjCClassMetadata((const ClassMetadata *)[NSObject class]));
}

const Metadata *language::getNSStringMetadata() {
  return LANGUAGE_LAZY_CONSTANT(language_getObjCClassMetadata(
    (const ClassMetadata *)objc_lookUpClass("NSString")
  ));
}

const HashableWitnessTable *
language::hashable_support::getNSStringHashableConformance() {
  return LANGUAGE_LAZY_CONSTANT(
    reinterpret_cast<const HashableWitnessTable *>(
      language_conformsToProtocolCommon(
        getNSStringMetadata(),
        &HashableProtocolDescriptor
      )
    )
  );
}

#endif

const ClassMetadata *language::getRootSuperclass() {
#if LANGUAGE_OBJC_INTEROP
  static Lazy<const ClassMetadata *> CodiraObjectClass;

  return CodiraObjectClass.get([](void *ptr) {
    *((const ClassMetadata **) ptr) =
        (const ClassMetadata *)[CodiraObject class];
  });
#else
  return nullptr;
#endif
}

#define OVERRIDE_OBJC COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"

#define OVERRIDE_FOREIGN COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"
