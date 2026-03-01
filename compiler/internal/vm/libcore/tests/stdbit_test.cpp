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

#include <sys/cdefs.h>

#if __has_include(<stdbit.h>)

#include <gtest/gtest.h>

#include <stdbit.h>

#include <limits.h>

typedef unsigned char uc;
typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned long int ul;
typedef unsigned long long int ull;

TEST(stdbit, version) {
  EXPECT_EQ(202311L, __STDC_VERSION_STDBIT_H__);
}

TEST(stdbit, endian) {
  EXPECT_EQ(__STDC_ENDIAN_LITTLE__, __STDC_ENDIAN_NATIVE__);
}

static constexpr unsigned kLongBit = static_cast<unsigned>(LONG_BIT);

TEST(stdbit, stdc_leading_zeros) {
  EXPECT_EQ(64u, stdc_leading_zeros(0x0000000000000000ull));
  EXPECT_EQ(32u, stdc_leading_zeros(0x00000000ffffffffull));
  EXPECT_EQ(48u, stdc_leading_zeros(0x000000000000ffffull));
  EXPECT_EQ(0u, stdc_leading_zeros(0xffffffffffffffffull));
}

TEST(stdbit, stdc_leading_zeros_forwarding) {
  EXPECT_EQ(8u, stdc_leading_zeros_uc(0x0000000000000000ull));
  EXPECT_EQ(16u, stdc_leading_zeros_us(0x0000000000000000ull));
  EXPECT_EQ(32u, stdc_leading_zeros_ui(0x0000000000000000ull));
  EXPECT_EQ(kLongBit, stdc_leading_zeros_ul(0x0000000000000000ull));
  EXPECT_EQ(64u, stdc_leading_zeros_ull(0x0000000000000000ull));
}

TEST(stdbit, stdc_leading_ones) {
  EXPECT_EQ(64u, stdc_leading_ones(0xffffffffffffffffull));
  EXPECT_EQ(32u, stdc_leading_ones(0xffffffff00000000ull));
  EXPECT_EQ(16u, stdc_leading_ones(0xffff000000000000ull));
  EXPECT_EQ(0u, stdc_leading_ones(0x0000000000000000ull));
}

TEST(stdbit, stdc_leading_ones_forwarding) {
  EXPECT_EQ(8u, stdc_leading_ones_uc(0xffffffffffffffffull));
  EXPECT_EQ(16u, stdc_leading_ones_us(0xffffffffffffffffull));
  EXPECT_EQ(32u, stdc_leading_ones_ui(0xffffffffffffffffull));
  EXPECT_EQ(kLongBit, stdc_leading_ones_ul(0xffffffffffffffffull));
  EXPECT_EQ(64u, stdc_leading_ones_ull(0xffffffffffffffffull));
}

TEST(stdbit, stdc_trailing_zeros) {
  EXPECT_EQ(64u, stdc_trailing_zeros(0x0000000000000000ull));
  EXPECT_EQ(0u, stdc_trailing_zeros(0x0000000000000001ull));
  EXPECT_EQ(8u, stdc_trailing_zeros(0x0000000000000100ull));
  EXPECT_EQ(16u, stdc_trailing_zeros(0x0000000000010000ull));
  EXPECT_EQ(32u, stdc_trailing_zeros(0x0000000100000000ull));
  EXPECT_EQ(63u, stdc_trailing_zeros(0x8000000000000000ull));
  EXPECT_EQ(0u, stdc_trailing_zeros(0xffffffffffffffffull));
}

TEST(stdbit, stdc_trailing_zeros_forwarding) {
  EXPECT_EQ(8u, stdc_trailing_zeros_uc(0x0000000000000000ull));
  EXPECT_EQ(16u, stdc_trailing_zeros_us(0x0000000000000000ull));
  EXPECT_EQ(32u, stdc_trailing_zeros_ui(0x0000000000000000ull));
  EXPECT_EQ(kLongBit, stdc_trailing_zeros_ul(0x0000000000000000ull));
  EXPECT_EQ(64u, stdc_trailing_zeros_ull(0x0000000000000000ull));
}

