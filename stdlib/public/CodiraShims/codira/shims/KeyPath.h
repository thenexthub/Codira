//===--- KeyPath.h - ABI constants for key path objects ---------*- C++ -*-===//
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
//  Constants used in the layout of key path objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_KEYPATH_H
#define LANGUAGE_STDLIB_SHIMS_KEYPATH_H

#include "CodiraStdint.h"

#ifdef __cplusplus
namespace language {
extern "C" {
#endif

// Bitfields for the key path buffer header.

static const __language_uint32_t _CodiraKeyPathBufferHeader_SizeMask
  = 0x00FFFFFFU;
static const __language_uint32_t _CodiraKeyPathBufferHeader_TrivialFlag
  = 0x80000000U;
static const __language_uint32_t _CodiraKeyPathBufferHeader_HasReferencePrefixFlag
  = 0x40000000U;
static const __language_uint32_t _CodiraKeyPathBufferHeader_IsSingleComponentFlag
  = 0x20000000U;
static const __language_uint32_t _CodiraKeyPathBufferHeader_ReservedMask
  = 0x1F000000U;
  
// Bitfields for a key path component header.

static const __language_uint32_t _CodiraKeyPathComponentHeader_PayloadMask
  = 0x00FFFFFFU;

static const __language_uint32_t _CodiraKeyPathComponentHeader_DiscriminatorMask
  = 0x7F000000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_DiscriminatorShift
  = 24;

static const __language_uint32_t _CodiraKeyPathComponentHeader_StructTag
  = 1;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedTag
  = 2;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ClassTag
  = 3;
static const __language_uint32_t _CodiraKeyPathComponentHeader_OptionalTag
  = 4;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ExternalTag
  = 0;

static const __language_uint32_t
_CodiraKeyPathComponentHeader_TrivialPropertyDescriptorMarker = 0U;

static const __language_uint32_t _CodiraKeyPathComponentHeader_StoredOffsetPayloadMask
  = 0x007FFFFFU;

static const __language_uint32_t _CodiraKeyPathComponentHeader_MaximumOffsetPayload
  = 0x007FFFFCU;
  
static const __language_uint32_t _CodiraKeyPathComponentHeader_UnresolvedIndirectOffsetPayload
  = 0x007FFFFDU;
  
static const __language_uint32_t _CodiraKeyPathComponentHeader_UnresolvedFieldOffsetPayload
  = 0x007FFFFEU;

static const __language_uint32_t _CodiraKeyPathComponentHeader_OutOfLineOffsetPayload
  = 0x007FFFFFU;
  
static const __language_uint32_t _CodiraKeyPathComponentHeader_StoredMutableFlag
  = 0x00800000U;

static const __language_uint32_t _CodiraKeyPathComponentHeader_OptionalChainPayload
  = 0;
static const __language_uint32_t _CodiraKeyPathComponentHeader_OptionalWrapPayload
  = 1;
static const __language_uint32_t _CodiraKeyPathComponentHeader_OptionalForcePayload
  = 2;

static const __language_uint32_t _CodiraKeyPathComponentHeader_EndOfReferencePrefixFlag
  = 0x80000000U;
  
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedMutatingFlag
  = 0x00800000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedSettableFlag
  = 0x00400000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDByStoredPropertyFlag
  = 0x00200000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDByVTableOffsetFlag
  = 0x00100000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedHasArgumentsFlag
  = 0x00080000U;
// Not ABI, used internally by key path runtime implementation
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedInstantiatedFromExternalWithArgumentsFlag
  = 0x00000010U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDResolutionMask
  = 0x0000000FU;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDResolved
  = 0x00000000U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDResolvedAbsolute
  = 0x00000003U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDUnresolvedIndirectPointer
  = 0x00000002U;
static const __language_uint32_t _CodiraKeyPathComponentHeader_ComputedIDUnresolvedFunctionCall
  = 0x00000001U;

#ifndef __cplusplus
extern const void *_Nonnull (language_keyPathGenericWitnessTable[]);

static inline const void *_Nonnull __language_keyPathGenericWitnessTable_addr(void) {
  return language_keyPathGenericWitnessTable;
}
#endif

// Discriminators for pointer authentication in key path patterns and objects

static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentDestroy = 0x7072;
static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentCopy = 0x6f66;
static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentEquals = 0x756e;
static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentHash = 0x6374;
static const __language_uint16_t _CodiraKeyPath_ptrauth_Getter = 0x6f72;
static const __language_uint16_t _CodiraKeyPath_ptrauth_NonmutatingSetter = 0x6f70;
static const __language_uint16_t _CodiraKeyPath_ptrauth_MutatingSetter = 0x7469;
static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentLayout = 0x6373;
static const __language_uint16_t _CodiraKeyPath_ptrauth_ArgumentInit = 0x6275;
static const __language_uint16_t _CodiraKeyPath_ptrauth_MetadataAccessor = 0x7474;

#ifdef __cplusplus
} // extern "C"
} // namespace language
#endif

#endif // LANGUAGE_STDLIB_SHIMS_KEYPATH_H
