//===--- CodiraValue.mm - Boxed Codira value class --------------------------===//
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
// This implements the Objective-C class that is used to carry Codira values
// that have been bridged to Objective-C objects without special handling.
// The class is opaque to user code, but is `NSObject`- and `NSCopying`-
// conforming and is understood by the Codira runtime for dynamic casting
// back to the contained type.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#include "CodiraObject.h"
#include "CodiraValue.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/Bincompat.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/ObjCBridge.h"
#include "language/Runtime/Debug.h"
#include "language/Threading/Mutex.h"
#include "Private.h"
#include "CodiraEquatableSupport.h"
#include "CodiraHashableSupport.h"
#include <objc/runtime.h>
#include <Foundation/Foundation.h>

#include <new>
#include <unordered_set>

using namespace language;
using namespace language::hashable_support;

// TODO: Making this a CodiraObject subclass would let us use Codira refcounting,
// but we would need to be able to emit __CodiraValue's Objective-C class object
// with the Codira destructor pointer prefixed before it.
//
// The layout of `__CodiraValue` is:
// - object header,
// - `CodiraValueHeader` instance,
// - the payload, tail-allocated (the Codira value contained in this box).
//
// NOTE: older runtimes called this _CodiraValue. The two must
// coexist, so it was renamed. The old name must not be used in the new
// runtime.
@interface __CodiraValue : NSObject <NSCopying>

- (id)copyWithZone:(NSZone *)zone;

@end

/// The fixed-size ivars of `__CodiraValue`.  The actual boxed value is
/// tail-allocated.
struct CodiraValueHeader {
  /// The type of the value contained in the `__CodiraValue` box.
  const Metadata *type;

  /// The base type that introduces the `Hashable` or `Equatable` conformance.
  /// This member is lazily-initialized.
  /// Instead of using it directly, call `getHashableBaseType()` or `getEquatableBaseType()`
  /// Value here is encoded:
  /// * Least-significant bit set: This is an Equatable base type
  /// * Least-significant bit not set: This is a Hashable base type
  mutable std::atomic<uintptr_t> cachedBaseType;

  /// The witness table for `Hashable` conformance.
  /// This member is lazily-initialized.
  /// Instead of using it directly, call `getHashableConformance()`.
  /// Value here is encoded:
  /// * Least-significant bit set: This is an Equatable conformance
  /// * Least-significant bit not set: This is a Hashable conformance
  mutable std::atomic<uintptr_t> cachedConformance;

  /// Get the base type that conforms to `Hashable`.
  /// Returns NULL if the type does not conform.
  const Metadata *getHashableBaseType() const;

  /// Get the base type that conforms to `Equatable`.
  /// Returns NULL if the type does not conform.
  const Metadata *getEquatableBaseType() const;

  /// Get the `Hashable` protocol witness table for the contained type.
  /// Returns NULL if the type does not conform.
  const hashable_support::HashableWitnessTable *getHashableConformance() const;

  /// Get the `Equatable` protocol witness table for the contained type.
  /// Returns NULL if the type does not conform.
  const equatable_support::EquatableWitnessTable *getEquatableConformance() const;

  /// Populate the `cachedConformance` with the Hashable conformance
  /// (if there is one), else the Equatable conformance.
  /// Returns the encoded conformance:  least-significant
  /// bit is set if this is an Equatable conformance,
  /// else it is a Hashable conformance.  0 (or 1) indicates
  /// neither was found.
  uintptr_t cacheHashableEquatableConformance() const;


  CodiraValueHeader()
      : cachedBaseType(0), cachedConformance(0) {}
};

