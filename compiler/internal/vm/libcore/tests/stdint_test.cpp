/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>

#include <gtest/gtest.h>

#include <limits>

TEST(stdint, fast_type_sizes) {
  EXPECT_EQ(1U, sizeof(int_fast8_t));
  EXPECT_EQ(8U, sizeof(int_fast64_t));
  EXPECT_EQ(1U, sizeof(uint_fast8_t));
  EXPECT_EQ(8U, sizeof(uint_fast64_t));
#if defined(__LP64__)
  EXPECT_EQ(8U, sizeof(int_fast16_t));
  EXPECT_EQ(8U, sizeof(int_fast32_t));
  EXPECT_EQ(8U, sizeof(uint_fast16_t));
  EXPECT_EQ(8U, sizeof(uint_fast32_t));
#else
  EXPECT_EQ(4U, sizeof(int_fast16_t));
  EXPECT_EQ(4U, sizeof(int_fast32_t));
  EXPECT_EQ(4U, sizeof(uint_fast16_t));
  EXPECT_EQ(4U, sizeof(uint_fast32_t));
#endif
}

TEST(stdint, least_type_sizes) {
  EXPECT_EQ(1U, sizeof(int_least8_t));
  EXPECT_EQ(1U, sizeof(uint_least8_t));
  EXPECT_EQ(2U, sizeof(int_least16_t));
  EXPECT_EQ(2U, sizeof(uint_least16_t));
  EXPECT_EQ(4U, sizeof(int_least32_t));
  EXPECT_EQ(4U, sizeof(uint_least32_t));
  EXPECT_EQ(8U, sizeof(int_least64_t));
  EXPECT_EQ(8U, sizeof(uint_least64_t));
}

TEST(stdint, max) {
  EXPECT_EQ(std::numeric_limits<int8_t>::max(), INT8_MAX);
  EXPECT_EQ(std::numeric_limits<uint8_t>::max(), UINT8_MAX);
  EXPECT_EQ(std::numeric_limits<int16_t>::max(), INT16_MAX);
  EXPECT_EQ(std::numeric_limits<uint16_t>::max(), UINT16_MAX);
  EXPECT_EQ(std::numeric_limits<int32_t>::max(), INT32_MAX);
  EXPECT_EQ(std::numeric_limits<uint32_t>::max(), UINT32_MAX);
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), INT64_MAX);
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), UINT64_MAX);

  EXPECT_EQ(std::numeric_limits<int_fast8_t>::max(), INT_FAST8_MAX);
  EXPECT_EQ(std::numeric_limits<uint_fast8_t>::max(), UINT_FAST8_MAX);
  EXPECT_EQ(std::numeric_limits<int_fast16_t>::max(), INT_FAST16_MAX);
  EXPECT_EQ(std::numeric_limits<uint_fast16_t>::max(), UINT_FAST16_MAX);
  EXPECT_EQ(std::numeric_limits<int_fast32_t>::max(), INT_FAST32_MAX);
  EXPECT_EQ(std::numeric_limits<uint_fast32_t>::max(), UINT_FAST32_MAX);
  EXPECT_EQ(std::numeric_limits<int_fast64_t>::max(), INT_FAST64_MAX);
  EXPECT_EQ(std::numeric_limits<uint_fast64_t>::max(), UINT_FAST64_MAX);

  EXPECT_EQ(std::numeric_limits<int_least8_t>::max(), INT_LEAST8_MAX);
  EXPECT_EQ(std::numeric_limits<uint_least8_t>::max(), UINT_LEAST8_MAX);
  EXPECT_EQ(std::numeric_limits<int_least16_t>::max(), INT_LEAST16_MAX);
  EXPECT_EQ(std::numeric_limits<uint_least16_t>::max(), UINT_LEAST16_MAX);
  EXPECT_EQ(std::numeric_limits<int_least32_t>::max(), INT_LEAST32_MAX);
  EXPECT_EQ(std::numeric_limits<uint_least32_t>::max(), UINT_LEAST32_MAX);
  EXPECT_EQ(std::numeric_limits<int_least64_t>::max(), INT_LEAST64_MAX);
  EXPECT_EQ(std::numeric_limits<uint_least64_t>::max(), UINT_LEAST64_MAX);

  EXPECT_EQ(std::numeric_limits<wchar_t>::max(), WCHAR_MAX);
  EXPECT_EQ(std::numeric_limits<wint_t>::max(), WINT_MAX);

  EXPECT_EQ(std::numeric_limits<intptr_t>::max(), INTPTR_MAX);
  EXPECT_EQ(std::numeric_limits<uintptr_t>::max(), UINTPTR_MAX);
  EXPECT_EQ(std::numeric_limits<intmax_t>::max(), INTMAX_MAX);
  EXPECT_EQ(std::numeric_limits<uintmax_t>::max(), UINTMAX_MAX);

  EXPECT_EQ(std::numeric_limits<ptrdiff_t>::max(), PTRDIFF_MAX);

  EXPECT_EQ(std::numeric_limits<size_t>::max(), SIZE_MAX);

  EXPECT_EQ(std::numeric_limits<sig_atomic_t>::max(), SIG_ATOMIC_MAX);
}

