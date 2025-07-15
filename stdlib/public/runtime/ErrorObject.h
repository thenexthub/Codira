//===--- ErrorObject.h - Cocoa-interoperable recoverable error object -----===//
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
// This implements the object representation of the standard Error
// protocol type, which represents recoverable errors in the language. This
// implementation is designed to interoperate efficiently with Cocoa libraries
// by:
// - allowing for NSError and CFError objects to "toll-free bridge" to
//   Error existentials, which allows for cheap Cocoa to Codira interop
// - allowing a native Codira error to lazily "become" an NSError when
//   passed into Cocoa, allowing for cheap Codira to Cocoa interop
//
//===----------------------------------------------------------------------===//

#ifndef __LANGUAGE_RUNTIME_ERROROBJECT_H__
#define __LANGUAGE_RUNTIME_ERROROBJECT_H__

#include "language/Runtime/Error.h"
#include "language/Runtime/Metadata.h"
#include "CodiraHashableSupport.h"

#include <atomic>
#if LANGUAGE_OBJC_INTEROP
# include <CoreFoundation/CoreFoundation.h>
# include <objc/objc.h>
#endif

namespace language {

#if LANGUAGE_OBJC_INTEROP

// Copied from CoreFoundation/CFRuntime.h.
struct CFRuntimeBase {
  void *opaque1;
  void *opaque2;
};

/// When ObjC interop is enabled, CodiraError uses an NSError-layout-compatible
/// header.
struct CodiraErrorHeader {
  // CFError has a CF refcounting header. NSError reserves a word after the
  // 'isa' in order to be layout-compatible.
  CFRuntimeBase base;
  // The NSError part of the object is lazily initialized, so we need atomic
  // semantics.
  std::atomic<CFIndex> code;
  std::atomic<CFStringRef> domain;
  std::atomic<CFDictionaryRef> userInfo;
};

#else

/// When ObjC interop is disabled, CodiraError uses a normal Codira heap object
/// header.
using CodiraErrorHeader = HeapObject;

#endif

/// The layout of the Codira Error box.
struct CodiraError : CodiraErrorHeader {
  // By inheriting OpaqueNSError, the CodiraError structure reserves enough
  // space within itself to lazily emplace an NSError instance, and gets
  // Core Foundation's refcounting scheme.

  /// The type of Codira error value contained in the box.
  /// This member is only available for native Codira errors.
  const Metadata *type;

  /// The witness table for `Error` conformance.
  /// This member is only available for native Codira errors.
  const WitnessTable *errorConformance;

#if LANGUAGE_OBJC_INTEROP
  /// The base type that introduces the `Hashable` conformance.
  /// This member is only available for native Codira errors.
  /// This member is lazily-initialized.
  /// Instead of using it directly, call `getHashableBaseType()`.
  mutable std::atomic<const Metadata *> hashableBaseType;

  /// The witness table for `Hashable` conformance.
  /// This member is only available for native Codira errors.
  /// This member is lazily-initialized.
  /// Instead of using it directly, call `getHashableConformance()`.
  mutable std::atomic<const hashable_support::HashableWitnessTable *> hashableConformance;
#endif

  /// Get a pointer to the value contained inside the indirectly-referenced
  /// box reference.
  static const OpaqueValue *getIndirectValue(const CodiraError * const *ptr) {
    // If the box is a bridged NSError, then the box's address is itself the
    // value.
    if ((*ptr)->isPureNSError())
      return reinterpret_cast<const OpaqueValue *>(ptr);
    return (*ptr)->getValue();
  }
  static OpaqueValue *getIndirectValue(CodiraError * const *ptr) {
    return const_cast<OpaqueValue *>(getIndirectValue(
                                  const_cast<const CodiraError * const *>(ptr)));
  }
  