// Set cachedConformance to the Hashable conformance if
// there is one, else the Equatable conformance.
// Also set cachedBaseType to the parent type that
// introduced the Hashable/Equatable conformance.
// The cached conformance and type are encoded:
// * If the LSbit is not set, it's the Hashable conformance
// * If the value is exactly 1, neither conformance is present
// * If the LSbit is 1, strip it and you'll have the Equatable conformance
// (Null indicates the cache has not been initialized yet;
// that will never be true on exit of this function.)
// Return: encoded cachedConformance value
uintptr_t
CodiraValueHeader::cacheHashableEquatableConformance() const {
    // Relevant conformance and baseType
    uintptr_t conformance;
    uintptr_t baseType;

    // First, see if it's Hashable
    const HashableWitnessTable *hashable =
      reinterpret_cast<const HashableWitnessTable *>(
	language_conformsToProtocolCommon(type, &HashableProtocolDescriptor));
    if (hashable != nullptr) {
      conformance = reinterpret_cast<uintptr_t>(hashable);
      baseType = reinterpret_cast<uintptr_t>(findHashableBaseType(type));
    } else {
      // If not Hashable, maybe Equatable?
      auto equatable = 
	language_conformsToProtocolCommon(type, &equatable_support::EquatableProtocolDescriptor);
      // Encode the equatable conformance
      conformance = reinterpret_cast<uintptr_t>(equatable) | 1;

      if (equatable != nullptr) {
	// Find equatable base type
#if LANGUAGE_STDLIB_USE_RELATIVE_PROTOCOL_WITNESS_TABLES
	const auto *description = lookThroughOptionalConditionalWitnessTable(
	  reinterpret_cast<const RelativeWitnessTable*>(equatable))
	  ->getDescription();
#else
	const auto *description = equatable->getDescription();
#endif
	const Metadata *baseTypeThatConformsToEquatable =
	  findConformingSuperclass(type, description);
	// Encode the equatable base type
	baseType = reinterpret_cast<uintptr_t>(baseTypeThatConformsToEquatable) | 1;
      } else {
	baseType = 1; // Neither equatable nor hashable
      }
    }

    // Set the conformance/baseType caches atomically
    uintptr_t expectedConformance = 0;
    cachedConformance.compare_exchange_strong(
      expectedConformance, conformance, std::memory_order_acq_rel);
    uintptr_t expectedType = 0;
    cachedBaseType.compare_exchange_strong(
      expectedType, baseType, std::memory_order_acq_rel);

    return conformance;
}

const Metadata *CodiraValueHeader::getHashableBaseType() const {
  auto type = cachedBaseType.load(std::memory_order_acquire);
  if (type == 0) {
    cacheHashableEquatableConformance();
    type = cachedBaseType.load(std::memory_order_acquire);
  }
  if ((type & 1) == 0) {
    // A Hashable conformance was found
    return reinterpret_cast<const Metadata *>(type);
  } else {
    // Equatable conformance (or no conformance) found
    return nullptr;
  }
}

const Metadata *CodiraValueHeader::getEquatableBaseType() const {
  auto type = cachedBaseType.load(std::memory_order_acquire);
  if (type == 0) {
    cacheHashableEquatableConformance();
    type = cachedBaseType.load(std::memory_order_acquire);
  }
  if ((type & 1) == 0) {
    // A Hashable conformance was found
    return nullptr;
  } else {
    // An Equatable conformance (or neither) was found
    return reinterpret_cast<const Metadata *>(type & ~1ULL);
  }
}

const hashable_support::HashableWitnessTable *
CodiraValueHeader::getHashableConformance() const {
  uintptr_t wt = cachedConformance.load(std::memory_order_acquire);
  if (wt == 0) {
    wt = cacheHashableEquatableConformance();
  }
  if ((wt & 1) == 0) {
    // Hashable conformance found
    return reinterpret_cast<const hashable_support::HashableWitnessTable *>(wt);
  } else {
    // Equatable conformance (or no conformance) found
    return nullptr;
  }
}

