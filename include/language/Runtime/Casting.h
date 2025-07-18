//===--- Casting.h - Codira type-casting runtime support ---------*- C++ -*-===//
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
// Codira runtime functions for casting values.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_CASTING_H
#define LANGUAGE_RUNTIME_CASTING_H

#include "language/Runtime/Metadata.h"

namespace language {

/// Perform a checked dynamic cast of a value to a target type.
///
/// \param dest A buffer into which to write the destination value.
/// In all cases, this will be left uninitialized if the cast fails.
///
/// \param src Pointer to the source value to cast.  This may be left
///   uninitialized after the operation, depending on the flags.
///
/// \param targetType The type to which we are casting.
///
/// \param srcType The static type of the source value.
///
/// \param flags Flags to control the operation.
///
/// \return true if the cast succeeded. Depending on the flags,
///   language_dynamicCast may fail rather than return false.
LANGUAGE_RUNTIME_EXPORT
bool
language_dynamicCast(OpaqueValue *dest, OpaqueValue *src,
                  const Metadata *srcType,
                  const Metadata *targetType,
                  DynamicCastFlags flags);

/// Checked dynamic cast to a Codira class type.
///
/// \param object The object to cast.
/// \param targetType The type to which we are casting, which is known to be
/// a Codira class type.
///
/// \returns the object if the cast succeeds, or null otherwise.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastClass(const void *object, const ClassMetadata *targetType);

/// Unconditional, checked dynamic cast to a Codira class type.
///
/// Aborts if the object isn't of the target type.
///
/// \param object The object to cast.
/// \param targetType The type to which we are casting, which is known to be
/// a Codira class type.
/// \param file The source filename from which to report failure. May be null.
/// \param line The source line from which to report failure.
/// \param column The source column from which to report failure.
///
/// \returns the object.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastClassUnconditional(const void *object,
                                    const ClassMetadata *targetType,
                                    const char *file, unsigned line, unsigned column);

#if LANGUAGE_OBJC_INTEROP
/// Checked Objective-C-style dynamic cast to a class type.
///
/// \param object The object to cast, or nil.
/// \param targetType The type to which we are casting, which is known to be
/// a class type, but not necessarily valid type metadata.
///
/// \returns the object if the cast succeeds, or null otherwise.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastObjCClass(const void *object, const ClassMetadata *targetType);

/// Checked dynamic cast to a foreign class type.
///
/// \param object The object to cast, or nil.
/// \param targetType The type to which we are casting, which is known to be
/// a foreign class type.
///
/// \returns the object if the cast succeeds, or null otherwise.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastForeignClass(const void *object,
                              const ForeignClassMetadata *targetType);

/// Unconditional, checked, Objective-C-style dynamic cast to a class
/// type.
///
/// Aborts if the object isn't of the target type.
/// Note that unlike language_dynamicCastClassUnconditional, this does not abort
/// if the object is 'nil'.
///
/// \param object The object to cast, or nil.
/// \param targetType The type to which we are casting, which is known to be
/// a class type, but not necessarily valid type metadata.
/// \param file The source filename from which to report failure. May be null.
/// \param line The source line from which to report failure.
/// \param column The source column from which to report failure.
///
/// \returns the object.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastObjCClassUnconditional(const void *object,
                                        const ClassMetadata *targetType,
                                        const char *file, unsigned line, unsigned column);

/// Unconditional, checked dynamic cast to a foreign class type.
///
/// \param object The object to cast, or nil.
/// \param targetType The type to which we are casting, which is known to be
/// a foreign class type.
/// \param file The source filename from which to report failure. May be null.
/// \param line The source line from which to report failure.
/// \param column The source column from which to report failure.
///
/// \returns the object if the cast succeeds, or null otherwise.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastForeignClassUnconditional(
  const void *object,
  const ForeignClassMetadata *targetType,
  const char *file, unsigned line, unsigned column);
#endif

/// Checked dynamic cast of a class instance pointer to the given type.
///
/// \param object The class instance to cast.
///
/// \param targetType The type to which we are casting, which may be either a
/// class type or a wrapped Objective-C class type.
///
/// \returns the object, or null if it doesn't have the given target type.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastUnknownClass(const void *object, const Metadata *targetType);

/// Unconditional checked dynamic cast of a class instance pointer to
/// the given type.
///
/// Aborts if the object isn't of the target type.
///
/// \param object The class instance to cast.
///
/// \param targetType The type to which we are casting, which may be either a
/// class type or a wrapped Objective-C class type.
///
/// \param file The source filename from which to report failure. May be null.
/// \param line The source line from which to report failure.
/// \param column The source column from which to report failure.
///
/// \returns the object.
LANGUAGE_RUNTIME_EXPORT
const void *
language_dynamicCastUnknownClassUnconditional(const void *object,
                                           const Metadata *targetType,
                                           const char *file, unsigned line, unsigned column);

