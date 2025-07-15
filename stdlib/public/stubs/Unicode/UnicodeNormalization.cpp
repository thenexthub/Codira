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

#if LANGUAGE_STDLIB_ENABLE_UNICODE_DATA

#if defined(__APPLE__)
#include "Apple/NormalizationData.h"
#else
#include "Common/NormalizationData.h"
#endif

#else
#include "language/Runtime/Debug.h"
#endif

#include "language/shims/UnicodeData.h"
#include <stdint.h>

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint16_t _language_stdlib_getNormData(__language_uint32_t scalar) {
  // Fast Path: ASCII and some latiny scalars are very basic and have no
  // normalization properties.
  if (scalar < 0xC0) {
    return 0;
  }
  
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto dataIdx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                                    _language_stdlib_normData,
                                                  _language_stdlib_normData_ranks);

  // If we don't have an index into the data indices, then this scalar has no
  // normalization information.
  if (dataIdx == INTPTR_MAX) {
    return 0;
  }

  auto scalarDataIdx = _language_stdlib_normData_data_indices[dataIdx];
  return _language_stdlib_normData_data[scalarDataIdx];
#endif
}

#if LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t * const _language_stdlib_nfd_decompositions = _language_stdlib_nfd_decomp;
#else
LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t * const _language_stdlib_nfd_decompositions = nullptr;
#endif

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint32_t _language_stdlib_getDecompositionEntry(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto levelCount = NFD_DECOMP_LEVEL_COUNT;
  __language_intptr_t decompIdx = _language_stdlib_getMphIdx(scalar, levelCount,
                                                  _language_stdlib_nfd_decomp_keys,
                                                  _language_stdlib_nfd_decomp_ranks,
                                                  _language_stdlib_nfd_decomp_sizes);

  return _language_stdlib_nfd_decomp_indices[decompIdx];
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint32_t _language_stdlib_getComposition(__language_uint32_t x,
                                            __language_uint32_t y) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto levelCount = NFC_COMP_LEVEL_COUNT;
  __language_intptr_t compIdx = _language_stdlib_getMphIdx(y, levelCount,
                                                  _language_stdlib_nfc_comp_keys,
                                                  _language_stdlib_nfc_comp_ranks,
                                                  _language_stdlib_nfc_comp_sizes);
  auto array = _language_stdlib_nfc_comp_indices[compIdx];

  // Ensure that the first element in this array is equal to our y scalar.
  auto realY = (array[0] << 11) >> 11;

  if (y != realY) {
    return UINT32_MAX;
  }

  auto count = array[0] >> 21;

  __language_uint32_t low = 1;
  __language_uint32_t high = count - 1;

  while (high >= low) {
    auto idx = low + (high - low) / 2;
  
    auto entry = array[idx];
  
    // Shift the range count out of the scalar.
    auto lower = (entry << 15) >> 15;
  
    bool isNegative = entry >> 31;
    auto rangeCount = (entry << 1) >> 18;
  
    if (isNegative) {
      rangeCount = -rangeCount;
    }
  
    auto composed = lower + rangeCount;
  
    if (x == lower) {
      return composed;
    }
  
    if (x > lower) {
      low = idx + 1;
      continue;
    }
  
    if (x < lower) {
      high = idx - 1;
      continue;
    }
  }

  // If we made it out here, then our scalar was not found in the composition
  // array.
  // Return the max here to indicate that we couldn't find one.
  return UINT32_MAX;
#endif
}