TEST(stdbit, stdc_trailing_ones) {
  EXPECT_EQ(0u, stdc_trailing_ones(0x0000000000000000ull));
  EXPECT_EQ(8u, stdc_trailing_ones(0x00000000000000ffull));
  EXPECT_EQ(16u, stdc_trailing_ones(0x000000000000ffffull));
  EXPECT_EQ(32u, stdc_trailing_ones(0x00000000ffffffffull));
  EXPECT_EQ(64u, stdc_trailing_ones(0xffffffffffffffffull));
}

TEST(stdbit, stdc_trailing_ones_forwarding) {
  EXPECT_EQ(8u, stdc_trailing_ones_uc(0xffffffffffffffffull));
  EXPECT_EQ(16u, stdc_trailing_ones_us(0xffffffffffffffffull));
  EXPECT_EQ(32u, stdc_trailing_ones_ui(0xffffffffffffffffull));
  EXPECT_EQ(kLongBit, stdc_trailing_ones_ul(0xffffffffffffffffull));
  EXPECT_EQ(64u, stdc_trailing_ones_ull(0xffffffffffffffffull));
}

TEST(stdbit, stdc_first_leading_zero) {
  EXPECT_EQ(1u, stdc_first_leading_zero(0x0000000000000000ull));
  EXPECT_EQ(9u, stdc_first_leading_zero(0xff00000000000000ull));
  EXPECT_EQ(17u, stdc_first_leading_zero(0xffff000000000000ull));
  EXPECT_EQ(33u, stdc_first_leading_zero(0xffffffff00000000ull));
  EXPECT_EQ(0u, stdc_first_leading_zero(0xffffffffffffffffull));
}

TEST(stdbit, stdc_first_leading_zero_forwarding) {
  EXPECT_EQ(8u, stdc_first_leading_zero_uc(0xfffffffffffffffeull));
  EXPECT_EQ(16u, stdc_first_leading_zero_us(0xfffffffffffffffeull));
  EXPECT_EQ(32u, stdc_first_leading_zero_ui(0xfffffffffffffffeull));
#if defined(__LP64__)
  EXPECT_EQ(64u, stdc_first_leading_zero_ul(0xfffffffffffffffeull));
#else
  EXPECT_EQ(32u, stdc_first_leading_zero_ul(0xfffffffffffffffeull));
#endif
  EXPECT_EQ(64u, stdc_first_leading_zero_ull(0xfffffffffffffffeull));
}

TEST(stdbit, stdc_first_leading_one) {
  EXPECT_EQ(0u, stdc_first_leading_one(0x0000000000000000ull));
  EXPECT_EQ(57u, stdc_first_leading_one(0x00000000000000ffull));
  EXPECT_EQ(49u, stdc_first_leading_one(0x000000000000ffffull));
  EXPECT_EQ(33u, stdc_first_leading_one(0x00000000ffffffffull));
  EXPECT_EQ(1u, stdc_first_leading_one(0xffffffffffffffffull));
}

TEST(stdbit, stdc_first_leading_one_forwarding) {
  EXPECT_EQ(8u, stdc_first_leading_one_uc(0x0000000000000001ull));
  EXPECT_EQ(16u, stdc_first_leading_one_us(0x0000000000000001ull));
  EXPECT_EQ(32u, stdc_first_leading_one_ui(0x0000000000000001ull));
  EXPECT_EQ(kLongBit, stdc_first_leading_one_ul(0x0000000000000001ull));
  EXPECT_EQ(64u, stdc_first_leading_one_ull(0x0000000000000001ull));
}

