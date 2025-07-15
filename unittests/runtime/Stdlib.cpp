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

#include "language/Runtime/Metadata.h"
#include "language/Demangling/ManglingMacros.h"

using namespace language;

// Placeholders for Codira functions that the C++ runtime references
// but that the test code does not link to.

// AnyHashable

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_makeAnyHashableUsingDefaultRepresentation(
  const OpaqueValue *value,
  const void *anyHashableResultPointer,
  const Metadata *T,
  const WitnessTable *hashableWT
) {
  abort();
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss11AnyHashableVMn[1] = {0};

// CodiraHashableSupport

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_stdlib_Hashable_isEqual_indirect(
  const void *lhsValue, const void *rhsValue, const Metadata *type,
  const void *wt) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
intptr_t _language_stdlib_Hashable_hashValue_indirect(
  const void *value, const Metadata *type, const void *wt) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_convertToAnyHashableIndirect(
  OpaqueValue *source, OpaqueValue *destination, const Metadata *sourceType,
  const void *sourceConformance) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_anyHashableDownCastConditionalIndirect(
  OpaqueValue *source, OpaqueValue *destination, const Metadata *targetType) {
  abort();
}

// CodiraEquatableSupport

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_stdlib_Equatable_isEqual_indirect(
  const void *lhsValue, const void *rhsValue, const Metadata *type,
  const void *wt) {
  abort();
}

// Casting

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_arrayDownCastIndirect(OpaqueValue *destination,
                                  OpaqueValue *source,
                                  const Metadata *sourceValueType,
                                  const Metadata *targetValueType) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_arrayDownCastConditionalIndirect(OpaqueValue *destination,
                                             OpaqueValue *source,
                                             const Metadata *sourceValueType,
                                             const Metadata *targetValueType) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_setDownCastIndirect(OpaqueValue *destination,
                                OpaqueValue *source,
                                const Metadata *sourceValueType,
                                const Metadata *targetValueType,
                                const void *sourceValueHashable,
                                const void *targetValueHashable) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_setDownCastConditionalIndirect(OpaqueValue *destination,
                                           OpaqueValue *source,
                                           const Metadata *sourceValueType,
                                           const Metadata *targetValueType,
                                           const void *sourceValueHashable,
                                           const void *targetValueHashable) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_dictionaryDownCastIndirect(OpaqueValue *destination,
                                       OpaqueValue *source,
                                       const Metadata *sourceKeyType,
                                       const Metadata *sourceValueType,
                                       const Metadata *targetKeyType,
                                       const Metadata *targetValueType,
                                       const void *sourceKeyHashable,
                                       const void *targetKeyHashable) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool _language_dictionaryDownCastConditionalIndirect(OpaqueValue *destination,
                                        OpaqueValue *source,
                                        const Metadata *sourceKeyType,
                                        const Metadata *sourceValueType,
                                        const Metadata *targetKeyType,
                                        const Metadata *targetValueType,
                                        const void *sourceKeyHashable,
                                        const void *targetKeyHashable) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _bridgeNonVerbatimBoxedValue(const OpaqueValue *sourceValue,
                                  OpaqueValue *destValue,
                                  const Metadata *nativeType) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _bridgeNonVerbatimFromObjectiveCToAny(HeapObject *sourceValue,
                                           OpaqueValue *destValue) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool language_unboxFromCodiraValueWithType(OpaqueValue *source,
                                       OpaqueValue *result,
                                       const Metadata *destinationType) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
bool language_languageValueConformsTo(const Metadata *destinationType) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_API
HeapObject *$ss27_bridgeAnythingToObjectiveCyyXlxlF(OpaqueValue *src, const Metadata *srcType) {
  abort();
}

// ErrorObject

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
int $ss13_getErrorCodeySiSPyxGs0B0RzlF(void *) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void *$ss23_getErrorDomainNSStringyyXlSPyxGs0B0RzlF(void *) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void *$ss29_getErrorUserInfoNSDictionaryyyXlSgSPyxGs0B0RzlF(void *) {
  abort();
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
void *$ss32_getErrorEmbeddedNSErrorIndirectyyXlSgSPyxGs0B0RzlF(void *) {
  abort();
}

// Hashable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $SkMp[1] = {0};

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSHMp[1] = {0};

// Array

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSaMn[1] = {0};

// Dictionary

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ssSdVMn[1] = {0};

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSDMn[1] = {0};

// Set

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ssSeVMn[1] = {0};

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sShMn[1] = {0};

// Bool

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSbMn[1] = {0};

// Binary Floating Point

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSBMp[1] = {0};

// Double

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSdMn[1] = {0};

// RandomNumberGenerator

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSGMp[1] = {0};

// Int

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSiMn[1] = {0};

// DefaultIndices

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSIMn[1] = {0};

// Character

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSJMn[1] = {0};

// Numeric

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSjMp[1] = {0};

// RandomAccessCollection

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSkMp[1] = {0};

// BidirectionalCollection

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSKMp[1] = {0};

// RangeReplacementCollection

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSmMp[1] = {0};

// MutationCollection

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSMMp[1] = {0};

// Range

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSnMn[1] = {0};

// ClosedRange

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSNMn[1] = {0};

// ObjectIdentifier

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSOMn[1] = {0};

// UnsafeMutablePointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSpMn[1] = {0};

// Optional

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSqMn[1] = {0};

// Equatable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSQMp[1] = {0};

// UnsafeMutableBufferPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSrMn[1] = {0};

// UnsafeBufferPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSRMn[1] = {0};

// String

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSSMn[1] = {0};

// Sequence

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSTMp[1] = {0};

// UInt

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSuMn[1] = {0};

// UnsignedInteger

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSUMp[1] = {0};

// UnsafeMutableRawPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSvMn[1] = {0};

// Strideable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSxMp[1] = {0};

// RangeExpression

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSXMp[1] = {0};

// StringProtocol

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSyMp[1] = {0};

// RawRepresentable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSYMp[1] = {0};

// BinaryInteger

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSzMp[1] = {0};

// Decodable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSeMp[1] = {0};

// Encodable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSEMp[1] = {0};

// Float

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSfMn[1] = {0};

// FloatingPoint

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSFMp[1] = {0};

// Collection

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSlMp[1] = {0};

// Comparable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSLMp[1] = {0};

// UnsafePointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSPMn[1] = {0};

// Substring

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSsMn[1] = {0};

// IteratorProtocol

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sStMp[1] = {0};

// UnsafeRawPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSVMn[1] = {0};

// UnsafeMutableRawBufferPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSwMn[1] = {0};

// UnsafeRawBufferPointer

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSWMn[1] = {0};

// SignedInteger

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $sSZMp[1] = {0};

// Mirror

// protocol witness table for Codira._ClassSuperMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss17_ClassSuperMirrorVs01_C0sWP[1] = {0};

// type metadata accessor for Codira._ClassSuperMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss17_ClassSuperMirrorVMa[1] = {0};

// protocol witness table for Codira._MetatypeMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss15_MetatypeMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._MetatypeMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss15_MetatypeMirrorVMa[1] = {0};

// protocol witness table for Codira._EnumMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss11_EnumMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._EnumMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss11_EnumMirrorVMa[1] = {0};

// protocol witness table for Codira._OpaqueMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss13_OpaqueMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._OpaqueMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss13_OpaqueMirrorVMa[1] = {0};

// protocol witness table for Codira._StructMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss13_StructMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._StructMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss13_StructMirrorVMa[1] = {0};

// protocol witness table for Codira._TupleMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss12_TupleMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._TupleMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss12_TupleMirrorVMa[1] = {0};

// protocol witness table for Codira._ClassMirror : Codira._Mirror in Codira
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss12_ClassMirrorVs01_B0sWP[1] = {0};

// type metadata accessor for Codira._ClassMirror
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss12_ClassMirrorVMa[1] = {0};

// KeyPath

LANGUAGE_RUNTIME_STDLIB_SPI
const HeapObject *language_getKeyPathImpl(const void *p,
                                       const void *e,
                                       const void *a) {
  abort();
}

// playground print hook

struct language_closure {
  void *fptr;
  HeapObject *context;
};
LANGUAGE_RUNTIME_STDLIB_API LANGUAGE_CC(language) language_closure
MANGLE_SYM(s20_playgroundPrintHookySScSgvg)() {
  return {nullptr, nullptr};
}

// ObjectiveC Bridgeable

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss21_ObjectiveCBridgeableMp[1] = {0};

// Key Path

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss7KeyPathCMo[1] = {0};

// Boxing

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss12__CodiraValueCMn[1] = {0};

// Never and Error

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss5NeverOMn[1] = {0};

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const long long $ss5ErrorMp[1] = {0};
