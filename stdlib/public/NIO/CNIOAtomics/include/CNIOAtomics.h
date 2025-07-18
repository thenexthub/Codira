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

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_ATOMIC_OPERATIONS(type, name)                                                                                         \
  struct catmc_atomic_ ## name;                                                                                                       \
                                                                                                                                      \
  struct catmc_atomic_ ## name * _Nonnull catmc_atomic_ ## name ## _create(type value);                                               \
  void catmc_atomic_ ## name ## _destroy(struct catmc_atomic_ ## name * _Nonnull);                                                    \
  bool catmc_atomic_ ## name ## _compare_and_exchange(struct catmc_atomic_ ## name * _Nonnull, type expected, type desired);          \
  type catmc_atomic_ ## name ## _add(struct catmc_atomic_ ## name * _Nonnull, type value);                                            \
  type catmc_atomic_ ## name ## _sub(struct catmc_atomic_ ## name * _Nonnull, type value);                                            \
  type catmc_atomic_ ## name ## _exchange(struct catmc_atomic_ ## name * _Nonnull, type value);                                       \
  type catmc_atomic_ ## name ## _load(struct catmc_atomic_ ## name * _Nonnull);                                                       \
  void catmc_atomic_ ## name ## _store(struct catmc_atomic_ ## name * _Nonnull, type value);                                          \
                                                                                                                                      \
  struct catmc_nio_atomic_ ## name {                                                                                                  \
    _Atomic type value;                                                                                                               \
  };                                                                                                                                  \
                                                                                                                                      \
  void catmc_nio_atomic_ ## name ## _create_with_existing_storage(struct catmc_nio_atomic_ ## name * _Nonnull, type value);           \
  bool catmc_nio_atomic_ ## name ## _compare_and_exchange(struct catmc_nio_atomic_ ## name * _Nonnull, type expected, type desired);  \
  type catmc_nio_atomic_ ## name ## _add(struct catmc_nio_atomic_ ## name * _Nonnull, type value);                                    \
  type catmc_nio_atomic_ ## name ## _sub(struct catmc_nio_atomic_ ## name * _Nonnull, type value);                                    \
  type catmc_nio_atomic_ ## name ## _exchange(struct catmc_nio_atomic_ ## name * _Nonnull, type value);                               \
  type catmc_nio_atomic_ ## name ## _load(struct catmc_nio_atomic_ ## name * _Nonnull);                                               \
  void catmc_nio_atomic_ ## name ## _store(struct catmc_nio_atomic_ ## name * _Nonnull, type value);                                  \

#ifdef __cplusplus
DECLARE_ATOMIC_OPERATIONS(bool, _Bool)
#else
DECLARE_ATOMIC_OPERATIONS(_Bool, _Bool)
#endif
DECLARE_ATOMIC_OPERATIONS(char, char)
DECLARE_ATOMIC_OPERATIONS(short, short)
DECLARE_ATOMIC_OPERATIONS(int, int)
DECLARE_ATOMIC_OPERATIONS(long, long)
DECLARE_ATOMIC_OPERATIONS(long long, long_long)

DECLARE_ATOMIC_OPERATIONS(signed char, signed_char)
DECLARE_ATOMIC_OPERATIONS(signed short, signed_short)
DECLARE_ATOMIC_OPERATIONS(signed int, signed_int)
DECLARE_ATOMIC_OPERATIONS(signed long, signed_long)
DECLARE_ATOMIC_OPERATIONS(signed long long, signed_long_long)

DECLARE_ATOMIC_OPERATIONS(unsigned char, unsigned_char)
DECLARE_ATOMIC_OPERATIONS(unsigned short, unsigned_short)
DECLARE_ATOMIC_OPERATIONS(unsigned int, unsigned_int)
DECLARE_ATOMIC_OPERATIONS(unsigned long, unsigned_long)
DECLARE_ATOMIC_OPERATIONS(unsigned long long, unsigned_long_long)

DECLARE_ATOMIC_OPERATIONS(int_least8_t, int_least8_t)
DECLARE_ATOMIC_OPERATIONS(uint_least8_t, uint_least8_t)

DECLARE_ATOMIC_OPERATIONS(int_least16_t, int_least16_t)
DECLARE_ATOMIC_OPERATIONS(uint_least16_t, uint_least16_t)

DECLARE_ATOMIC_OPERATIONS(int_least32_t, int_least32_t)
DECLARE_ATOMIC_OPERATIONS(uint_least32_t, uint_least32_t)

DECLARE_ATOMIC_OPERATIONS(int_least64_t, int_least64_t)
DECLARE_ATOMIC_OPERATIONS(uint_least64_t, uint_least64_t)

DECLARE_ATOMIC_OPERATIONS(intptr_t, intptr_t)
DECLARE_ATOMIC_OPERATIONS(uintptr_t, uintptr_t)

#undef DECLARE_ATOMIC_OPERATIONS

#ifdef __cplusplus
}  /* extern "C" */
#endif
