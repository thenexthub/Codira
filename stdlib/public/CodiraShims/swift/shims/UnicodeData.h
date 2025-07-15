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

#ifndef LANGUAGE_STDLIB_SHIMS_UNICODEDATA_H
#define LANGUAGE_STDLIB_SHIMS_UNICODEDATA_H

#include "CodiraStdbool.h"
#include "CodiraStdint.h"
#include "Visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LANGUAGE_STDLIB_LARGEST_NAME_COUNT 88

//===----------------------------------------------------------------------===//
// Utilities
//===----------------------------------------------------------------------===//

__language_intptr_t _language_stdlib_getMphIdx(__language_uint32_t scalar,
                                         __language_intptr_t levels,
                                         const __language_uint64_t * const *keys,
                                         const __language_uint16_t * const *ranks,
                                         const __language_uint16_t * const sizes);

__language_intptr_t _language_stdlib_getScalarBitArrayIdx(__language_uint32_t scalar,
                                              const __language_uint64_t *bitArrays,
                                              const __language_uint16_t *ranks);

//===----------------------------------------------------------------------===//
// Normalization
//===----------------------------------------------------------------------===//

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint16_t _language_stdlib_getNormData(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t * const _language_stdlib_nfd_decompositions;

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint32_t _language_stdlib_getDecompositionEntry(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint32_t _language_stdlib_getComposition(__language_uint32_t x,
                                              __language_uint32_t y);

//===----------------------------------------------------------------------===//
// Grapheme Breaking
//===----------------------------------------------------------------------===//

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getGraphemeBreakProperty(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_bool _language_stdlib_isInCB_Consonant(__language_uint32_t scalar);

//===----------------------------------------------------------------------===//
// Word Breaking
//===----------------------------------------------------------------------===//

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getWordBreakProperty(__language_uint32_t scalar);

//===----------------------------------------------------------------------===//
// Unicode.Scalar.Properties
//===----------------------------------------------------------------------===//

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint64_t _language_stdlib_getBinaryProperties(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getNumericType(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
double _language_stdlib_getNumericValue(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const char *_language_stdlib_getNameAlias(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_int32_t _language_stdlib_getMapping(__language_uint32_t scalar,
                                         __language_uint8_t mapping);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t *_language_stdlib_getSpecialMapping(__language_uint32_t scalar,
                                                       __language_uint8_t mapping,
                                                       __language_intptr_t *length);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_intptr_t _language_stdlib_getScalarName(__language_uint32_t scalar,
                                             __language_uint8_t *buffer,
                                             __language_intptr_t capacity);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint16_t _language_stdlib_getAge(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getGeneralCategory(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getScript(__language_uint32_t scalar);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t *_language_stdlib_getScriptExtensions(
                                                        __language_uint32_t scalar,
                                                        __language_uint8_t *count);

LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_stdlib_getCaseMapping(__language_uint32_t scalar,
                                  __language_uint32_t *buffer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_UNICODEDATA_H