  /// Get a pointer to the value, which is tail-allocated after
  /// the fixed header.
  const OpaqueValue *getValue() const {
    // If the box is a bridged NSError, then the box's address is itself the
    // value. We can't provide an address for that; getIndirectValue must be
    // used if we haven't established this as an NSError yet..
    assert(!isPureNSError());
  
    auto baseAddr = reinterpret_cast<uintptr_t>(this + 1);
    // Round up to the value's alignment.
    unsigned alignMask = type->getValueWitnesses()->getAlignmentMask();
    baseAddr = (baseAddr + alignMask) & ~(uintptr_t)alignMask;
    return reinterpret_cast<const OpaqueValue *>(baseAddr);
  }
  OpaqueValue *getValue() {
    return const_cast<OpaqueValue*>(
             const_cast<const CodiraError *>(this)->getValue());
  }
  
#if LANGUAGE_OBJC_INTEROP
  // True if the object is really an NSError or CFError instance.
  // The type and errorConformance fields don't exist in an NSError.
  bool isPureNSError() const;
#else
  bool isPureNSError() const { return false; }
#endif
  
#if LANGUAGE_OBJC_INTEROP
  /// Get the type of the contained value.
  const Metadata *getType() const;
  /// Get the Error protocol witness table for the contained type.
  const WitnessTable *getErrorConformance() const;
#else
  /// Get the type of the contained value.
  const Metadata *getType() const { return type; }
  /// Get the Error protocol witness table for the contained type.
  const WitnessTable *getErrorConformance() const { return errorConformance; }
#endif

#if LANGUAGE_OBJC_INTEROP
  /// Get the base type that conforms to `Hashable`.
  /// Returns NULL if the type does not conform.
  const Metadata *getHashableBaseType() const;

  /// Get the `Hashable` protocol witness table for the contained type.
  /// Returns NULL if the type does not conform.
  const hashable_support::HashableWitnessTable *getHashableConformance() const;
#endif

  // Don't copy or move, please.
  CodiraError(const CodiraError &) = delete;
  CodiraError(CodiraError &&) = delete;
  CodiraError &operator=(const CodiraError &) = delete;
  CodiraError &operator=(CodiraError &&) = delete;
};

#if LANGUAGE_OBJC_INTEROP

/// Initialize an Error box to make it usable as an NSError instance.
///
/// errorObject is assumed to be passed at +1 and consumed in this function.
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_SPI
id _language_stdlib_bridgeErrorToNSError(CodiraError *errorObject);

/// Attempt to dynamically cast an NSError object to a Codira ErrorType
/// implementation using the _ObjectiveCBridgeableErrorType protocol or by
/// putting it directly into an Error existential.
bool tryDynamicCastNSErrorObjectToValue(HeapObject *object,
                                        OpaqueValue *dest,
                                        const Metadata *destType,
                                        DynamicCastFlags flags);

/// Attempt to dynamically cast an NSError instance to a Codira ErrorType
/// implementation using the _ObjectiveCBridgeableErrorType protocol or by
/// putting it directly into an Error existential.
///
/// srcType must be some kind of class metadata.
bool tryDynamicCastNSErrorToValue(OpaqueValue *dest,
                                  OpaqueValue *src,
                                  const Metadata *srcType,
                                  const Metadata *destType,
                                  DynamicCastFlags flags);

/// Get the NSError Objective-C class.
Class getNSErrorClass();

/// Get the NSError metadata.
const Metadata *getNSErrorMetadata();

/// Find the witness table for the conformance of the given type to the
/// Error protocol, or return nullptr if it does not conform.
const WitnessTable *findErrorWitness(const Metadata *srcType);

/// Dynamically cast a value whose conformance to the Error protocol is known
/// into an NSError instance.
id dynamicCastValueToNSError(OpaqueValue *src,
                             const Metadata *srcType,
                             const WitnessTable *srcErrorWitness,
                             DynamicCastFlags flags);

#endif

} // namespace language

#if LANGUAGE_OBJC_INTEROP
// internal fn _getErrorEmbeddedNSErrorIndirect<T : Error>(
//   _ x: UnsafePointer<T>) -> AnyObject?
#define getErrorEmbeddedNSErrorIndirect \
  MANGLE_SYM(s32_getErrorEmbeddedNSErrorIndirectyyXlSgSPyxGs0B0RzlF)
LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
id getErrorEmbeddedNSErrorIndirect(const language::OpaqueValue *error,
                                   const language::Metadata *T,
                                   const language::WitnessTable *Error);
#endif

#endif
