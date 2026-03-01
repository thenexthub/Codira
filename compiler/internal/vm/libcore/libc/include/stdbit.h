/*
 * Copyright (C) 2025 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include <sys/cdefs.h>

/* C23 7.18.1 says this header also makes int*_t/uint*_t etc available. */
#include <stdint.h>

__BEGIN_DECLS

/* C23 7.18.1 General */

/** Declares this implementation as supporting the C23 functionality. */
#define __STDC_VERSION_STDBIT_H__ 202311L


/* C23 7.18.2 Endian */

/** Constant representing little-endian. */
#define __STDC_ENDIAN_LITTLE__ 1234
/** Constant representing big-endian. */
#define __STDC_ENDIAN_BIG__ 4321

/**
 * Constant representing the endian of the target.
 * Android is always little-endian.
 */
#define __STDC_ENDIAN_NATIVE__ __STDC_ENDIAN_LITTLE__

/*
 * Notes on the implementation.
 *
 * Ideally we'd have clang builtins for all of these, like gcc has.
 * If/when clang catches up, we can replace all of this with direct calls to
 * the relevant builtins.
 *
 * In the meantime:
 *
 * We only use macros because (a) we want to support all api levels and
 * (b) the whole point of these functions is that they should all be just an
 * instruction or two, where function call overhead would ruin everything.
 *
 * Where there's a similar generic builtin (such as __builtin_clzg) we
 * forward all the type-specific variants to that.
 *
 * Even when not, we try to forward all the type-specific variants to the
 * generic variant so it's all "obviously correct".
 *
 * (It's 2025, and this will only be used in new code, so we haven't bothered
 * with special cases for ILP32 systems.)
 *
 * We cast the arguments to the expected argument types because the builtins
 * do argument type checking that the <stdbit.h> functionality is not expected
 * to do (causing warnings rather than silent conversion when passing a signed
 * int, say).
 *
 * We cast the results to the expected `unsigned int` type because the builtins
 * return signed results.
 *
 * We don't worry about C++ casts because it's unclear how this file is
 * supposed to interact with C++, and C++ has had <bit> longer than C has
 * had <stdbit.h>.
 */


/* C23 7.18.3 Count Leading Zeros */

/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros_uc(__x) stdc_leading_zeros((unsigned char) (__x))
/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros_us(__x) stdc_leading_zeros((unsigned short) (__x))
/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros_ui(__x) stdc_leading_zeros((unsigned int) (__x))
/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros_ul(__x) stdc_leading_zeros((unsigned long) (__x))
/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros_ull(__x) stdc_leading_zeros((unsigned long long) (__x))
/** Returns the count of consecutive 0 bits, starting from the most-significant. */
#define stdc_leading_zeros(__x) \
    ((unsigned int) __builtin_clzg(__x, (int) (8 * sizeof(__x))))


/* C23 7.18.4 Count Leading Ones */

/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones_uc(__x) stdc_leading_ones((unsigned char) (__x))
/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones_us(__x) stdc_leading_ones((unsigned short) (__x))
/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones_ui(__x) stdc_leading_ones((unsigned int) (__x))
/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones_ul(__x) stdc_leading_ones((unsigned long) (__x))
/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones_ull(__x) stdc_leading_ones((unsigned long long) (__x))
/** Returns the count of consecutive 1 bits, starting from the most-significant. */
#define stdc_leading_ones(__x) \
    ((unsigned int) (__builtin_clzg(~(((unsigned long long) (__x)) << ((8 * sizeof(unsigned long long)) - (8 * sizeof(__x)))), \
                                    (int) (8 * sizeof(__x)))))


/* C23 7.18.5 Count Trailing Zeros */

/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros_uc(__x) stdc_trailing_zeros((unsigned char) (__x))
/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros_us(__x) stdc_trailing_zeros((unsigned short) (__x))
/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros_ui(__x) stdc_trailing_zeros((unsigned int) (__x))
/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros_ul(__x) stdc_trailing_zeros((unsigned long) (__x))
/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros_ull(__x) stdc_trailing_zeros((unsigned long long) (__x))
/** Returns the count of consecutive 0 bits, starting from the least-significant. */
#define stdc_trailing_zeros(__x) \
    ((unsigned int) __builtin_ctzg(__x, \
                                   (int) (8 * sizeof(__x))))