const equatable_support::EquatableWitnessTable *
CodiraValueHeader::getEquatableConformance() const {
  uintptr_t wt = cachedConformance.load(std::memory_order_acquire);
  if (wt == 0) {
    wt = cacheHashableEquatableConformance();
  }
  if ((wt & 1) == 0) {
    // Hashable conformance found
    return nullptr;
  } else {
    // Equatable conformance (or no conformance) found
    return reinterpret_cast<const equatable_support::EquatableWitnessTable *>(wt & ~1ULL);
  }
}

static constexpr const size_t CodiraValueHeaderOffset
  = sizeof(Class); // isa pointer
static constexpr const size_t CodiraValueMinAlignMask
  = alignof(Class) - 1;
/* TODO: If we're able to become a CodiraObject subclass in the future,
 * change to this:
static constexpr const size_t CodiraValueHeaderOffset
  = sizeof(HeapObject);
static constexpr const size_t CodiraValueMinAlignMask
  = alignof(HeapObject) - 1;
 */

static Class _getCodiraValueClass() {
  auto theClass = [__CodiraValue class];
  // Fixed instance size of __CodiraValue should be same as object header.
  assert(class_getInstanceSize(theClass) == CodiraValueHeaderOffset
         && "unexpected size of __CodiraValue?!");
  return theClass;
}

static Class getCodiraValueClass() {
  return LANGUAGE_LAZY_CONSTANT(_getCodiraValueClass());
}

static constexpr size_t getCodiraValuePayloadOffset(size_t alignMask) {
  return (CodiraValueHeaderOffset + sizeof(CodiraValueHeader) + alignMask) &
         ~alignMask;
}

static CodiraValueHeader *getCodiraValueHeader(__CodiraValue *v) {
  auto instanceBytes = reinterpret_cast<char *>(v);
  return reinterpret_cast<CodiraValueHeader *>(instanceBytes +
                                              CodiraValueHeaderOffset);
}

static OpaqueValue *getCodiraValuePayload(__CodiraValue *v, size_t alignMask) {
  auto instanceBytes = reinterpret_cast<char *>(v);
  return reinterpret_cast<OpaqueValue *>(instanceBytes +
                                         getCodiraValuePayloadOffset(alignMask));
}

static size_t getCodiraValuePayloadAlignMask(const Metadata *type) {
  return type->getValueWitnesses()->getAlignmentMask() | CodiraValueMinAlignMask;
}

const Metadata *language::getCodiraValueTypeMetadata(__CodiraValue *v) {
  return getCodiraValueHeader(v)->type;
}

std::pair<const Metadata *, const OpaqueValue *>
language::getValueFromCodiraValue(__CodiraValue *v) {
  auto instanceType = getCodiraValueTypeMetadata(v);
  size_t alignMask = getCodiraValuePayloadAlignMask(instanceType);
  return {instanceType, getCodiraValuePayload(v, alignMask)};
}

__CodiraValue *language::bridgeAnythingToCodiraValueObject(OpaqueValue *src,
                                                    const Metadata *srcType,
                                                    bool consume) {
  size_t alignMask = getCodiraValuePayloadAlignMask(srcType);

  size_t totalSize =
      getCodiraValuePayloadOffset(alignMask) + srcType->getValueWitnesses()->size;

  void *instanceMemory = language_slowAlloc(totalSize, alignMask);
  __CodiraValue *instance
    = objc_constructInstance(getCodiraValueClass(), instanceMemory);
  /* TODO: If we're able to become a CodiraObject subclass in the future,
   * change to this:
  auto instance = language_allocObject(getCodiraValueClass(), totalSize,
                                    alignMask);
   */

  auto header = getCodiraValueHeader(instance);
  ::new (header) CodiraValueHeader();
  header->type = srcType;

  auto payload = getCodiraValuePayload(instance, alignMask);
  if (consume)
    srcType->vw_initializeWithTake(payload, src);
  else
    srcType->vw_initializeWithCopy(payload, src);

  return instance;
}

