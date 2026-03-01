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

#include <memory>
#include <vector>
#include <thread>

#include "libarkbase/utils/tsan_interface.h"
#include "gtest/gtest.h"
#include "bitmap_test_base.h"
#include "runtime/mem/gc/bitmap.h"

namespace ark::mem {

TEST_F(BitmapTest, Init)
{
    const size_t sz = 1_MB;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto bmPtr = std::make_unique<BitmapWordType[]>(sz >> MemBitmap<>::LOG_BITSPERWORD);
    MemBitmap<> bm(ToVoidPtr(HEAP_STARTING_ADDRESS), sz, bmPtr.get());
    EXPECT_EQ(bm.Size(), sz);
}

TEST_F(BitmapTest, ScanRange)
{
    auto heapBegin = HEAP_STARTING_ADDRESS;
    constexpr size_t HEAP_CAPACITY = 16_MB;
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    auto bmPtr =
        std::make_unique<BitmapWordType[]>((HEAP_CAPACITY >> Bitmap::LOG_BITSPERWORD) / DEFAULT_ALIGNMENT_IN_BYTES);
    // NOLINTEND(modernize-avoid-c-arrays)

    MemBitmap<DEFAULT_ALIGNMENT_IN_BYTES> bm(ToVoidPtr(heapBegin), HEAP_CAPACITY, bmPtr.get());

    constexpr size_t BIT_SET_RANGE_END = Bitmap::BITSPERWORD * 3U;

    for (size_t j = 0; j < BIT_SET_RANGE_END; ++j) {
        auto *obj = ToVoidPtr(heapBegin + j * DEFAULT_ALIGNMENT_IN_BYTES);
        if ((ToUintPtr(obj) & BitmapVerify::ADDRESS_MASK_TO_SET) != 0U) {
            bm.Set(obj);
        }
    }

    constexpr size_t BIT_VERIFY_RANGE_END = Bitmap::BITSPERWORD * 2;

    for (size_t i = 0; i < Bitmap::BITSPERWORD; ++i) {
        auto *start = ToVoidPtr(heapBegin + i * DEFAULT_ALIGNMENT_IN_BYTES);
        for (size_t j = 0; j < BIT_VERIFY_RANGE_END; ++j) {
            auto *end = ToVoidPtr(heapBegin + (i + j) * DEFAULT_ALIGNMENT_IN_BYTES);
            BitmapVerify(&bm, start, end);
        }
    }
}

TEST_F(BitmapTest, VisitorPageAlignment)
{
    RunTestCount<4_KB>();
}

TEST_F(BitmapTest, OrderPageAlignment)
{
    RunTestOrder<4_KB>();
}

// test check that IterateOverMarkedChunkInRange & AtomicTestAndSetBit works fine with TSAN from two threads
// concurrently
TEST_F(BitmapTest, TSANMultithreadingTest)
{
#ifdef PANDA_TSAN_ON
    const size_t heapCapacity = 1_MB;
    auto bmPtr = std::make_unique<BitmapWordType[]>(heapCapacity >> MemBitmap<>::LOG_BITSPERWORD);
    auto heapBegin = BitmapTest::HEAP_STARTING_ADDRESS;
    MemBitmap<> bm(ToVoidPtr(heapBegin), heapCapacity, bmPtr.get());

    std::srand(0xBADDEAD);
    size_t iterations;
#ifdef PANDA_NIGHTLY_TEST_ON
    iterations = 3000U;
#else
    iterations = 1000U;
#endif

    auto iterateThread = std::thread([&bm, &iterations] {
        // we do less iterations for IterateOverMarkedChunks
        for (size_t i = 0; i < iterations; i++) {
            bm.IterateOverMarkedChunks<true>([](const void *object) { ASSERT_NE(object, nullptr); });
        }
    });

    auto setThread = std::thread([&bm, &heapBegin, &iterations] {
        for (size_t i = 0; i < iterations * iterations; i++) {
            bool value = std::rand() % 2 == 1;
            size_t offset = FnRounddown(std::rand() % heapCapacity, 4_KB);

            if (value) {
                bm.AtomicTestAndSet(ToVoidPtr(heapBegin + offset));
            } else {
                bm.AtomicTestAndClear(ToVoidPtr(heapBegin + offset));
            }
        }
    });
    iterateThread.join();
    setThread.join();
#endif
}

}  // namespace ark::mem