LANGUAGE_RUNTIME_EXPORT
const Metadata *
language_dynamicCastMetatype(const Metadata *sourceType,
                          const Metadata *targetType);
LANGUAGE_RUNTIME_EXPORT
const Metadata *
language_dynamicCastMetatypeUnconditional(const Metadata *sourceType,
                                       const Metadata *targetType,
                                       const char *file, unsigned line, unsigned column);
#if LANGUAGE_OBJC_INTEROP
LANGUAGE_RUNTIME_EXPORT
const ClassMetadata *
language_dynamicCastObjCClassMetatype(const ClassMetadata *sourceType,
                                   const ClassMetadata *targetType);
LANGUAGE_RUNTIME_EXPORT
const ClassMetadata *
language_dynamicCastObjCClassMetatypeUnconditional(const ClassMetadata *sourceType,
                                                const ClassMetadata *targetType,
                                                const char *file, unsigned line, unsigned column);
#endif

LANGUAGE_RUNTIME_EXPORT
const ClassMetadata *
language_dynamicCastForeignClassMetatype(const ClassMetadata *sourceType,
                                   const ClassMetadata *targetType);
LANGUAGE_RUNTIME_EXPORT
const ClassMetadata *
language_dynamicCastForeignClassMetatypeUnconditional(
  const ClassMetadata *sourceType,
  const ClassMetadata *targetType,
  const char *file, unsigned line, unsigned column);

/// Return the dynamic type of an opaque value.
///
/// \param value An opaque value.
/// \param self  The static type metadata for the opaque value and the result
///              type value.
/// \param existentialMetatype Whether the result type value is an existential
///                            metatype. If `self` is an existential type,
///                            then a `false` value indicates that the result
///                            is of concrete metatype type `self.Protocol`,
///                            and existential containers will not be projected
///                            through. A `true` value indicates that the result
///                            is of existential metatype type `self.Type`,
///                            so existential containers can be projected
///                            through as long as a subtype relationship holds
///                            from `self` to the contained dynamic type.
LANGUAGE_RUNTIME_EXPORT
const Metadata *
language_getDynamicType(OpaqueValue *value, const Metadata *self,
                     bool existentialMetatype);

/// Fetch the type metadata associated with the formal dynamic
/// type of the given (possibly Objective-C) object.  The formal
/// dynamic type ignores dynamic subclasses such as those introduced
/// by KVO. See [NOTE: Dynamic-subclass-KVO]
///
/// The object pointer may be a tagged pointer, but cannot be null.
LANGUAGE_RUNTIME_EXPORT
const Metadata *language_getObjectType(HeapObject *object);

/// Check whether a type conforms to a given native Codira protocol,
/// visible from the named module.
///
/// If so, returns a pointer to the witness table for its conformance.
/// Returns void if the type does not conform to the protocol.
///
/// \param type The metadata for the type for which to do the conformance
///             check.
/// \param protocol The protocol descriptor for the protocol to check
///                 conformance for. This pointer does not have ptrauth applied.
LANGUAGE_RUNTIME_EXPORT
const WitnessTable *language_conformsToProtocol(const Metadata *type,
                                             const void *protocol);

/// Check whether a type conforms to a given native Codira protocol. Identical to
/// language_conformsToProtocol, except that the protocol parameter has a ptrauth
/// signature on ARM64e that is signed with a process independent key.
LANGUAGE_RUNTIME_EXPORT
const WitnessTable *
language_conformsToProtocol2(const Metadata *type,
                          const ProtocolDescriptor *protocol);

/// Check whether a type conforms to a given native Codira protocol. Identical to
/// language_conformsToProtocol, except that the protocol parameter has a ptrauth
/// signature on ARM64e that is signed with a process dependent key.
LANGUAGE_RUNTIME_EXPORT
const WitnessTable *
language_conformsToProtocolCommon(const Metadata *type,
                               const ProtocolDescriptor *protocol);

/// The size of the ConformanceExecutionContext structure.
LANGUAGE_RUNTIME_EXPORT
size_t language_ConformanceExecutionContextSize;

/// Check whether a type conforms to a given native Codira protocol. This
/// is similar to language_conformsToProtocolCommon, but allows the caller to
/// either capture the execution context (in *context).
LANGUAGE_RUNTIME_EXPORT
const WitnessTable *
language_conformsToProtocolWithExecutionContext(
    const Metadata *type,
    const ProtocolDescriptor *protocol,
    ConformanceExecutionContext *context);

/// Determine whether this function is being executed within the execution
/// context for a conformance. For example, if the conformance is
/// isolated to a given global actor, checks whether this code is running on
/// that global actor's executor.
///
/// The context should have been filled in by
/// language_conformsToProtocolWithExecutionContext.
LANGUAGE_RUNTIME_EXPORT
bool language_isInConformanceExecutionContext(
    const Metadata *type,
    const ConformanceExecutionContext *context);

} // end namespace language

#endif // LANGUAGE_RUNTIME_CASTING_H
