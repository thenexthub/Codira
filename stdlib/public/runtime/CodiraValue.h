//===--- CodiraValue.h - Boxed Codira value class -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_RUNTIME_LANGUAGEVALUE_H
#define LANGUAGE_RUNTIME_LANGUAGEVALUE_H

#if LANGUAGE_OBJC_INTEROP
#include <objc/runtime.h>
#include "language/Runtime/Metadata.h"

// __CodiraValue is an Objective-C class, but we shouldn't interface with it
// directly as such. Keep the type opaque.
#if __OBJC__
@class __CodiraValue;
#else
typedef struct __CodiraValue __CodiraValue;
#endif

namespace language {

/// Bridge a Codira value to an Objective-C object by boxing it as a __CodiraValue.
__CodiraValue *bridgeAnythingToCodiraValueObject(OpaqueValue *src,
                                              const Metadata *srcType,
                                              bool consume);

/// Get the type metadata for a value in a Codira box.
const Metadata *getCodiraValueTypeMetadata(__CodiraValue *v);

/// Get the value out of a Codira box along with its type metadata. The value
/// inside the box is immutable and must not be modified or taken from the box.
std::pair<const Metadata *, const OpaqueValue *>
getValueFromCodiraValue(__CodiraValue *v);

/// Return the object reference as a __CodiraValue* if it is a __CodiraValue instance,
/// or nil if it is not.
__CodiraValue *getAsCodiraValue(id object);

/// Find conformances for CodiraValue to the given existential type.
///
/// Returns true if CodiraValue does conform to all the protocols.
bool findCodiraValueConformances(const ExistentialTypeMetadata *existentialType,
                                const WitnessTable **tablesBuffer);

} // namespace language
#endif

#endif
