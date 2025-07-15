//===--- RuntimeValueWitness.h                                         ---===//
// Codira Language Bytecode Layouts Runtime Implementation
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
// Implementations of runtime determined value witness functions
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BYTECODE_LAYOUTS_H
#define LANGUAGE_BYTECODE_LAYOUTS_H

#include "language/Runtime/Metadata.h"
#include <cstdint>
#include <vector>

namespace language {

enum class RefCountingKind : uint8_t {
  End = 0x00,
  Error = 0x01,
  NativeStrong = 0x02,
  NativeUnowned = 0x03,
  NativeWeak = 0x04,
  Unknown = 0x05,
  UnknownUnowned = 0x06,
  UnknownWeak = 0x07,
  Bridge = 0x08,
  Block = 0x09,
  ObjC = 0x0a,
  NativeCodiraObjC = 0x0b,

  Metatype = 0x0c,
  Generic = 0x0d,
  Existential = 0x0e,
  Resilient = 0x0f,

  SinglePayloadEnumSimple = 0x10,
  SinglePayloadEnumFN = 0x11,
  SinglePayloadEnumFNResolved = 0x12,
  SinglePayloadEnumGeneric = 0x13,

  MultiPayloadEnumFN = 0x14,
  MultiPayloadEnumFNResolved = 0x15,
  MultiPayloadEnumGeneric = 0x16,

  // Skip = 0x80,
  // We may use the MSB as flag that a count follows,
  // so all following values are reserved
  // Reserved: 0x81 - 0xFF
};

struct LayoutStringReader1 {
  const uint8_t *layoutStr;

  template <typename T>
  inline T readBytes() {
    T returnVal;
    memcpy(&returnVal, layoutStr, sizeof(T));
    layoutStr += sizeof(T);
    return returnVal;
  }

  template <typename... T>
  inline void readBytes(T&... result) {
    uintptr_t additionalOffset = 0;
    ([&] {
        memcpy(&result, layoutStr + additionalOffset, sizeof(T));
        additionalOffset += sizeof(T);
    }(), ...);
    layoutStr += additionalOffset;
  }

  template<typename T, typename F>
  inline T modify(F &&f) {
    LayoutStringReader1 readerCopy = *this;
    T res = f(readerCopy);
    layoutStr = readerCopy.layoutStr;
    return res;
  }

  template<typename F>
  inline void modify(F &&f) {
    LayoutStringReader1 readerCopy = *this;
    f(readerCopy);
    layoutStr = readerCopy.layoutStr;
  }

  template <typename T>
  inline T peekBytes(size_t peekOffset = 0) const {
    T returnVal;
    memcpy(&returnVal, layoutStr + peekOffset, sizeof(T));
    return returnVal;
  }

  inline void skip(size_t n) { layoutStr += n; }

  inline uintptr_t getAbsolute() {
    return (uintptr_t) layoutStr;
  }
};

struct LayoutStringReader {
  const uint8_t *layoutStr;
  size_t offset;

  template <typename T>
  inline T readBytes() {
    T returnVal;
    memcpy(&returnVal, layoutStr + offset, sizeof(T));
    offset += sizeof(T);
    return returnVal;
  }

  template <typename... T>
  inline void readBytes(T&... result) {
    uintptr_t additionalOffset = 0;
    ([&] {
        memcpy(&result, layoutStr + offset + additionalOffset, sizeof(T));
        additionalOffset += sizeof(T);
    }(), ...);
    offset += additionalOffset;
  }

  template<typename T, typename F>
  inline T modify(F &&f) {
    LayoutStringReader readerCopy = *this;
    T res = f(readerCopy);
    offset = readerCopy.offset;
    return res;
  }

  template<typename F>
  inline void modify(F &&f) {
    LayoutStringReader readerCopy = *this;
    f(readerCopy);
    offset = readerCopy.offset;
  }

  template <typename T>
  inline T peekBytes(size_t peekOffset = 0) const {
    T returnVal;
    memcpy(&returnVal, layoutStr + offset + peekOffset, sizeof(T));
    return returnVal;
  }

  inline void skip(size_t n) { offset += n; }

  inline uintptr_t getAbsolute() {
    return (uintptr_t) layoutStr + offset;
  }
};

struct LayoutStringWriter {
  uint8_t *layoutStr;
  size_t offset;

  template <typename T>
  inline void writeBytes(T value) {
    memcpy(layoutStr + offset, &value, sizeof(T));
    offset += sizeof(T);
  }