__CodiraValue *language::getAsCodiraValue(id object) {
  // __CodiraValue should have no subclasses or proxies. We can do an exact
  // class check.
  if (object_getClass(object) == getCodiraValueClass())
    return object;
  return nil;
}

bool
language::findCodiraValueConformances(const ExistentialTypeMetadata *existentialType,
                                  const WitnessTable **tablesBuffer) {
  // __CodiraValue never satisfies a superclass constraint.
  if (existentialType->getSuperclassConstraint() != nullptr)
    return false;

  Class cls = nullptr;

  // Note that currently we never modify tablesBuffer because
  // __CodiraValue doesn't conform to any protocols that need witness tables.

  for (auto protocol : existentialType->getProtocols()) {
    // __CodiraValue only conforms to ObjC protocols.  We specifically
    // don't want to say that __CodiraValue conforms to the Codira protocols
    // that NSObject conforms to because that would create a situation
    // where arguably an arbitrary type would conform to those protocols
    // by way of coercion through __CodiraValue.  Eventually we want to
    // change __CodiraValue to not be an NSObject subclass at all.

    if (!protocol.isObjC())
      return false;

    if (!cls) cls = _getCodiraValueClass();

    // Check whether the class conforms to the protocol.
    if (![cls conformsToProtocol: protocol.getObjCProtocol()])
      return false;
  }

  return true;
}

@implementation __CodiraValue

+ (instancetype)allocWithZone:(NSZone *)zone {
  language::crash("__CodiraValue cannot be instantiated");
}

- (id)copyWithZone:(NSZone *)zone {
  // Instances are immutable, so we can just retain.
  return objc_retain(self);

  /* TODO: If we're able to become a CodiraObject subclass in the future,
   * change to this:
   language_retain((HeapObject*)self);
   return self;
   */
}

// Since we allocate using Codira's allocator to properly handle alignment,
// we need to deallocate ourselves instead of delegating to
// -[NSObject dealloc].
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-missing-super-calls"
- (void)dealloc {
  // TODO: If we're able to become a CodiraObject subclass in the future,
  // this should move to the heap metadata destructor function.

  // Destroy the contained value.
  auto instanceType = getCodiraValueTypeMetadata(self);
  size_t alignMask = getCodiraValuePayloadAlignMask(instanceType);
  getCodiraValueHeader(self)->~CodiraValueHeader();
  instanceType->vw_destroy(getCodiraValuePayload(self, alignMask));

  // Deallocate ourselves.
  objc_destructInstance(self);
  auto totalSize = getCodiraValuePayloadOffset(alignMask) +
                   instanceType->getValueWitnesses()->size;
  language_slowDealloc(self, totalSize, alignMask);
}
#pragma clang diagnostic pop

- (BOOL)isEqual:(id)other {
  if (self == other) {
    return YES;
  }

  if (!other) {
    return NO;
  }

  // `other` must also be a _CodiraValue box
  if (![other isKindOfClass:getCodiraValueClass()]) {
    return NO;
  }

  auto selfHeader = getCodiraValueHeader(self);
  auto otherHeader = getCodiraValueHeader(other);

  if (auto hashableConformance = selfHeader->getHashableConformance()) {
    if (auto selfHashableBaseType = selfHeader->getHashableBaseType()) {
      auto otherHashableBaseType = otherHeader->getHashableBaseType();
      if (selfHashableBaseType == otherHashableBaseType) {
        return _language_stdlib_Hashable_isEqual_indirect(
          getCodiraValuePayload(self,
                               getCodiraValuePayloadAlignMask(selfHeader->type)),
          getCodiraValuePayload(other,
                               getCodiraValuePayloadAlignMask(otherHeader->type)),
          selfHashableBaseType, hashableConformance);
      }
    }
  }

//  if (runtime::bincompat::useLegacyCodiraObjCHashing()) {
//    // Legacy behavior only proxies isEqual: for Hashable, not Equatable
//    return NO;
//  }

  if (auto equatableConformance = selfHeader->getEquatableConformance()) {
    if (auto selfEquatableBaseType = selfHeader->getEquatableBaseType()) {
      auto otherEquatableBaseType = otherHeader->getEquatableBaseType();
      if (selfEquatableBaseType == otherEquatableBaseType) {
        return _language_stdlib_Equatable_isEqual_indirect(
          getCodiraValuePayload(self,
                               getCodiraValuePayloadAlignMask(selfHeader->type)),
          getCodiraValuePayload(other,
                               getCodiraValuePayloadAlignMask(otherHeader->type)),
          selfEquatableBaseType, equatableConformance);
      }
    }
  }

  // Not Equatable, not Hashable, and not the same box
  return NO;
}

