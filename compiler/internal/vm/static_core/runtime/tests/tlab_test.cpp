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

#include <gtest/gtest.h>

#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "runtime/mem/tlab.h"

namespace ark::mem {

constexpr size_t TLAB_TEST_SIZE = 4_MB;

class TLABTest : public testing::Test {
public:
    TLABTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0x0BADDEAD;
#endif
        srand(seed_);
    }

    ~TLABTest() override
    {
        for (auto i : allocatedMemMmap_) {
            ark::os::mem::UnmapRaw(std::get<0>(i), std::get<1>(i));
        }
    }

    NO_COPY_SEMANTIC(TLABTest);
    NO_MOVE_SEMANTIC(TLABTest);

protected:
    TLAB *CreateNewTLAB()
    {
        void *mem = ark::os::mem::MapRWAnonymousRaw(TLAB_TEST_SIZE);
        ASAN_UNPOISON_MEMORY_REGION(mem, TLAB_TEST_SIZE);
        std::pair<void *, size_t> newPair {mem, TLAB_TEST_SIZE};
        allocatedMemMmap_.push_back(newPair);
        auto tlabBuffOffs = AlignUp(ToUintPtr(mem) + sizeof(mem::TLAB), DEFAULT_ALIGNMENT_IN_BYTES) - ToUintPtr(mem);
        auto newTlab = new (mem) TLAB(ToVoidPtr(ToUintPtr(mem) + tlabBuffOffs), TLAB_TEST_SIZE - tlabBuffOffs);
        return newTlab;
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    unsigned seed_ {};

private:
    std::vector<std::pair<void *, size_t>> allocatedMemMmap_;
};

TEST_F(TLABTest, AccessTest)
{
    static constexpr size_t ALLOC_SIZE = 512;
    static constexpr size_t ALLOC_COUNT = 500000;
    TLAB *tlab = CreateNewTLAB();
    ASSERT_TRUE(tlab != nullptr);
    // All accesses has been created according implementation as we want to create in JIT.
    bool overflow = false;
    auto freePointerAddr = static_cast<uintptr_t *>(ToVoidPtr(ToUintPtr(tlab) + TLAB::TLABFreePointerOffset()));
    auto endAddr = static_cast<uintptr_t *>(ToVoidPtr(ToUintPtr(tlab) + TLAB::TLABEndAddrOffset()));
    for (size_t i = 1; i < ALLOC_COUNT; i++) {
        uintptr_t oldFreePointer = (*freePointerAddr);
        // NOTE: All objects, allocated in Runtime, must have the DEFAULT_ALIGNMENT alignment.
        void *mem = tlab->Alloc(AlignUp(ALLOC_SIZE, DEFAULT_ALIGNMENT_IN_BYTES));
        if (mem != nullptr) {
            ASSERT_TRUE(ToUintPtr(mem) == oldFreePointer);
        } else {
            ASSERT_TRUE(*endAddr < (oldFreePointer + ALLOC_SIZE));
            overflow = true;
        }
    }
    ASSERT_EQ(overflow, true) << "Increase the size of alloc_count to get overflow";
}

TEST_F(TLABTest, AlignedAlloc)
{
    constexpr size_t ARRAY_SIZE = 1024;
    TLAB *tlab = CreateNewTLAB();
    Alignment align = DEFAULT_ALIGNMENT;
    std::array<int *, ARRAY_SIZE> arr {};

    size_t mask = GetAlignmentInBytes(align) - 1;

    // Allocations
    srand(seed_);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        arr[i] = static_cast<int *>(tlab->Alloc(sizeof(int)));
        // NOLINTNEXTLINE(cert-msc50-cpp)
        *arr[i] = rand() % std::numeric_limits<int>::max();
    }

    // Allocations checking
    srand(seed_);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        ASSERT_NE(arr[i], nullptr) << "value of i: " << i << ", align: " << align;
        ASSERT_EQ(reinterpret_cast<size_t>(arr[i]) & mask, static_cast<size_t>(0))
            << "value of i: " << i << ", align: " << align;
        // NOLINTNEXTLINE(cert-msc50-cpp)
        ASSERT_EQ(*arr[i], rand() % std::numeric_limits<int>::max()) << "value of i: " << i << ", align: " << align;
    }

    void *ptr = tlab->Alloc(TLAB_TEST_SIZE);
    ASSERT_EQ(ptr, nullptr) << "Here Alloc with allocation size = " << TLAB_TEST_SIZE << " bytes should return nullptr";
}

}  // namespace ark::mem
