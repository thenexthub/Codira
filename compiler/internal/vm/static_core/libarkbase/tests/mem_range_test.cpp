/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <cstdlib>
#include <ctime>
#include <climits>
#include <random>
#include "gtest/gtest.h"

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_range.h"

namespace ark::test::mem_range {

// NOLINTBEGIN(readability-magic-numbers)

constexpr uintptr_t MAX_PTR = std::numeric_limits<uintptr_t>::max();

constexpr uint64_t NUM_RANDOM_TESTS = 100;
constexpr uint64_t NUM_ITER_PER_TEST = 1000;
constexpr uintptr_t RANDOM_AREA_SIZE = 100000;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
std::default_random_engine g_generator;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::uniform_int_distribution<uintptr_t> g_distribution(0U, MAX_PTR);

// support function to generate random uintptr_t
static uintptr_t RandomUintptr()
{
    return g_distribution(g_generator);
}

// function to generate MemRange randomly from given range
static ark::mem::MemRange RandomMemRange(uintptr_t minStart, uintptr_t maxEnd)
{
    ASSERT(maxEnd > minStart);

    uintptr_t rand1 = minStart + RandomUintptr() % (maxEnd - minStart + 1U);
    uintptr_t rand2 = minStart + RandomUintptr() % (maxEnd - minStart + 1U);
    if (rand1 < rand2) {
        return ark::mem::MemRange(rand1, rand2);
    }

    if (rand1 > rand2) {
        return ark::mem::MemRange(rand2, rand1);
    }

    if (rand1 > 0U) {
        return ark::mem::MemRange(rand1 - 1L, rand1);
    }

    return ark::mem::MemRange(rand1, rand1 + 1U);
}

// test constructor and simple methods
TEST(MemRangeTest, BasicTest)
{
    constexpr uintptr_t START = 10;
    constexpr uintptr_t END = 10000;
    constexpr uintptr_t LOWER_THAN_START = 0;
    constexpr uintptr_t HIGHER_THAN_END = 50000;

    auto memRange = ark::mem::MemRange(START, END);

    // test correct start and end addresses
    ASSERT_EQ(START, memRange.GetStartAddress());
    ASSERT_EQ(END, memRange.GetEndAddress());

    // test inner addresses
    ASSERT_TRUE(memRange.IsAddressInRange(START));
    ASSERT_TRUE(memRange.IsAddressInRange(END));
    ASSERT_TRUE(memRange.IsAddressInRange((START + END) / 2U));

    // test outer addresses
    ASSERT_FALSE(memRange.IsAddressInRange(LOWER_THAN_START));
    ASSERT_FALSE(memRange.IsAddressInRange(START - 1L));
    ASSERT_FALSE(memRange.IsAddressInRange(END + 1U));
    ASSERT_FALSE(memRange.IsAddressInRange(HIGHER_THAN_END));

    // test mem_range with the same start and end
    auto memRangeWithOneElement = ark::mem::MemRange(START, START);
}

// test constructor with incorrect args
TEST(MemRangeTest, AssertTest)
{
    constexpr uintptr_t MIN = 10000;
    constexpr uintptr_t MAX = 50000;

    ASSERT_DEBUG_DEATH(ark::mem::MemRange(MAX, MIN), "");
}

// test IsIntersect method
TEST(MemRangeTest, IntersectTest)
{
    constexpr uintptr_t START_1 = 10;
    constexpr uintptr_t END_1 = 100;
    constexpr uintptr_t START_2 = 101;
    constexpr uintptr_t END_2 = 200;
    constexpr uintptr_t START_3 = 50;
    constexpr uintptr_t END_3 = 500;
    constexpr uintptr_t START_4 = 500;
    constexpr uintptr_t END_4 = 600;
    constexpr uintptr_t START_5 = 10;
    constexpr uintptr_t END_5 = 100;

    auto memRange1 = ark::mem::MemRange(START_1, END_1);
    auto memRange2 = ark::mem::MemRange(START_2, END_2);
    auto memRange3 = ark::mem::MemRange(START_3, END_3);
    auto memRange4 = ark::mem::MemRange(START_4, END_4);
    auto memRange5 = ark::mem::MemRange(START_5, END_5);

    // ranges are not intersecting
    ASSERT_FALSE(memRange1.IsIntersect(memRange2));
    ASSERT_FALSE(memRange2.IsIntersect(memRange1));

    // ranges are partly intersecting
    ASSERT_TRUE(memRange1.IsIntersect(memRange3));
    ASSERT_TRUE(memRange3.IsIntersect(memRange1));

    // ranges are nested
    ASSERT_TRUE(memRange2.IsIntersect(memRange3));
    ASSERT_TRUE(memRange3.IsIntersect(memRange2));

    // ranges have common bound
    ASSERT_TRUE(memRange3.IsIntersect(memRange4));
    ASSERT_TRUE(memRange4.IsIntersect(memRange3));

    // ranges are equal
    ASSERT_TRUE(memRange1.IsIntersect(memRange5));

    // test self
    ASSERT_TRUE(memRange1.IsIntersect(memRange1));
}

static void RandomTestInBoundsR1LR2(ark::mem::MemRange &memRange1, ark::mem::MemRange &memRange2)
{
    for (uintptr_t i = memRange1.GetStartAddress(); i < MAX_PTR; i++) {
        if (i == memRange2.GetStartAddress()) {
            ASSERT_TRUE(memRange1.IsIntersect(memRange2));
            ASSERT_TRUE(memRange2.IsIntersect(memRange1));
            break;
        }
        if (i == memRange1.GetEndAddress()) {
            ASSERT_FALSE(memRange1.IsIntersect(memRange2));
            ASSERT_FALSE(memRange2.IsIntersect(memRange1));
            break;
        }
    }
}

static void RandomTestInBoundsR1GR2(ark::mem::MemRange &memRange1, ark::mem::MemRange &memRange2)
{
    for (uintptr_t i = memRange2.GetStartAddress(); i < MAX_PTR; i++) {
        if (i == memRange1.GetStartAddress()) {
            ASSERT_TRUE(memRange1.IsIntersect(memRange2));
            ASSERT_TRUE(memRange2.IsIntersect(memRange1));
            break;
        }
        if (i == memRange2.GetEndAddress()) {
            ASSERT_FALSE(memRange1.IsIntersect(memRange2));
            ASSERT_FALSE(memRange2.IsIntersect(memRange1));
            break;
        }
    }
}

// function to conduct num_iter random tests with addresses in given bounds
static void RandomTestInBounds(uintptr_t from, uintptr_t to, uint64_t numIter = NUM_ITER_PER_TEST)
{
    ASSERT(from < to);

    ark::mem::MemRange memRange1(0U, 1U);
    ark::mem::MemRange memRange2(0U, 1U);
    // check intersection via cycle
    for (uint64_t iter = 0; iter < numIter; iter++) {
        memRange1 = RandomMemRange(from, to);
        memRange2 = RandomMemRange(from, to);
        if (memRange1.GetStartAddress() < memRange2.GetStartAddress()) {
            RandomTestInBoundsR1LR2(memRange1, memRange2);
        } else if (memRange1.GetStartAddress() > memRange2.GetStartAddress()) {
            RandomTestInBoundsR1GR2(memRange1, memRange2);
        } else {
            // case with equal start addresses
            ASSERT_TRUE(memRange1.IsIntersect(memRange2));
            ASSERT_TRUE(memRange2.IsIntersect(memRange1));
        }
    }
}

// set of random tests with different address ranges
// no bug detected during a lot of tries with different parameters
TEST(MemRangeTest, RandomIntersectTest)
{
    unsigned int seed;
#ifdef PANDA_NIGHTLY_TEST_ON
    seed_ = std::time(NULL);
#else
    seed = 0xDEADBEEF;
#endif
    srand(seed);
    g_generator.seed(seed);

    // random tests in interesting ranges
    RandomTestInBounds(0U, RANDOM_AREA_SIZE);
    RandomTestInBounds(MAX_PTR - RANDOM_AREA_SIZE, MAX_PTR);

    // tests in random ranges
    uintptr_t position;
    for (uint64_t i = 0; i < NUM_RANDOM_TESTS; i++) {
        position = RandomUintptr();
        if (position > RANDOM_AREA_SIZE) {
            RandomTestInBounds(position - RANDOM_AREA_SIZE, position);
        } else {
            RandomTestInBounds(position, position + RANDOM_AREA_SIZE);
        }
    }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test::mem_range