TEST(stdbit, stdc_first_trailing_zero) {
  EXPECT_EQ(1u, stdc_first_trailing_zero(0x0000000000000000ull));
  EXPECT_EQ(9u, stdc_first_trailing_zero(0x00000000000000ffull));
  EXPECT_EQ(17u, stdc_first_trailing_zero(0x000000000000ffffull));
  EXPECT_EQ(33u, stdc_first_trailing_zero(0x00000000ffffffffull));
  EXPECT_EQ(0u, stdc_first_trailing_zero(0xffffffffffffffffull));
}

TEST(stdbit, stdc_first_trailing_zero_forwarding) {
  EXPECT_EQ(8u, stdc_first_trailing_zero_uc(0x00000000000007full));
  EXPECT_EQ(16u, stdc_first_trailing_zero_us(0x000000000007fffull));
  EXPECT_EQ(32u, stdc_first_trailing_zero_ui(0x00000007fffffffull));
#if defined(__LP64__)
  EXPECT_EQ(64u, stdc_first_trailing_zero_ul(0x7fffffffffffffffull));
#else
  EXPECT_EQ(32u, stdc_first_trailing_zero_ul(0x00000007fffffffull));
#endif
  EXPECT_EQ(64u, stdc_first_trailing_zero_ull(0x7fffffffffffffffull));
}

TEST(stdbit, stdc_first_trailing_one) {
  EXPECT_EQ(0u, stdc_first_trailing_one(0x0000000000000000ull));
  EXPECT_EQ(57u, stdc_first_trailing_one(0xff00000000000000ull));
  EXPECT_EQ(49u, stdc_first_trailing_one(0xffff000000000000ull));
  EXPECT_EQ(33u, stdc_first_trailing_one(0xffffffff00000000ull));
  EXPECT_EQ(1u, stdc_first_trailing_one(0xffffffffffffffffull));
}

TEST(stdbit, stdc_first_trailing_one_forwarding) {
  EXPECT_EQ(8u, stdc_first_trailing_one_uc(0x000000000000080ull));
  EXPECT_EQ(16u, stdc_first_trailing_one_us(0x000000000008000ull));
  EXPECT_EQ(32u, stdc_first_trailing_one_ui(0x000000080000000ull));
#if defined(__LP64__)
  EXPECT_EQ(64u, stdc_first_trailing_one_ul(0x8000000000000000ull));
#else
  EXPECT_EQ(32u, stdc_first_trailing_one_ul(0x000000080000000ull));
#endif
  EXPECT_EQ(64u, stdc_first_trailing_one_ull(0x8000000000000000ull));
}

TEST(stdbit, stdc_count_zeros) {
  EXPECT_EQ(64u, stdc_count_zeros(0x0000000000000000ull));
  EXPECT_EQ(56u, stdc_count_zeros(0xff00000000000000ull));
  EXPECT_EQ(48u, stdc_count_zeros(0xffff000000000000ull));
  EXPECT_EQ(32u, stdc_count_zeros(0xffffffff00000000ull));
  EXPECT_EQ(0u, stdc_count_zeros(0xffffffffffffffffull));
}

TEST(stdbit, stdc_count_zeros_forwarding) {
  EXPECT_EQ(8u, stdc_count_zeros_uc(0x0000000000000000ull));
  EXPECT_EQ(16u, stdc_count_zeros_us(0x0000000000000000ull));
  EXPECT_EQ(32u, stdc_count_zeros_ui(0x0000000000000000ull));
  EXPECT_EQ(kLongBit, stdc_count_zeros_ul(0x0000000000000000ull));
  EXPECT_EQ(64u, stdc_count_zeros_ull(0x0000000000000000ull));
}

TEST(stdbit, stdc_count_ones) {
  EXPECT_EQ(0u, stdc_count_ones(0x0000000000000000ull));
  EXPECT_EQ(8u, stdc_count_ones(0xff00000000000000ull));
  EXPECT_EQ(16u, stdc_count_ones(0xffff000000000000ull));
  EXPECT_EQ(32u, stdc_count_ones(0xffffffff00000000ull));
  EXPECT_EQ(64u, stdc_count_ones(0xffffffffffffffffull));
}