  inline void skip(size_t n) { offset += n; }
};

LANGUAGE_RUNTIME_EXPORT
void language_cvw_destroy(language::OpaqueValue *address, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_cvw_assignWithCopy(language::OpaqueValue *dest,
                                             language::OpaqueValue *src,
                                             const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_cvw_assignWithTake(language::OpaqueValue *dest,
                                             language::OpaqueValue *src,
                                             const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_cvw_initWithCopy(language::OpaqueValue *dest,
                                           language::OpaqueValue *src,
                                           const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_cvw_initWithTake(language::OpaqueValue *dest,
                                           language::OpaqueValue *src,
                                           const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_initializeBufferWithCopyOfBuffer(language::ValueBuffer *dest,
                                           language::ValueBuffer *src,
                                           const Metadata *metadata);

LANGUAGE_RUNTIME_EXPORT
void language_cvw_destroyMultiPayloadEnumFN(language::OpaqueValue *address,
                                         const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_assignWithCopyMultiPayloadEnumFN(language::OpaqueValue *dest,
                                           language::OpaqueValue *src,
                                           const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_assignWithTakeMultiPayloadEnumFN(language::OpaqueValue *dest,
                                           language::OpaqueValue *src,
                                           const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_initWithCopyMultiPayloadEnumFN(language::OpaqueValue *dest,
                                         language::OpaqueValue *src,
                                         const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_initWithTakeMultiPayloadEnumFN(language::OpaqueValue *dest,
                                         language::OpaqueValue *src,
                                         const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_cvw_initializeBufferWithCopyOfBufferMultiPayloadEnumFN(
    language::ValueBuffer *dest, language::ValueBuffer *src,
    const Metadata *metadata);

LANGUAGE_RUNTIME_EXPORT
unsigned language_cvw_singletonEnum_getEnumTag(language::OpaqueValue *address,
                                            const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_cvw_singletonEnum_destructiveInjectEnumTag(
    language::OpaqueValue *address, unsigned tag, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_cvw_enumSimple_getEnumTag(language::OpaqueValue *address,
                                         const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_cvw_enumSimple_destructiveInjectEnumTag(language::OpaqueValue *address,
                                                   unsigned tag,
                                                   const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_cvw_enumFn_getEnumTag(language::OpaqueValue *address,
                                     const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned
language_cvw_multiPayloadEnumGeneric_getEnumTag(language::OpaqueValue *address,
                                             const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_cvw_multiPayloadEnumGeneric_destructiveInjectEnumTag(
    language::OpaqueValue *address, unsigned tag, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned
language_cvw_singlePayloadEnumGeneric_getEnumTag(language::OpaqueValue *address,
                                              const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_cvw_singlePayloadEnumGeneric_destructiveInjectEnumTag(
    language::OpaqueValue *address, unsigned tag, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_cvw_instantiateLayoutString(const uint8_t *layoutStr,
                                       Metadata *type);

void language_cvw_resolve_resilientAccessors(uint8_t *layoutStr,
                                          size_t layoutStrOffset,
                                          const uint8_t *fieldLayoutStr,
                                          const Metadata *fieldType);

void language_cvw_arrayDestroy(language::OpaqueValue *addr, size_t count,
                            size_t stride, const Metadata *metadata);

void language_cvw_arrayInitWithCopy(language::OpaqueValue *dest,
                                 language::OpaqueValue *src, size_t count,
                                 size_t stride, const Metadata *metadata);

extern "C" void language_cvw_arrayAssignWithCopy(language::OpaqueValue *dest,
                                              language::OpaqueValue *src,
                                              size_t count, size_t stride,
                                              const Metadata *metadata);

// For backwards compatibility
LANGUAGE_RUNTIME_EXPORT
void language_generic_destroy(language::OpaqueValue *address,
                           const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_generic_assignWithCopy(language::OpaqueValue *dest,
                                                 language::OpaqueValue *src,
                                                 const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_generic_assignWithTake(language::OpaqueValue *dest,
                                                 language::OpaqueValue *src,
                                                 const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_generic_initWithCopy(language::OpaqueValue *dest,
                                               language::OpaqueValue *src,
                                               const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *language_generic_initWithTake(language::OpaqueValue *dest,
                                               language::OpaqueValue *src,
                                               const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
language::OpaqueValue *
language_generic_initializeBufferWithCopyOfBuffer(language::ValueBuffer *dest,
                                               language::ValueBuffer *src,
                                               const Metadata *metadata);

LANGUAGE_RUNTIME_EXPORT
unsigned language_singletonEnum_getEnumTag(language::OpaqueValue *address,
                                        const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_singletonEnum_destructiveInjectEnumTag(language::OpaqueValue *address,
                                                  unsigned tag,
                                                  const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_enumSimple_getEnumTag(language::OpaqueValue *address,
                                     const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_enumSimple_destructiveInjectEnumTag(language::OpaqueValue *address,
                                               unsigned tag,
                                               const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_enumFn_getEnumTag(language::OpaqueValue *address,
                                 const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_multiPayloadEnumGeneric_getEnumTag(language::OpaqueValue *address,
                                                  const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_multiPayloadEnumGeneric_destructiveInjectEnumTag(
    language::OpaqueValue *address, unsigned tag, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
unsigned language_singlePayloadEnumGeneric_getEnumTag(language::OpaqueValue *address,
                                                   const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_singlePayloadEnumGeneric_destructiveInjectEnumTag(
    language::OpaqueValue *address, unsigned tag, const Metadata *metadata);
LANGUAGE_RUNTIME_EXPORT
void language_generic_instantiateLayoutString(const uint8_t *layoutStr, Metadata *type);

constexpr size_t layoutStringHeaderSize = sizeof(uint64_t) + sizeof(size_t);

} // namespace language

#endif // LANGUAGE_BYTECODE_LAYOUTS_H
