/**
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

#ifndef PANDA_RUNTIME_TESTS_BITMAP_TEST_BASE_H
#define PANDA_RUNTIME_TESTS_BITMAP_TEST_BASE_H

#include <cstdlib>
#include <memory>
#include <vector>

#include "runtime/mem/gc/bitmap.h"

namespace ark::mem {

using BitmapWordType = ark::mem::Bitmap::BitmapWordType;

inline size_t FnRounddown(size_t val, size_t alignment)
{
    size_t mask = ~((static_cast<size_t>(1) * alignment) - 1);
    return val & mask;
}

class BitmapTest : public testing::Test {
public:
    static constexpr ObjectPointerType HEAP_STARTING_ADDRESS = static_cast<ObjectPointerType>(0x10000000);

    template <size_t K_ALIGNMENT, typename TestFn>
    static void RunTest(TestFn &&fn)
    {
        auto heapBegin = BitmapTest::HEAP_STARTING_ADDRESS;
        const size_t heapCapacity = 16_MB;
#ifdef PANDA_NIGHTLY_TEST_ON
        // NOLINTNEXTLINE(cert-msc51-cpp)
        std::srand(time(nullptr));
#else
        // NOLINTNEXTLINE(cert-msc51-cpp,readability-magic-numbers)
        std::srand(0x1234);
#endif
        constexpr int TEST_REPEAT = 1;
        for (int i = 0; i < TEST_REPEAT; ++i) {
            // NOLINTNEXTLINE(modernize-avoid-c-arrays)
            auto bmPtr = std::make_unique<BitmapWordType[]>((heapCapacity >> Bitmap::LOG_BITSPERWORD) / K_ALIGNMENT);
            MemBitmap<K_ALIGNMENT> bm(ToVoidPtr(heapBegin), heapCapacity, bmPtr.get());
            constexpr int NUM_BITS_TO_MODIFY = 1000;
            for (int j = 0; j < NUM_BITS_TO_MODIFY; ++j) {
                // NOLINTNEXTLINE(cert-msc50-cpp)
                size_t offset = FnRounddown(std::rand() % heapCapacity, K_ALIGNMENT);
                // NOLINTNEXTLINE(cert-msc50-cpp)
                bool set = std::rand() % 2 == 1;
                if (set) {
                    bm.Set(ToVoidPtr(heapBegin + offset));
                } else {
                    bm.Clear(ToVoidPtr(heapBegin + offset));
                }
            }
            constexpr int NUM_TEST_RANGES = 50;
            for (int j = 0; j < NUM_TEST_RANGES; ++j) {
                // NOLINTNEXTLINE(cert-msc50-cpp)
                const size_t offset = FnRounddown(std::rand() % heapCapacity, K_ALIGNMENT);
                const size_t remain = heapCapacity - offset;
                // NOLINTNEXTLINE(cert-msc50-cpp)
                const size_t end = offset + FnRounddown(std::rand() % (remain + 1), K_ALIGNMENT);
                size_t manual = 0;
                for (ObjectPointerType k = offset; k < end; k += K_ALIGNMENT) {
                    manual += bm.Test(ToVoidPtr(heapBegin + k)) ? 1U : 0U;
                }
                fn(&bm, heapBegin + offset, heapBegin + end, manual);
            }
        }
    }

    template <size_t K_ALIGNMENT>
    static void RunTestCount()
    {
        auto countTestFn = [](MemBitmap<K_ALIGNMENT> *bitmap, ObjectPointerType begin, ObjectPointerType end,
                              size_t manualCount) {
            size_t count = 0;
            auto countFn = [&count, begin, end]([[maybe_unused]] void *obj) {
                auto p = ToObjPtr(obj);
                if (p >= begin && p < end) {
                    count++;
                }
            };
            bitmap->IterateOverMarkedChunkInRange(ToVoidPtr(begin), ToVoidPtr(end), countFn);
            EXPECT_EQ(count, manualCount);
        };
        RunTest<K_ALIGNMENT>(countTestFn);
    }

    template <size_t K_ALIGNMENT>
    static void RunTestOrder()
    {
        auto orderTestFn = [](MemBitmap<K_ALIGNMENT> *bitmap, ObjectPointerType begin, ObjectPointerType end,
                              size_t manualCount) {
            void *lastPtr = nullptr;
            auto orderCheck = [&lastPtr](void *obj) {
                EXPECT_LT(lastPtr, obj);
                lastPtr = obj;
            };

            // Test complete walk.
            bitmap->IterateOverChunks(orderCheck);
            if (manualCount > 0) {
                EXPECT_NE(nullptr, lastPtr);
            }

            // Test range.
            lastPtr = nullptr;
            bitmap->IterateOverMarkedChunkInRange(ToVoidPtr(begin), ToVoidPtr(end), orderCheck);
            if (manualCount > 0) {
                EXPECT_NE(nullptr, lastPtr);
            }
        };
        RunTest<K_ALIGNMENT>(orderTestFn);
    }
};

class BitmapVerify {
public:
    using BitmapType = MemBitmap<DEFAULT_ALIGNMENT_IN_BYTES>;
    BitmapVerify(BitmapType *bitmapArg, void *beginArg, void *endArg)
        : bitmap_(bitmapArg), begin_(beginArg), end_(endArg)
    {
    }

    void operator()(void *obj)
    {
        EXPECT_TRUE(obj >= begin_);
        EXPECT_TRUE(obj <= end_);
        EXPECT_EQ(bitmap_->Test(obj), ((BitmapType::ToPointerType(obj) & ADDRESS_MASK_TO_SET) != 0));
    }

    static constexpr BitmapWordType ADDRESS_MASK_TO_SET = 0xF;

private:
    BitmapType *const bitmap_;
    void *begin_;
    void *end_;
};

TEST_F(BitmapTest, AtomicClearSetTest)
{
    void *object = ToVoidPtr(HEAP_STARTING_ADDRESS);
    const size_t sz = 1_MB;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto bmPtr = std::make_unique<BitmapWordType[]>(sz >> MemBitmap<>::LOG_BITSPERWORD);
    MemBitmap<> bm(ToVoidPtr(HEAP_STARTING_ADDRESS), sz, bmPtr.get());

    // Set bit
    ASSERT_TRUE(bm.Test(object) == bm.AtomicTestAndSet(object));
    ASSERT_TRUE(bm.Test(object));
    ASSERT_TRUE(bm.AtomicTest(object));

    // Clear bit
    ASSERT_TRUE(bm.Test(object) == bm.AtomicTestAndClear(object));
    ASSERT_TRUE(!bm.Test(object));
    ASSERT_TRUE(!bm.AtomicTest(object));
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_TESTS_BITMAP_TEST_BASE_H
