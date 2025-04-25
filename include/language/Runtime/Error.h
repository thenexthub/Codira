//===--- Error.h - Swift Runtime ABI for error values -----------*- C++ -*-===//
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
// Swift runtime support for working with error values.
//
// The ABI here is quite different in ObjC and non-ObjC modes.
// In ObjC mode, SwiftError is closely related to the NSError class:
// native errors are boxed as a subclass of NSError, but non-native
// errors may simply be NSError objects directly from Objective-C.
// In non-ObjC mode, SwiftError boxes are always native.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_ERROR_H
#define SWIFT_RUNTIME_ERROR_H

#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"

namespace language {

struct SwiftError;

/// Allocate a catchable error object.
///
/// If value is nonnull, it should point to a value of \c type, which will be
/// copied (or taken if \c isTake is true) into the newly-allocated error box.
/// If value is null, the box's contents will be left uninitialized, and
/// \c isTake should be false.
SWIFT_CC(swift) SWIFT_RUNTIME_STDLIB_API
BoxPair swift_allocError(const Metadata *type,
                         const WitnessTable *errorConformance,
                         OpaqueValue *value, bool isTake);

/// Deallocate an error object whose contained object has already been
/// destroyed.
SWIFT_RUNTIME_STDLIB_API
void swift_deallocError(SwiftError *error, const Metadata *type);

struct ErrorValueResult {
  const OpaqueValue *value;
  const Metadata *type;
  const WitnessTable *errorConformance;
};

/// Extract a pointer to the value, the type metadata, and the Error
/// protocol witness from an error object.
///
/// The "scratch" pointer should point to an uninitialized word-sized
/// temporary buffer. The implementation may write a reference to itself to
/// that buffer if the error object is a toll-free-bridged NSError instead of
/// a native Swift error, in which case the object itself is the "boxed" value.
SWIFT_RUNTIME_STDLIB_API
void swift_getErrorValue(const SwiftError *errorObject,
                         void **scratch,
                         ErrorValueResult *out);

/// Called when throwing an error.  Serves as a breakpoint hook
/// for debuggers.
SWIFT_CC(swift)
SWIFT_RUNTIME_STDLIB_API void
swift_willThrow(SWIFT_CONTEXT void *unused,
                SWIFT_ERROR_RESULT SwiftError **object);

/// Called when throwing a typed error.  Serves as a breakpoint hook
/// for debuggers.
SWIFT_CC(swift)
SWIFT_RUNTIME_STDLIB_API void
swift_willThrowTypedImpl(OpaqueValue *value,
                         const Metadata *type,
                         const WitnessTable *errorConformance);

/// Called when an error is thrown out of the top level of a script.
SWIFT_CC(swift)
SWIFT_RUNTIME_STDLIB_API SWIFT_NORETURN void
swift_errorInMain(SwiftError *object);

/// Called when the try! operator fails.
SWIFT_CC(swift)
SWIFT_RUNTIME_STDLIB_API SWIFT_NORETURN void
swift_unexpectedError(SwiftError *object, OpaqueValue *filenameStart,
                      long filenameLength, bool isAscii, unsigned long line);

/// Retain an error box.
SWIFT_RUNTIME_STDLIB_API
SwiftError *swift_errorRetain(SwiftError *object);

/// Release an error box.
SWIFT_RUNTIME_STDLIB_API
void swift_errorRelease(SwiftError *object);

} // end namespace language

#endif
