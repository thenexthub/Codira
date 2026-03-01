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

#include <cstdlib>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "bitmap_test_base.h"
#include "runtime/mem/gc/bitmap.h"

namespace ark::mem {

TEST_F(BitmapTest, ClearRange)
{
    auto heapBegin = HEAP_STARTING_ADDRESS;
    constexpr size_t HEAP_CAPACITY = 16_MB;
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    auto bmPtr =
        std::make_unique<BitmapWordType[]>((HEAP_CAPACITY >> Bitmap::LOG_BITSPERWORD) / DEFAULT_ALIGNMENT_IN_BYTES);
    // NOLINTEND(modernize-avoid-c-arrays)
    MemBitmap<DEFAULT_ALIGNMENT_IN_BYTES> bm(ToVoidPtr(heapBegin), HEAP_CAPACITY, bmPtr.get());

    using MemRangeTest = std::pair<ObjectPointerType, ObjectPointerType>;
    constexpr MemRangeTest FIRST_RANGE {0, 10_KB + DEFAULT_ALIGNMENT_IN_BYTES};
    constexpr MemRangeTest SECOND_RANGE {DEFAULT_ALIGNMENT_IN_BYTES, DEFAULT_ALIGNMENT_IN_BYTES};
    constexpr MemRangeTest THIRD_RANGE {DEFAULT_ALIGNMENT_IN_BYTES, 2 * DEFAULT_ALIGNMENT_IN_BYTES};
    constexpr MemRangeTest FOURTH_RANGE {DEFAULT_ALIGNMENT_IN_BYTES, 5 * DEFAULT_ALIGNMENT_IN_BYTES};
    constexpr MemRangeTest FIFTH_RANGE {1_KB + DEFAULT_ALIGNMENT_IN_BYTES, 2_KB + 5 * DEFAULT_ALIGNMENT_IN_BYTES};
    constexpr MemRangeTest SIXTH_RANGE {0, HEAP_CAPACITY};

    std::vector<MemRangeTest> ranges {FIRST_RANGE, SECOND_RANGE, THIRD_RANGE, FOURTH_RANGE, FIFTH_RANGE, SIXTH_RANGE};

    for (const auto &range : ranges) {
        bm.IterateOverChunks([&bm](void *mem) { bm.Set(mem); });
        bm.ClearRange(ToVoidPtr(heapBegin + range.first), ToVoidPtr(heapBegin + range.second));

        auto testTrueFn = [&bm](void *mem) { EXPECT_TRUE(bm.Test(mem)) << "address: " << mem << std::endl; };
        auto testFalseFn = [&bm](void *mem) { EXPECT_FALSE(bm.Test(mem)) << "address: " << mem << std::endl; };
        bm.IterateOverChunkInRange(ToVoidPtr(heapBegin), ToVoidPtr(heapBegin + range.first), testTrueFn);
        bm.IterateOverChunkInRange(ToVoidPtr(heapBegin + range.first), ToVoidPtr(heapBegin + range.second),
                                   testFalseFn);
        // for SIXTH_RANGE, range.second is not in the heap, so we skip this test
        if (range.second < bm.MemSizeInBytes()) {
            bm.IterateOverChunkInRange(ToVoidPtr(heapBegin + range.second), ToVoidPtr(heapBegin + bm.MemSizeInBytes()),
                                       testTrueFn);
        }
    }
}

}  // namespace ark::mem
