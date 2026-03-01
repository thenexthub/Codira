/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Test call from asembly

#include <iostream>
#include <cstdint>
#include <cmath>
#include <cassert>

#include <gtest/gtest.h>

class AsmCaller : public ::testing::Test {};

// Debug print to stdout
#ifdef ENABLE_DEBUG_STDOUT_PRINT
#define STDOUT_PRINT
#endif  // ENABLE_DEBUG_STDOUT_PRINT

template <typename T>
static auto Abs(T param)
{
    return (param > 0) ? param : -param;
}

template <typename T>
static auto Not(T param)
{
    return -param - 1U;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EMITED_ONE_PARAM_INST_LIST(DEF) \
    DEF(mov, +);                        \
    DEF(neg, -);                        \
    DEF(abs, Abs);                      \
    DEF(not, Not)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EMITED_TWO_PARAM_INST_LIST(DEF) \
    DEF(add, +);                        \
    DEF(sub, -);                        \
    DEF(mul, *);                        \
    DEF(and, &);                        \
    DEF(or, |);                         \
    DEF(xor, ^)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define C_EXTERN_ONE_PARAM(opc, param)                       \
    extern "C" std::uint8_t test_##opc##_8(std::uint8_t);    \
    extern "C" std::uint16_t test_##opc##_16(std::uint16_t); \
    extern "C" std::uint32_t test_##opc##_32(std::uint32_t); \
    extern "C" std::uint64_t test_##opc##_64(std::uint64_t)

EMITED_ONE_PARAM_INST_LIST(C_EXTERN_ONE_PARAM);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define C_EXTERN_TWO_PARAM(opc, param)                                      \
    extern "C" std::uint8_t test_##opc##_8(std::uint8_t, std::uint8_t);     \
    extern "C" std::uint16_t test_##opc##_16(std::uint16_t, std::uint16_t); \
    extern "C" std::uint32_t test_##opc##_32(std::uint32_t, std::uint32_t); \
    extern "C" std::uint64_t test_##opc##_64(std::uint64_t, std::uint64_t)

EMITED_TWO_PARAM_INST_LIST(C_EXTERN_TWO_PARAM);

// CC-OFFNXT(G.FUN.01, huge_method) test, solid logic
TEST_F(AsmCaller, CallMath)
{
    // NOLINTBEGIN(readability-magic-numbers)
    std::int8_t ui8 = 1;
    std::int8_t ui82 = -12;
    std::int16_t ui16 = 6518;
    std::int16_t ui162 = 6;
    std::int32_t ui32 = 3;
    std::int32_t ui322 = 0xecf27abf;
    std::int64_t ui64 = 2423590325;
    std::int64_t ui642 = 8;
    // NOLINTEND(readability-magic-numbers)

#ifdef STDOUT_PRINT
#define CALL_ONE_PARAM_OUTPUT(opc, param)                                                   \
    std::cerr << "------------ OUTPUT FOR " << #opc << "------------\n";                    \
    std::cerr << "ui8 result:" << std::hex << static_cast<uint64_t>(test_##opc##_8(ui8))    \
              << "  input:" << static_cast<uint64_t>(ui8) << "\n";                          \
    std::cerr << "ui16 result:" << std::hex << static_cast<uint64_t>(test_##opc##_16(ui16)) \
              << "  input:" << static_cast<uint64_t>(ui16) << "\n";                         \
    std::cerr << "ui32 result:" << std::hex << static_cast<uint64_t>(test_##opc##_32(ui32)) \
              << "  input:" << static_cast<uint64_t>(ui32) << "\n";                         \
    std::cerr << "ui64 result:" << std::hex << static_cast<uint64_t>(test_##opc##_64(ui64)) \
              << "  input:" << static_cast<uint64_t>(ui64) << "\n"
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_ONE_PARAM_OUTPUT(opc, param)                                 \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                        \
    EXPECT_EQ(test_##opc##_8(ui8), static_cast<uint8_t>(param(ui8)));     \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                        \
    EXPECT_EQ(test_##opc##_16(ui16), static_cast<uint16_t>(param(ui16))); \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                        \
    EXPECT_EQ(test_##opc##_32(ui32), static_cast<uint32_t>(param(ui32))); \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                        \
    EXPECT_EQ(test_##opc##_64(ui64), static_cast<uint64_t>(param(ui64)))
#endif

    EMITED_ONE_PARAM_INST_LIST(CALL_ONE_PARAM_OUTPUT);

#ifdef STDOUT_PRINT
#define CALL_TWO_PARAM_OUTPUT(opc, param)                                                                            \
    std::cerr << "------------ OUTPUT FOR " << #opc << "------------\n";                                             \
    std::cerr << "ui8 result:" << std::hex << static_cast<uint64_t>(test_##opc##_8(ui8, ui8_2))                      \
              << "  input_1:" << static_cast<uint64_t>(ui8) << "  input_2:" << static_cast<uint64_t>(ui8_2) << "\n"; \
    std::cerr << "ui16 result:" << std::hex << static_cast<uint64_t>(test_##opc##_16(ui16, ui16_2))                  \
              << "  input:" << static_cast<uint64_t>(ui16) << "  input_2:" << static_cast<uint64_t>(ui16_2) << "\n"; \
    std::cerr << "ui32 result:" << std::hex << static_cast<uint64_t>(test_##opc##_32(ui32, ui32_2))                  \
              << "  input:" << static_cast<uint64_t>(ui32) << "  input_2:" << static_cast<uint64_t>(ui32_2) << "\n"; \
    std::cerr << "ui64 result:" << std::hex << static_cast<uint64_t>(test_##opc##_64(ui64, ui64_2))                  \
              << "  input:" << static_cast<uint64_t>(ui64) << "  input_2:" << static_cast<uint64_t>(ui64_2) << "\n"
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_TWO_PARAM_OUTPUT(opc, param)                                             \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                                    \
    EXPECT_EQ(test_##opc##_8(ui8, ui82), static_cast<uint8_t>(ui8 param ui82));       \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                                    \
    EXPECT_EQ(test_##opc##_16(ui16, ui162), static_cast<uint16_t>(ui16 param ui162)); \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                                    \
    EXPECT_EQ(test_##opc##_32(ui32, ui322), static_cast<uint32_t>(ui32 param ui322)); \
    /* CC-OFFNXT(G.PRE.10) local macro for test */                                    \
    EXPECT_EQ(test_##opc##_64(ui64, ui642), static_cast<uint64_t>(ui64 param ui642))
#endif
    EMITED_TWO_PARAM_INST_LIST(CALL_TWO_PARAM_OUTPUT);  // NOLINT(hicpp-signed-bitwise)
}