TEST(stdint, min) {
  EXPECT_EQ(std::numeric_limits<int8_t>::min(), INT8_MIN);
  EXPECT_EQ(std::numeric_limits<int16_t>::min(), INT16_MIN);
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), INT32_MIN);
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), INT64_MIN);

  EXPECT_EQ(std::numeric_limits<int_fast8_t>::min(), INT_FAST8_MIN);
  EXPECT_EQ(std::numeric_limits<int_fast16_t>::min(), INT_FAST16_MIN);
  EXPECT_EQ(std::numeric_limits<int_fast32_t>::min(), INT_FAST32_MIN);
  EXPECT_EQ(std::numeric_limits<int_fast64_t>::min(), INT_FAST64_MIN);

  EXPECT_EQ(std::numeric_limits<int_least8_t>::min(), INT_LEAST8_MIN);
  EXPECT_EQ(std::numeric_limits<int_least16_t>::min(), INT_LEAST16_MIN);
  EXPECT_EQ(std::numeric_limits<int_least32_t>::min(), INT_LEAST32_MIN);
  EXPECT_EQ(std::numeric_limits<int_least64_t>::min(), INT_LEAST64_MIN);

  EXPECT_EQ(std::numeric_limits<wchar_t>::min(), WCHAR_MIN);
  EXPECT_EQ(std::numeric_limits<wint_t>::min(), static_cast<uintmax_t>(WINT_MIN));

  EXPECT_EQ(std::numeric_limits<intptr_t>::min(), INTPTR_MIN);
  EXPECT_EQ(std::numeric_limits<intmax_t>::min(), INTMAX_MIN);

  EXPECT_EQ(std::numeric_limits<ptrdiff_t>::min(), PTRDIFF_MIN);

  EXPECT_EQ(std::numeric_limits<sig_atomic_t>::min(), SIG_ATOMIC_MIN);
}
template <typename T>
static inline int bitSize() {
  return sizeof(T) * 8;
}

TEST(stdint, widths) {
#if defined(__BIONIC__)
  EXPECT_EQ(bitSize<int8_t>(), INT8_WIDTH);
  EXPECT_EQ(bitSize<uint8_t>(), UINT8_WIDTH);
  EXPECT_EQ(bitSize<int16_t>(), INT16_WIDTH);
  EXPECT_EQ(bitSize<uint16_t>(), UINT16_WIDTH);
  EXPECT_EQ(bitSize<int32_t>(), INT32_WIDTH);
  EXPECT_EQ(bitSize<uint32_t>(), UINT32_WIDTH);
  EXPECT_EQ(bitSize<int64_t>(), INT64_WIDTH);
  EXPECT_EQ(bitSize<uint64_t>(), UINT64_WIDTH);

  EXPECT_EQ(bitSize<int_fast8_t>(), INT_FAST8_WIDTH);
  EXPECT_EQ(bitSize<uint_fast8_t>(), UINT_FAST8_WIDTH);
  EXPECT_EQ(bitSize<int_fast16_t>(), INT_FAST16_WIDTH);
  EXPECT_EQ(bitSize<uint_fast16_t>(), UINT_FAST16_WIDTH);
  EXPECT_EQ(bitSize<int_fast32_t>(), INT_FAST32_WIDTH);
  EXPECT_EQ(bitSize<uint_fast32_t>(), UINT_FAST32_WIDTH);
  EXPECT_EQ(bitSize<int_fast64_t>(), INT_FAST64_WIDTH);
  EXPECT_EQ(bitSize<uint_fast64_t>(), UINT_FAST64_WIDTH);

  EXPECT_EQ(bitSize<int_least8_t>(), INT_LEAST8_WIDTH);
  EXPECT_EQ(bitSize<uint_least8_t>(), UINT_LEAST8_WIDTH);
  EXPECT_EQ(bitSize<int_least16_t>(), INT_LEAST16_WIDTH);
  EXPECT_EQ(bitSize<uint_least16_t>(), UINT_LEAST16_WIDTH);
  EXPECT_EQ(bitSize<int_least32_t>(), INT_LEAST32_WIDTH);
  EXPECT_EQ(bitSize<uint_least32_t>(), UINT_LEAST32_WIDTH);
  EXPECT_EQ(bitSize<int_least64_t>(), INT_LEAST64_WIDTH);
  EXPECT_EQ(bitSize<uint_least64_t>(), UINT_LEAST64_WIDTH);

  EXPECT_EQ(bitSize<wchar_t>(), WCHAR_WIDTH);
  EXPECT_EQ(bitSize<wint_t>(), WINT_WIDTH);

  EXPECT_EQ(bitSize<intptr_t>(), INTPTR_WIDTH);
  EXPECT_EQ(bitSize<uintptr_t>(), UINTPTR_WIDTH);
  EXPECT_EQ(bitSize<intmax_t>(), INTMAX_WIDTH);
  EXPECT_EQ(bitSize<uintmax_t>() , UINTMAX_WIDTH);

  EXPECT_EQ(bitSize<ptrdiff_t>(), PTRDIFF_WIDTH);

  EXPECT_EQ(bitSize<size_t>(), SIZE_WIDTH);

  EXPECT_EQ(bitSize<sig_atomic_t>(), SIG_ATOMIC_WIDTH);
#endif
}