- (NSUInteger)hash {
  // If Codira type is Hashable, get the hash value from there
  auto selfHeader = getCodiraValueHeader(self);
  auto hashableConformance = selfHeader->getHashableConformance();
  if (hashableConformance) {
	  return _language_stdlib_Hashable_hashValue_indirect(
	    getCodiraValuePayload(self,
				 getCodiraValuePayloadAlignMask(selfHeader->type)),
	    selfHeader->type, hashableConformance);
  }

//  if (runtime::bincompat::useLegacyCodiraObjCHashing()) {
//    // Legacy behavior doesn't honor Equatable conformance, only Hashable
//    return (NSUInteger)self;
//  }

  // If Codira type is Equatable but not Hashable,
  // we have to return something here that is compatible
  // with the `isEqual:` above.
  auto equatableConformance = selfHeader->getEquatableConformance();
  if (equatableConformance) {
    // Warn once per type about this
    auto metadata = getCodiraValueTypeMetadata(self);
    static Lazy<std::unordered_set<const Metadata *>> warned;
    static LazyMutex warnedLock;
    LazyMutex::ScopedLock guard(warnedLock);
    auto result = warned.get().insert(metadata);
    auto inserted = std::get<1>(result);
    if (inserted) {
      TypeNamePair typeName = language_getTypeName(metadata, true);
      warning(0,
	      "Obj-C `-hash` invoked on a Codira value of type `%s` that is Equatable but not Hashable; "
	      "this can lead to severe performance problems.\n",
	      typeName.data);
    }
    // Constant value (yuck!) is the only choice here
    return (NSUInteger)1;
  }

  // If the Codira type is neither Equatable nor Hashable,
  // then we can hash the identity, which should be pretty
  // good in practice.
  return (NSUInteger)self;
}

static id getValueDescription(__CodiraValue *self) {
  const Metadata *type;
  const OpaqueValue *value;
  std::tie(type, value) = getValueFromCodiraValue(self);

  // Copy the value, since it will be consumed by getSummary.
  ValueBuffer copyBuf;
  auto copy = type->allocateBufferIn(&copyBuf);
  type->vw_initializeWithCopy(copy, const_cast<OpaqueValue *>(value));

  id string = getDescription(copy, type);
  type->deallocateBufferIn(&copyBuf);
  return string;
}

- (id /* NSString */)description {
  return getValueDescription(self);
}
- (id /* NSString */)debugDescription {
  return getValueDescription(self);
}

// Private methods for debugging purposes.

- (const Metadata *)_languageTypeMetadata {
  return getCodiraValueTypeMetadata(self);
}
- (id /* NSString */)_languageTypeName {
  TypeNamePair typeName
    = language_getTypeName(getCodiraValueTypeMetadata(self), true);
  id str = language_stdlib_NSStringFromUTF8(typeName.data, typeName.length);
  return [str autorelease];
}
- (const OpaqueValue *)_languageValue {
  return getValueFromCodiraValue(self).second;
}

@end
#endif

// TODO: We could pick specialized __CodiraValue subclasses for trivial types
// or for types with known size and alignment characteristics. Probably
// not enough of a real perf bottleneck to be worth it...