TEST(stdbit, stdc_count_ones_forwarding) {
  EXPECT_EQ(8u, stdc_count_ones_uc(0xffffffffffffffffull));
  EXPECT_EQ(16u, stdc_count_ones_us(0xffffffffffffffffull));
  EXPECT_EQ(32u, stdc_count_ones_ui(0xffffffffffffffffull));
  EXPECT_EQ(kLongBit, stdc_count_ones_ul(0xffffffffffffffffull));
  EXPECT_EQ(64u, stdc_count_ones_ull(0xffffffffffffffffull));
}

TEST(stdbit, stdc_has_single_bit) {
  EXPECT_EQ(false, stdc_has_single_bit(0x0000000000000000ull));
  EXPECT_EQ(false, stdc_has_single_bit(0x0000000000000003ull));
  EXPECT_EQ(false, stdc_has_single_bit(0x1000000000000001ull));
  EXPECT_EQ(false, stdc_has_single_bit(0xffffffffffffffffull));

  EXPECT_EQ(true, stdc_has_single_bit(0x0000000000000001ull));
  EXPECT_EQ(true, stdc_has_single_bit(0x0000000000008000ull));
  EXPECT_EQ(true, stdc_has_single_bit(0x0000000080000000ull));
  EXPECT_EQ(true, stdc_has_single_bit(0x8000000000000000ull));
}

TEST(stdbit, stdc_has_single_bit_forwarding) {
  EXPECT_EQ(true, stdc_has_single_bit_uc(0x8000000080008080ull));
  EXPECT_EQ(true, stdc_has_single_bit_us(0x8000000080008000ull));
  EXPECT_EQ(true, stdc_has_single_bit_ui(0x8000000080000000ull));
#if defined(__LP64__)
  EXPECT_EQ(true, stdc_has_single_bit_ul(0x8000000000000000ull));
#else
  EXPECT_EQ(true, stdc_has_single_bit_ul(0x8000000080000000ull));
#endif
  EXPECT_EQ(true, stdc_has_single_bit_ull(0x8000000000000000ull));
}

TEST(stdbit, stdc_bit_width) {
  EXPECT_EQ(0u, stdc_bit_width(0x0000000000000000ull));
  EXPECT_EQ(8u, stdc_bit_width(0x0000000000000080ull));
  EXPECT_EQ(16u, stdc_bit_width(0x0000000000008000ull));
  EXPECT_EQ(32u, stdc_bit_width(0x0000000080000000ull));
  EXPECT_EQ(64u, stdc_bit_width(0x8000000000000000ull));
  EXPECT_EQ(64u, stdc_bit_width(0xffffffffffffffffull));
}

TEST(stdbit, stdc_bit_width_forwarding) {
  EXPECT_EQ(8u, stdc_bit_width_uc(0xffffffffffffffffull));
  EXPECT_EQ(16u, stdc_bit_width_us(0xffffffffffffffffull));
  EXPECT_EQ(32u, stdc_bit_width_ui(0xffffffffffffffffull));
  EXPECT_EQ(kLongBit, stdc_bit_width_ul(0xffffffffffffffffull));
  EXPECT_EQ(64u, stdc_bit_width_ull(0xffffffffffffffffull));
}