/* C23 7.18.6 Count Trailing Ones */

/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones_uc(__x) stdc_trailing_ones((unsigned char) (__x))
/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones_us(__x) stdc_trailing_ones((unsigned short) (__x))
/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones_ui(__x) stdc_trailing_ones((unsigned int) (__x))
/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones_ul(__x) stdc_trailing_ones((unsigned long) (__x))
/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones_ull(__x) stdc_trailing_ones((unsigned long long) (__x))
/** Returns the count of consecutive 1 bits, starting from the least-significant. */
#define stdc_trailing_ones(__x) \
    ((unsigned int) (__builtin_ctzg((typeof(__x)) ~(__x), \
                                    (int) (8 * sizeof(__x)))))


/* C23 7.18.7 First Leading Zero */

/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero_uc(__x) stdc_first_leading_zero((unsigned char) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero_us(__x) stdc_first_leading_zero((unsigned short) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero_ui(__x) stdc_first_leading_zero((unsigned int) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero_ul(__x) stdc_first_leading_zero((unsigned long) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero_ull(__x) stdc_first_leading_zero((unsigned long long) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_zero(__x) ({ \
    typeof(__x) __val = (__x); \
    ((unsigned int) ((__val == (typeof(__x)) -1 \
        ? 0 \
        : 1 + __builtin_clzg((typeof(__x)) ~__val)))); \
  })


/* C23 7.18.8 First Leading One */

/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one_uc(__x) stdc_first_leading_one((unsigned char) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one_us(__x) stdc_first_leading_one((unsigned short) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one_ui(__x) stdc_first_leading_one((unsigned int) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one_ul(__x) stdc_first_leading_one((unsigned long) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one_ull(__x) stdc_first_leading_one((unsigned long long) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the most-significant (0 if none). */
#define stdc_first_leading_one(__x) ({ \
    typeof(__x) __val = (__x); \
    ((unsigned int) ((__val == (typeof(__val)) 0 \
        ? 0 \
        : 1 + __builtin_clzg(__val)))); \
  })


/* C23 7.18.9 First Trailing Zero */
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero_uc(__x) stdc_first_trailing_zero((unsigned char) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero_us(__x) stdc_first_trailing_zero((unsigned short) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero_ui(__x) stdc_first_trailing_zero((unsigned int) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero_ul(__x) stdc_first_trailing_zero((unsigned long) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero_ull(__x) stdc_first_trailing_zero((unsigned long long) (__x))
/** Returns the index of the first 0 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_zero(__x) ({ \
    typeof(__x) __val = (__x); \
    ((unsigned int) ((__val == (typeof(__val)) -1 \
        ? 0 \
        : 1 + __builtin_ctzg((typeof(__val)) ~__val)))); \
  })

/* C23 7.18.10 First Trailing One */
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one_uc(__x) stdc_first_trailing_one((unsigned char) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one_us(__x) stdc_first_trailing_one((unsigned short) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one_ui(__x) stdc_first_trailing_one((unsigned int) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one_ul(__x) stdc_first_trailing_one((unsigned long) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one_ull(__x) stdc_first_trailing_one((unsigned long long) (__x))
/** Returns the index of the first 1 bit, plus 1, starting from the least-significant (0 if none). */
#define stdc_first_trailing_one(__x) \
    ((unsigned int) (((__x) == (typeof(__x)) 0 \
        ? 0 \
        : 1 + __builtin_ctzg(__x))))


/* C23 7.18.11 Count Zeros */

/** Returns the count of 0 bits. */
#define stdc_count_zeros_uc(__x) stdc_count_zeros((unsigned char) (__x))
/** Returns the count of 0 bits. */
#define stdc_count_zeros_us(__x) stdc_count_zeros((unsigned short) (__x))
/** Returns the count of 0 bits. */
#define stdc_count_zeros_ui(__x) stdc_count_zeros((unsigned int) (__x))
/** Returns the count of 0 bits. */
#define stdc_count_zeros_ul(__x) stdc_count_zeros((unsigned long) (__x))
/** Returns the count of 0 bits. */
#define stdc_count_zeros_ull(__x) stdc_count_zeros((unsigned long long) (__x))
/** Returns the count of 0 bits. */
#define stdc_count_zeros(__x) ((unsigned int) ((8 * sizeof(__x)) -  __builtin_popcountg(__x)))


/* C23 7.18.12 Count Ones */

/** Returns the count of 1 bits. */
#define stdc_count_ones_uc(__x) stdc_count_ones((unsigned char) (__x))
/** Returns the count of 1 bits. */
#define stdc_count_ones_us(__x) stdc_count_ones((unsigned short) (__x))
/** Returns the count of 1 bits. */
#define stdc_count_ones_ui(__x) stdc_count_ones((unsigned int) (__x))
/** Returns the count of 1 bits. */
#define stdc_count_ones_ul(__x) stdc_count_ones((unsigned long) (__x))
/** Returns the count of 1 bits. */
#define stdc_count_ones_ull(__x) stdc_count_ones((unsigned long long) (__x))
/** Returns the count of 1 bits. */
#define stdc_count_ones(__x) ((unsigned int) __builtin_popcountg(__x))


/* C23 7.18.13 Single-Bit Check */
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit_uc(__x) stdc_has_single_bit((unsigned char) (__x))
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit_us(__x) stdc_has_single_bit((unsigned short) (__x))
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit_ui(__x) stdc_has_single_bit((unsigned int) (__x))
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit_ul(__x) stdc_has_single_bit((unsigned long) (__x))
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit_ull(__x) stdc_has_single_bit((unsigned long long) (__x))
/** Returns true if exactly one bit is set (that is: it's a non-zero power of 2). */
#define stdc_has_single_bit(__x) ({ \
    typeof(__x) __val = (__x); \
    ((bool) (__val != 0 && ((__val & (__val - 1)) == 0))); \
  })


/* C23 7.18.14 Bit Width */
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width_uc(__x) stdc_bit_width((unsigned char) (__x))
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width_us(__x) stdc_bit_width((unsigned short) (__x))
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width_ui(__x) stdc_bit_width((unsigned int) (__x))
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width_ul(__x) stdc_bit_width((unsigned long) (__x))
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width_ull(__x) stdc_bit_width((unsigned long long) (__x))
/** Returns the smallest number of bits needed to store the value (0 for 0). */
#define stdc_bit_width(__x) \
    ((unsigned int) (8 * sizeof(__x)) - __builtin_clzg(__x, \
                                                       (int) (8 * sizeof(__x))))


/* C23 7.18.15 Bit Floor */
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor_uc(__x) stdc_bit_floor((unsigned char) (__x))
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor_us(__x) stdc_bit_floor((unsigned short) (__x))
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor_ui(__x) stdc_bit_floor((unsigned int) (__x))
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor_ul(__x) stdc_bit_floor((unsigned long) (__x))
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor_ull(__x) stdc_bit_floor((unsigned long long) (__x))
/** Returns the largest integral power of 2 not greater than the value (0 for 0). */
#define stdc_bit_floor(__x) ({ \
    typeof(__x) __val = (__x); \
    ((typeof(__val)) (__val == 0 ? 0 : ((typeof(__val)) 1) << (stdc_bit_width(__val) - 1))); \
  })


/* C23 7.18.15 Bit Ceil */
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil_uc(__x) stdc_bit_ceil((unsigned char) (__x))
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil_us(__x) stdc_bit_ceil((unsigned short) (__x))
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil_ui(__x) stdc_bit_ceil((unsigned int) (__x))
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil_ul(__x) stdc_bit_ceil((unsigned long) (__x))
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil_ull(__x) stdc_bit_ceil((unsigned long long) (__x))
/** Returns the smallest integral power of 2 not less than the value. */
#define stdc_bit_ceil(__x) ({ \
    typeof(__x) __result = 1; \
    typeof(__x) __val = (__x); \
    if (__val > 1) { \
      /* Avoid shifts greater than the width of the type. */ \
      unsigned __shift = stdc_bit_width(__val - 1u); \
      if (__shift < 8 * sizeof(__result)) __result = 1ull << __shift; \
      else __result = 0; \
    } \
    __result; })

__END_DECLS
