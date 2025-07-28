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

#ifndef SOURCEKITLSP_CATOMICS_H
#define SOURCEKITLSP_CATOMICS_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

typedef struct {
  _Atomic(uint32_t) value;
} CAtomicUInt32;

static inline CAtomicUInt32 *_Nonnull atomic_uint32_create(uint32_t initialValue) {
  CAtomicUInt32 *atomic = malloc(sizeof(CAtomicUInt32));
  atomic->value = initialValue;
  return atomic;
}

static inline uint32_t atomic_uint32_get(CAtomicUInt32 *_Nonnull atomic) {
  return atomic->value;
}

static inline void atomic_uint32_set(CAtomicUInt32 *_Nonnull atomic, uint32_t newValue) {
  atomic->value = newValue;
}

static inline uint32_t atomic_uint32_fetch_and_increment(CAtomicUInt32 *_Nonnull atomic) {
  return atomic->value++;
}

static inline void atomic_uint32_destroy(CAtomicUInt32 *_Nonnull atomic) {
  free(atomic);
}

typedef struct {
  _Atomic(int32_t) value;
} CAtomicInt32;

static inline CAtomicInt32 *_Nonnull atomic_int32_create(int32_t initialValue) {
  CAtomicInt32 *atomic = malloc(sizeof(CAtomicInt32));
  atomic->value = initialValue;
  return atomic;
}

static inline int32_t atomic_int32_get(CAtomicInt32 *_Nonnull atomic) {
  return atomic->value;
}

static inline void atomic_int32_set(CAtomicInt32 *_Nonnull atomic, int32_t newValue) {
  atomic->value = newValue;
}

static inline int32_t atomic_int32_fetch_and_increment(CAtomicInt32 *_Nonnull atomic) {
  return atomic->value++;
}

static inline void atomic_int32_destroy(CAtomicInt32 *_Nonnull atomic) {
  free(atomic);
}

#endif // SOURCEKITLSP_CATOMICS_H
