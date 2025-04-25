//===--- SwiftValue.h - Boxed Swift value class -----------------*- C++ -*-===//
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
// This implements the Objective-C class that is used to carry Swift values
// that have been bridged to Objective-C objects without special handling.
// The class is opaque to user code, but is `NSObject`- and `NSCopying`-
// conforming and is understood by the Swift runtime for dynamic casting
// back to the contained type.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_SWIFTVALUE_H
#define SWIFT_RUNTIME_SWIFTVALUE_H

#if SWIFT_OBJC_INTEROP
#include <objc/runtime.h>
#include "language/Runtime/Metadata.h"

// __SwiftValue is an Objective-C class, but we shouldn't interface with it
// directly as such. Keep the type opaque.
#if __OBJC__
@class __SwiftValue;
#else
typedef struct __SwiftValue __SwiftValue;
#endif

namespace language {

/// Bridge a Swift value to an Objective-C object by boxing it as a __SwiftValue.
__SwiftValue *bridgeAnythingToSwiftValueObject(OpaqueValue *src,
                                              const Metadata *srcType,
                                              bool consume);

/// Get the type metadata for a value in a Swift box.
const Metadata *getSwiftValueTypeMetadata(__SwiftValue *v);

/// Get the value out of a Swift box along with its type metadata. The value
/// inside the box is immutable and must not be modified or taken from the box.
std::pair<const Metadata *, const OpaqueValue *>
getValueFromSwiftValue(__SwiftValue *v);

/// Return the object reference as a __SwiftValue* if it is a __SwiftValue instance,
/// or nil if it is not.
__SwiftValue *getAsSwiftValue(id object);

/// Find conformances for SwiftValue to the given existential type.
///
/// Returns true if SwiftValue does conform to all the protocols.
bool findSwiftValueConformances(const ExistentialTypeMetadata *existentialType,
                                const WitnessTable **tablesBuffer);

} // namespace language
#endif

#endif
