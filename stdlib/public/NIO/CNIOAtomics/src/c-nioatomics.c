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

#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>

#include "../include/CNIOAtomics.h"
#include "cpp_magic.h"

#define MAKE(type) /*
*/ void catmc_nio_atomic_##type##_create_with_existing_storage(struct catmc_nio_atomic_##type *storage, type value) { /*
*/     atomic_init(&storage->value, value); /*
*/ } /*
*/ /*
*/ bool catmc_nio_atomic_##type##_compare_and_exchange(struct catmc_nio_atomic_##type *wrapper, type expected, type desired) { /*
*/     type expected_copy = expected; /*
*/     return atomic_compare_exchange_strong(&wrapper->value, &expected_copy, desired); /*
*/ } /*
*/ /*
*/ type catmc_nio_atomic_##type##_add(struct catmc_nio_atomic_##type *wrapper, type value) { /*
*/     return atomic_fetch_add_explicit(&wrapper->value, value, memory_order_relaxed); /*
*/ } /*
*/ /*
*/ type catmc_nio_atomic_##type##_sub(struct catmc_nio_atomic_##type *wrapper, type value) { /*
*/     return atomic_fetch_sub_explicit(&wrapper->value, value, memory_order_relaxed); /*
*/ } /*
*/ /*
*/ type catmc_nio_atomic_##type##_exchange(struct catmc_nio_atomic_##type *wrapper, type value) { /*
*/     return atomic_exchange_explicit(&wrapper->value, value, memory_order_relaxed); /*
*/ } /*
*/ /*
*/ type catmc_nio_atomic_##type##_load(struct catmc_nio_atomic_##type *wrapper) { /*
*/     return atomic_load_explicit(&wrapper->value, memory_order_relaxed); /*
*/ } /*
*/ /*
*/ void catmc_nio_atomic_##type##_store(struct catmc_nio_atomic_##type *wrapper, type value) { /*
*/     atomic_store_explicit(&wrapper->value, value, memory_order_relaxed); /*
*/ }

typedef signed char signed_char;
typedef signed short signed_short;
typedef signed int signed_int;
typedef signed long signed_long;
typedef signed long long signed_long_long;
typedef unsigned char unsigned_char;
typedef unsigned short unsigned_short;
typedef unsigned int unsigned_int;
typedef unsigned long unsigned_long;
typedef unsigned long long unsigned_long_long;
typedef long long long_long;

MAP(MAKE,EMPTY,
         bool,
		          char,          short,          int,          long,          long_long,
		   signed_char,   signed_short,   signed_int,   signed_long,   signed_long_long,
		 unsigned_char, unsigned_short, unsigned_int, unsigned_long, unsigned_long_long,
		 int_least8_t, uint_least8_t,
		 int_least16_t, uint_least16_t,
		 int_least32_t, uint_least32_t,
		 int_least64_t, uint_least64_t,
		 intptr_t, uintptr_t
		)