TEST(stdbit, stdc_bit_floor) {
  EXPECT_EQ(0x0000000000000000ull, stdc_bit_floor(0x0000000000000000ull));

  EXPECT_EQ(0x0000000000000040ull, stdc_bit_floor(0x000000000000007full));
  EXPECT_EQ(0x0000000000000080ull, stdc_bit_floor(0x0000000000000080ull));
  EXPECT_EQ(0x0000000000000100ull, stdc_bit_floor(0x0000000000000100ull));
  EXPECT_EQ(0x0000000000004000ull, stdc_bit_floor(0x0000000000007fffull));
  EXPECT_EQ(0x0000000000008000ull, stdc_bit_floor(0x0000000000008000ull));
  EXPECT_EQ(0x0000000000010000ull, stdc_bit_floor(0x0000000000010000ull));
  EXPECT_EQ(0x0000000040000000ull, stdc_bit_floor(0x000000007fffffffull));
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_floor(0x0000000080000000ull));
  EXPECT_EQ(0x0000000100000000ull, stdc_bit_floor(0x0000000100000000ull));
  EXPECT_EQ(0x4000000000000000ull, stdc_bit_floor(0x7fffffffffffffffull));
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_floor(0x8000000000000000ull));

  EXPECT_EQ(0x8000000000000000ull, stdc_bit_floor(0xffffffffffffffffull));
}

TEST(stdbit, stdc_bit_floor_forwarding) {
  EXPECT_EQ(0x0000000000000080ull, stdc_bit_floor_uc(0x8000000080008080ull));
  EXPECT_EQ(0x0000000000008000ull, stdc_bit_floor_us(0x8000000080008080ull));
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_floor_ui(0x8000000080008080ull));
#if defined(__LP64__)
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_floor_ul(0x8000000080008080ull));
#else
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_floor_ul(0x8000000080008080ull));
#endif
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_floor_ull(0x8000000080008080ull));
}

TEST(stdbit, stdc_bit_ceil) {
  EXPECT_EQ(0x0000000000000001ull, stdc_bit_ceil(0x0000000000000000ull));

  EXPECT_EQ(0x0000000000000080ull, stdc_bit_ceil(0x000000000000007full));
  EXPECT_EQ(0x0000000000000080ull, stdc_bit_ceil(0x0000000000000080ull));
  EXPECT_EQ(0x0000000000000100ull, stdc_bit_ceil(0x0000000000000100ull));
  EXPECT_EQ(0x0000000000008000ull, stdc_bit_ceil(0x0000000000007fffull));
  EXPECT_EQ(0x0000000000008000ull, stdc_bit_ceil(0x0000000000008000ull));
  EXPECT_EQ(0x0000000000010000ull, stdc_bit_ceil(0x0000000000010000ull));
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_ceil(0x000000007fffffffull));
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_ceil(0x0000000080000000ull));
  EXPECT_EQ(0x0000000100000000ull, stdc_bit_ceil(0x0000000100000000ull));
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_ceil(0x7fffffffffffffffull));
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_ceil(0x8000000000000000ull));

  EXPECT_EQ(0x0000000000000000ull, stdc_bit_ceil(0xffffffffffffffffull));
}

TEST(stdbit, stdc_bit_ceil_forwarding) {
  EXPECT_EQ(0x0000000000000080ull, stdc_bit_ceil_uc(0x800000008000807full));
  EXPECT_EQ(0x0000000000008000ull, stdc_bit_ceil_us(0x8000000080007fffull));
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_ceil_ui(0x800000007fffffffull));
#if defined(__LP64__)
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_ceil_ul(0x7fffffffffffffffull));
#else
  EXPECT_EQ(0x0000000080000000ull, stdc_bit_ceil_ul(0x800000007fffffffull));
#endif
  EXPECT_EQ(0x8000000000000000ull, stdc_bit_ceil_ull(0x7fffffffffffffffull));
}

TEST(stdbit, no_double_evaluation) {
  ull x;

  x = 0;
  stdc_leading_zeros(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_leading_ones(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_trailing_zeros(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_trailing_ones(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_first_leading_zero(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_first_leading_one(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_first_trailing_zero(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_first_trailing_one(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_count_zeros(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_count_ones(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_has_single_bit(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_bit_width(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_bit_floor(x++);
  EXPECT_EQ(x, 1u);

  x = 0;
  stdc_bit_ceil(x++);
  EXPECT_EQ(x, 1u);
}

#endif
