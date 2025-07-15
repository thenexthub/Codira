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
#include "Common/GraphemeData.h"
#else
#include "language/Runtime/Debug.h"
#endif
#include "language/shims/UnicodeData.h"
#include <stdint.h>


LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getGraphemeBreakProperty(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto index = 1; //0th element is a dummy element
  while (index < GRAPHEME_BREAK_DATA_COUNT) {
    auto entry = _language_stdlib_graphemeBreakProperties[index];
    
    // Shift the enum and range count out of the value.
    auto lower = (entry << 11) >> 11;
    
    // Shift the enum out first, then shift out the scalar value.
    auto upper = lower + ((entry << 3) >> 24);
    
    // Shift everything out.
    auto enumValue = (__language_uint8_t)(entry >> 29);
    
    // Special case: extendedPictographic who used an extra bit for the range.
    if (enumValue == 5) {
      upper = lower + ((entry << 2) >> 23);
    }
    
    //If we want the left child of the current node in our virtual tree,
    //that's at index * 2, if we want the right child it's at (index * 2) + 1
    if (scalar < lower) {
      index = 2 * index;
    } else if (scalar <= upper) {
      return enumValue;
    } else {
      index = 2 * index + 1;
    }
  }
  // If we made it out here, then our scalar was not found in the grapheme
  // array (this occurs when a scalar doesn't map to any grapheme break
  // property). Return the max value here to indicate .any.
  return 0xFF;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_bool _language_stdlib_isInCB_Consonant(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto idx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                          _language_stdlib_InCB_Consonant,
                                          _language_stdlib_InCB_Consonant_ranks);

  if (idx == INTPTR_MAX) {
    return false;
  }

  return true;
#endif
}
