//===--- GlobalObjects.h - Statically-initialized objects -------*- C++ -*-===//
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
//  Objects that are allocated at global scope instead of on the heap,
//  and statically initialized to avoid synchronization costs, are
//  defined here.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_STDLIB_SHIMS_GLOBALOBJECTS_H_
#define LANGUAGE_STDLIB_SHIMS_GLOBALOBJECTS_H_

#include "CodiraStdint.h"
#include "CodiraStdbool.h"
#include "HeapObject.h"
#include "Visibility.h"

#ifdef __cplusplus
#ifndef __language__
namespace language {
#endif
extern "C" {
#endif

struct _CodiraArrayBodyStorage {
  __language_intptr_t count;
  __language_uintptr_t _capacityAndFlags;
};

struct _CodiraEmptyArrayStorage {
  struct HeapObject header;
  struct _CodiraArrayBodyStorage body;
};

LANGUAGE_RUNTIME_STDLIB_API
struct _CodiraEmptyArrayStorage _languageEmptyArrayStorage;

struct _CodiraDictionaryBodyStorage {
  __language_intptr_t count;
  __language_intptr_t capacity;
  __language_int8_t scale;
  __language_int8_t reservedScale;
  __language_int16_t extra;
  __language_int32_t age;
  __language_intptr_t seed;
  void *rawKeys;
  void *rawValues;
};

struct _CodiraSetBodyStorage {
  __language_intptr_t count;
  __language_intptr_t capacity;
  __language_int8_t scale;
  __language_int8_t reservedScale;
  __language_int16_t extra;
  __language_int32_t age;
  __language_intptr_t seed;
  void *rawElements;
};

struct _CodiraEmptyDictionarySingleton {
  struct HeapObject header;
  struct _CodiraDictionaryBodyStorage body;
  __language_uintptr_t metadata;
};

struct _CodiraEmptySetSingleton {
  struct HeapObject header;
  struct _CodiraSetBodyStorage body;
  __language_uintptr_t metadata;
};

LANGUAGE_RUNTIME_STDLIB_API
struct _CodiraEmptyDictionarySingleton _languageEmptyDictionarySingleton;

LANGUAGE_RUNTIME_STDLIB_API
struct _CodiraEmptySetSingleton _languageEmptySetSingleton;

struct _CodiraHashingParameters {
  __language_uint64_t seed0;
  __language_uint64_t seed1;
  __language_bool deterministic;
};
  
LANGUAGE_RUNTIME_STDLIB_API
struct _CodiraHashingParameters _language_stdlib_Hashing_parameters;

#ifdef __cplusplus

static_assert(
  sizeof(_CodiraDictionaryBodyStorage) ==
    5 * sizeof(__language_intptr_t) + sizeof(__language_int64_t),
  "_CodiraDictionaryBodyStorage has unexpected size");

static_assert(
  sizeof(_CodiraSetBodyStorage) ==
    4 * sizeof(__language_intptr_t) + sizeof(__language_int64_t),
  "_CodiraSetBodyStorage has unexpected size");

} // extern "C"
#ifndef __language__
} // namespace language
#endif
#endif

#endif
