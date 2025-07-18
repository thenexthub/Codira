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
#include "Apple/ScalarPropsData.h"
#else
#include "Common/ScalarPropsData.h"
#endif

#include "Common/CaseData.h"
#include "Common/ScriptData.h"

#else
#include "language/Runtime/Debug.h"
#endif

#include "language/shims/UnicodeData.h"
#include <stdint.h>

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint64_t _language_stdlib_getBinaryProperties(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto lowerBoundIndex = 0;
  auto endIndex = BIN_PROPS_COUNT;
  auto upperBoundIndex = endIndex - 1;

  while (upperBoundIndex >= lowerBoundIndex) {
    auto index = lowerBoundIndex + (upperBoundIndex - lowerBoundIndex) / 2;

    auto entry = _language_stdlib_scalar_binProps[index];

    // Shift the ccc value out of the scalar.
    auto lowerBoundScalar = (entry << 11) >> 11;

    __language_uint32_t upperBoundScalar = 0;

    // If we're not at the end of the array, the range count is simply the
    // distance to the next element.
    if (index != endIndex - 1) {
      auto nextEntry = _language_stdlib_scalar_binProps[index + 1];

      auto nextLower = (nextEntry << 11) >> 11;

      upperBoundScalar = nextLower - 1;
    } else {
      // Otherwise, the range count is the distance to 0x10FFFF
      upperBoundScalar = 0x10FFFF;
    }

    // Shift everything out.
    auto dataIndex = entry >> 21;

    if (scalar >= lowerBoundScalar && scalar <= upperBoundScalar) {
      return  _language_stdlib_scalar_binProps_data[dataIndex];
    }

    if (scalar > upperBoundScalar) {
      lowerBoundIndex = index + 1;
      continue;
    }

    if (scalar < lowerBoundScalar) {
      upperBoundIndex = index - 1;
      continue;
    }
  }

  // If we make it out of this loop, then it means the scalar was not found at
  // all in the array. This should never happen because the array represents all
  // scalars from 0x0 to 0x10FFFF, but if somehow this branch gets reached,
  // return 0 to indicate no properties.
  return 0;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getNumericType(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto lowerBoundIndex = 0;
  auto endIndex = NUMERIC_TYPE_COUNT;
  auto upperBoundIndex = endIndex - 1;

  while (upperBoundIndex >= lowerBoundIndex) {
    auto idx = lowerBoundIndex + (upperBoundIndex - lowerBoundIndex) / 2;

    auto entry = _language_stdlib_numeric_type[idx];

    auto lowerBoundScalar = (entry << 11) >> 11;
    auto rangeCount = (entry << 3) >> 24;
    auto upperBoundScalar = lowerBoundScalar + rangeCount;

    auto numericType = (__language_uint8_t)(entry >> 29);

    if (scalar >= lowerBoundScalar && scalar <= upperBoundScalar) {
      return numericType;
    }

    if (scalar > upperBoundScalar) {
      lowerBoundIndex = idx + 1;
      continue;
    }

    if (scalar < lowerBoundScalar) {
      upperBoundIndex = idx - 1;
      continue;
    }
  }

  // If we made it out here, then our scalar was not found in the composition
  // array.
  // Return the max here to indicate that we couldn't find one.
  return UINT8_MAX;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
double _language_stdlib_getNumericValue(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto levelCount = NUMERIC_VALUES_LEVEL_COUNT;
  __language_intptr_t scalarIdx = _language_stdlib_getMphIdx(scalar, levelCount,
                                                  _language_stdlib_numeric_values_keys,
                                                  _language_stdlib_numeric_values_ranks,
                                                  _language_stdlib_numeric_values_sizes);

  auto valueIdx = _language_stdlib_numeric_values_indices[scalarIdx];
  return _language_stdlib_numeric_values[valueIdx];
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const char *_language_stdlib_getNameAlias(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto dataIdx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                                    _language_stdlib_nameAlias,
                                                  _language_stdlib_nameAlias_ranks);

  if (dataIdx == INTPTR_MAX) {
    return nullptr;
  }

  return _language_stdlib_nameAlias_data[dataIdx];
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_int32_t _language_stdlib_getMapping(__language_uint32_t scalar,
                                         __language_uint8_t mapping) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto dataIdx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                                    _language_stdlib_mappings,
                                                  _language_stdlib_mappings_ranks);

  if (dataIdx == INTPTR_MAX) {
    return 0;
  }

  auto mappings = _language_stdlib_mappings_data_indices[dataIdx];

  __language_uint8_t mappingIdx;

  switch (mapping) {
    // Uppercase
    case 0:
      mappingIdx = mappings & 0xFF;
      break;

    // Lowercase
    case 1:
      mappingIdx = (mappings & 0xFF00) >> 8;
      break;

    // Titlecase
    case 2:
      mappingIdx = (mappings & 0xFF0000) >> 16;
      break;

    // Unknown mapping
    default:
      return 0;
  }

  if (mappingIdx == 0xFF) {
    return 0;
  }

  return _language_stdlib_mappings_data[mappingIdx];
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t *_language_stdlib_getSpecialMapping(__language_uint32_t scalar,
                                                       __language_uint8_t mapping,
                                                     __language_intptr_t *length) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto dataIdx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                                 _language_stdlib_special_mappings,
                                          _language_stdlib_special_mappings_ranks);

  if (dataIdx == INTPTR_MAX) {
    return nullptr;
  }

  auto index = _language_stdlib_special_mappings_data_indices[dataIdx];

  auto uppercase = _language_stdlib_special_mappings_data + index;
  auto lowercase = uppercase + 1 + *uppercase;
  auto titlecase = lowercase + 1 + *lowercase;

  switch (mapping) {
    // Uppercase
    case 0:
      *length = *uppercase;
      return uppercase + 1;

    // Lowercase
    case 1:
      *length = *lowercase;
      return lowercase + 1;

    // Titlecase
    case 2:
      *length = *titlecase;
      return titlecase + 1;

    // Unknown mapping.
    default:
      return nullptr;
  }
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_intptr_t _language_stdlib_getScalarName(__language_uint32_t scalar,
                                             __language_uint8_t *buffer,
                                             __language_intptr_t capacity) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto setOffset = _language_stdlib_names_scalar_sets[scalar >> 7];

  if (setOffset == UINT16_MAX) {
    return 0;
  }

  auto scalarIndex = (setOffset << 7) + (scalar & ((1 << 7) - 1));
  auto scalarOffset = _language_stdlib_names_scalars[scalarIndex];

  // U+20 is the first scalar that Unicode defines a name for, so their offset
  // will the only valid 0.
  if (scalarOffset == 0 && scalar != 0x20) {
    return 0;
  }

  __language_uint32_t nextScalarOffset = 0;

  if (scalarIndex != NAMES_SCALARS_MAX_INDEX) {
    int i = 1;

    // Look for the next scalar who has a name and their position in the names
    // array. This tells us exactly how many bytes our name takes up.
    while (nextScalarOffset == 0) {
      nextScalarOffset = _language_stdlib_names_scalars[scalarIndex + i];
      i += 1;
    }
  } else {
    // This is the last element in the array which represents the last scalar
    // name that Unicode defines (excluding variation selectors).
    nextScalarOffset = NAMES_LAST_SCALAR_OFFSET;
  }

  auto nameSize = nextScalarOffset - scalarOffset;

  // The total number of initialized bytes in the name string.
  int c = 0;

  for (__language_uint32_t i = 0; i < nameSize; i += 1) {
    __language_uint16_t wordIndex = (__language_uint16_t) _language_stdlib_names[
      scalarOffset + i
    ];

    // If our word index is 0xFF, then it means our word index is larger than a
    // byte, so the next two bytes will compose the 16 bit index.
    if (wordIndex == 0xFF) {
      i += 1;
      auto firstPart = _language_stdlib_names[scalarOffset + i];
      wordIndex = firstPart;

      i += 1;
      auto secondPart = _language_stdlib_names[scalarOffset + i];
      wordIndex |= secondPart << 8;
    }

    auto wordOffset = _language_stdlib_word_indices[wordIndex];

    auto word = _language_stdlib_words + wordOffset;

    // The last character in a word has the 7th bit set.
    while (*word < 0x80) {
      if (c >= capacity) {
        return c;
      }

      buffer[c++] = *word++;
    }

    if (c >= capacity) {
      return c;
    }

    buffer[c++] = *word & 0x7F;

    if (c >= capacity) {
      return c;
    }

    buffer[c++] = ' ';
  }

  // Remove the trailing space.
  c -= 1;

  // The return value is the number of initialized bytes.
  return c;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint16_t _language_stdlib_getAge(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto lowerBoundIndex = 0;
  auto endIndex = AGE_COUNT;
  auto upperBoundIndex = endIndex - 1;

  while (upperBoundIndex >= lowerBoundIndex) {
    auto idx = lowerBoundIndex + (upperBoundIndex - lowerBoundIndex) / 2;

    auto entry = _language_stdlib_ages[idx];

    auto lowerBoundScalar = (entry << 43) >> 43;
    auto rangeCount = entry >> 32;
    auto upperBoundScalar = lowerBoundScalar + rangeCount;

    auto ageIdx = (__language_uint8_t)((entry << 32) >> 32 >> 21);

    if (scalar >= lowerBoundScalar && scalar <= upperBoundScalar) {
      return _language_stdlib_ages_data[ageIdx];
    }

    if (scalar > upperBoundScalar) {
      lowerBoundIndex = idx + 1;
      continue;
    }

    if (scalar < lowerBoundScalar) {
      upperBoundIndex = idx - 1;
      continue;
    }
  }

  // If we made it out here, then our scalar was not found in the composition
  // array.
  // Return the max here to indicate that we couldn't find one.
  return UINT16_MAX;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getGeneralCategory(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto lowerBoundIndex = 0;
  auto endIndex = GENERAL_CATEGORY_COUNT;
  auto upperBoundIndex = endIndex - 1;

  while (upperBoundIndex >= lowerBoundIndex) {
    auto idx = lowerBoundIndex + (upperBoundIndex - lowerBoundIndex) / 2;

    auto entry = _language_stdlib_generalCategory[idx];

    auto lowerBoundScalar = (entry << 43) >> 43;
    auto rangeCount = entry >> 32;
    auto upperBoundScalar = lowerBoundScalar + rangeCount;

    auto generalCategory = (__language_uint8_t)((entry << 32) >> 32 >> 21);

    if (scalar >= lowerBoundScalar && scalar <= upperBoundScalar) {
      return generalCategory;
    }

    if (scalar > upperBoundScalar) {
      lowerBoundIndex = idx + 1;
      continue;
    }

    if (scalar < lowerBoundScalar) {
      upperBoundIndex = idx - 1;
      continue;
    }
  }

  // If we made it out here, then our scalar was not found in the composition
  // array.
  // Return the max here to indicate that we couldn't find one.
  return UINT8_MAX;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
__language_uint8_t _language_stdlib_getScript(__language_uint32_t scalar) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto lowerBoundIndex = 0;
  auto endIndex = SCRIPTS_COUNT;
  auto upperBoundIndex = endIndex - 1;
  
  while (upperBoundIndex >= lowerBoundIndex) {
    auto index = lowerBoundIndex + (upperBoundIndex - lowerBoundIndex) / 2;
    
    auto entry = _language_stdlib_scripts[index];
    
    // Shift the enum value out of the scalar.
    auto lowerBoundScalar = (entry << 11) >> 11;
    
    __language_uint32_t upperBoundScalar = 0;
    
    // If we're not at the end of the array, the range count is simply the
    // distance to the next element.
    if (index != endIndex - 1) {
      auto nextEntry = _language_stdlib_scripts[index + 1];
      
      auto nextLower = (nextEntry << 11) >> 11;
      
      upperBoundScalar = nextLower - 1;
    } else {
      // Otherwise, the range count is the distance to 0x10FFFF
      upperBoundScalar = 0x10FFFF;
    }
    
    // Shift the scalar out and get the enum value.
    auto script = entry >> 21;
    
    if (scalar >= lowerBoundScalar && scalar <= upperBoundScalar) {
      return script;
    }
    
    if (scalar > upperBoundScalar) {
      lowerBoundIndex = index + 1;
      continue;
    }
    
    if (scalar < lowerBoundScalar) {
      upperBoundIndex = index - 1;
      continue;
    }
  }
  
  // If we make it out of this loop, then it means the scalar was not found at
  // all in the array. This should never happen because the array represents all
  // scalars from 0x0 to 0x10FFFF, but if somehow this branch gets reached,
  // return 255 to indicate a failure.
  return UINT8_MAX;
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
const __language_uint8_t *_language_stdlib_getScriptExtensions(__language_uint32_t scalar,
                                                       __language_uint8_t *count) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto dataIdx = _language_stdlib_getScalarBitArrayIdx(scalar,
                                                _language_stdlib_script_extensions,
                                         _language_stdlib_script_extensions_ranks);
  
  // If we don't have an index into the data indices, then this scalar has no
  // script extensions
  if (dataIdx == INTPTR_MAX) {
    return 0;
  }
  
  auto scalarDataIdx = _language_stdlib_script_extensions_data_indices[dataIdx];
  *count = scalarDataIdx >> 11;
  
  return _language_stdlib_script_extensions_data + (scalarDataIdx & 0x7FF);
#endif
}

LANGUAGE_RUNTIME_STDLIB_INTERNAL
void _language_stdlib_getCaseMapping(__language_uint32_t scalar,
                                  __language_uint32_t *buffer) {
#if !LANGUAGE_STDLIB_ENABLE_UNICODE_DATA
  language::language_abortDisabledUnicodeSupport();
#else
  auto mphIdx = _language_stdlib_getMphIdx(scalar, CASE_FOLD_LEVEL_COUNT,
                                        _language_stdlib_case_keys,
                                        _language_stdlib_case_ranks,
                                        _language_stdlib_case_sizes);
  
  auto caseValue = _language_stdlib_case[mphIdx];
  __language_uint32_t hashedScalar = (caseValue << 43) >> 43;
  
  // If our scalar is not the original one we hashed, then this scalar has no
  // case mapping. It maps to itself.
  if (scalar != hashedScalar) {
    buffer[0] = scalar;
    return;
  }
  
  // If the top bit is NOT set, then this scalar simply maps to another scalar.
  // We have stored the distance to said scalar in this value.
  if ((caseValue & ((__language_uint64_t)(0x1) << 63)) == 0) {
    auto distance = (__language_int32_t)((caseValue << 1) >> 22);
    auto mappedScalar = (__language_uint32_t)((__language_int32_t)(scalar) - distance);
    
    buffer[0] = mappedScalar;
    return;
  }
  
  // Our top bit WAS set which means this scalar maps to multiple scalars.
  // Lookup our mapping in the full mph.
  auto fullMphIdx = _language_stdlib_getMphIdx(scalar, CASE_FULL_FOLD_LEVEL_COUNT,
                                            _language_stdlib_case_full_keys,
                                            _language_stdlib_case_full_ranks,
                                            _language_stdlib_case_full_sizes);
  
  auto fullCaseValue = _language_stdlib_case_full[fullMphIdx];

  // Count is either 2 or 3.
  auto count = fullCaseValue >> 62;

  for (__language_uint64_t i = 0; i != count; i += 1) {
    auto distance = (__language_int32_t)(fullCaseValue & 0xFFFF);

    if ((fullCaseValue & 0x10000) != 0) {
      distance = -distance;
    }

    fullCaseValue >>= 17;

    auto mappedScalar = (__language_uint32_t)((__language_int32_t)(scalar) - distance);

    buffer[i] = mappedScalar;
  }
#endif
}
