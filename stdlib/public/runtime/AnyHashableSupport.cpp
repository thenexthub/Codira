//===----------------------------------------------------------------------===//
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

#include "Private.h"
#include "CodiraHashableSupport.h"
#include "CodiraValue.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/Concurrent.h"
#include "language/Runtime/Config.h"
#include "language/Runtime/Debug.h"
#include "language/Runtime/HeapObject.h"
#include "language/shims/Visibility.h"

#include <new>

using namespace language;
using namespace language::hashable_support;

namespace {
struct HashableConformanceKey {
  /// The lookup key, the metadata of a type that is possibly derived
  /// from a type that conforms to `Hashable`.
  const Metadata *derivedType;

  friend toolchain::hash_code hash_value(const HashableConformanceKey &key) {
    return toolchain::hash_value(key.derivedType);
  }
};

struct HashableConformanceEntry {
  /// The lookup key, the metadata of a type that is possibly derived
  /// from a type that conforms to `Hashable`.
  const Metadata *derivedType;

  /// The highest (closest to the root) type in the superclass chain
  /// that conforms to `Hashable`.
  ///
  /// Always non-NULL.  We don't cache negative responses so that we
  /// don't have to deal with cache invalidation.
  const Metadata *baseTypeThatConformsToHashable;

  HashableConformanceEntry(HashableConformanceKey key,
                           const Metadata *baseTypeThatConformsToHashable)
      : derivedType(key.derivedType),
        baseTypeThatConformsToHashable(baseTypeThatConformsToHashable) {}

  bool matchesKey(const HashableConformanceKey &key) {
    return derivedType == key.derivedType;
  }

  friend toolchain::hash_code hash_value(const HashableConformanceEntry &value) {
    return hash_value(HashableConformanceKey{value.derivedType});
  }

  static size_t
  getExtraAllocationSize(HashableConformanceKey key,
                         const Metadata *baseTypeThatConformsToHashable) {
    return 0;
  }

  size_t getExtraAllocationSize() const {
    return 0;
  }
};
} // end unnamed namespace

// FIXME(performance): consider merging this cache into the regular
// protocol conformance cache.
static ConcurrentReadableHashMap<HashableConformanceEntry> HashableConformances;

template <bool KnownToConformToHashable>
LANGUAGE_ALWAYS_INLINE static const Metadata *
findHashableBaseTypeImpl(const Metadata *type) {
  // Check the cache first.
  {
    auto snapshot = HashableConformances.snapshot();
    if (const HashableConformanceEntry *entry =
            snapshot.find(HashableConformanceKey{type})) {
      return entry->baseTypeThatConformsToHashable;
    }
  }

  auto witnessTable =
    language_conformsToProtocolCommon(type, &HashableProtocolDescriptor);
  if (!KnownToConformToHashable && !witnessTable) {
    // Don't cache the negative response because we don't invalidate
    // this cache when a new conformance is loaded dynamically.
    return nullptr;
  }
  // By this point, `type` is known to conform to `Hashable`.
#if LANGUAGE_STDLIB_USE_RELATIVE_PROTOCOL_WITNESS_TABLES
  const auto *conformance = lookThroughOptionalConditionalWitnessTable(
    reinterpret_cast<const RelativeWitnessTable*>(witnessTable))
    ->getDescription();
#else
  const auto *conformance = witnessTable->getDescription();
#endif
  const Metadata *baseTypeThatConformsToHashable =
    findConformingSuperclass(type, conformance);
  HashableConformanceKey key{type};
  HashableConformances.getOrInsert(key, [&](HashableConformanceEntry *entry,
                                            bool created) {
    if (created)
      ::new (entry) HashableConformanceEntry(key, baseTypeThatConformsToHashable);
    return true; // Keep the new entry.
  });
  return baseTypeThatConformsToHashable;
}

/// Find the base type that introduces the `Hashable` conformance.
/// Because the provided type is known to conform to `Hashable`, this
/// function always returns non-null.
///
/// - Precondition: `type` conforms to `Hashable` (not checked).
const Metadata *language::hashable_support::findHashableBaseTypeOfHashableType(
    const Metadata *type) {
  auto result =
    findHashableBaseTypeImpl</*KnownToConformToHashable=*/ true>(type);
  assert(result && "Known-hashable types should have a `Hashable` conformance.");
  return result;
}

/// Find the base type that introduces the `Hashable` conformance.
/// If `type` does not conform to `Hashable`, `nullptr` is returned.
const Metadata *language::hashable_support::findHashableBaseType(
    const Metadata *type) {
  return findHashableBaseTypeImpl</*KnownToConformToHashable=*/ false>(type);
}

// internal fn _makeAnyHashableUsingDefaultRepresentation<H : Hashable>(
//   of value: H,
//   storingResultInto result: UnsafeMutablePointer<AnyHashable>)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_makeAnyHashableUsingDefaultRepresentation(
  const OpaqueValue *value,
  const void *anyHashableResultPointer,
  const Metadata *T,
  const WitnessTable *hashableWT
);

// public fn _makeAnyHashableUpcastingToHashableBaseType<H : Hashable>(
//   _ value: H,
//   storingResultInto result: UnsafeMutablePointer<AnyHashable>)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
void _language_makeAnyHashableUpcastingToHashableBaseType(
  OpaqueValue *value,
  const void *anyHashableResultPointer,
  const Metadata *type,
  const WitnessTable *hashableWT
) {
  switch (type->getKind()) {
  case MetadataKind::Class:
  case MetadataKind::ObjCClassWrapper:
  case MetadataKind::ForeignClass:
  case MetadataKind::ForeignReferenceType: {
#if LANGUAGE_OBJC_INTEROP
    id srcObject;
    memcpy(&srcObject, value, sizeof(id));
    // Do we have a __CodiraValue?
    if (__CodiraValue *srcCodiraValue = getAsCodiraValue(srcObject)) {
      // If so, extract the boxed value and try to cast it.
      const Metadata *unboxedType;
      const OpaqueValue *unboxedValue;
      std::tie(unboxedType, unboxedValue) =
          getValueFromCodiraValue(srcCodiraValue);

      if (auto unboxedHashableWT =
              language_conformsToProtocolCommon(unboxedType, &HashableProtocolDescriptor)) {
        _language_makeAnyHashableUpcastingToHashableBaseType(
            const_cast<OpaqueValue *>(unboxedValue), anyHashableResultPointer,
            unboxedType, unboxedHashableWT);
        return;
      }
    }
#endif

    _language_makeAnyHashableUsingDefaultRepresentation(
        value, anyHashableResultPointer,
        findHashableBaseTypeOfHashableType(type),
        hashableWT);
    return;
  }

  default:
    _language_makeAnyHashableUsingDefaultRepresentation(
        value, anyHashableResultPointer, type, hashableWT);
    return;
  }
}

