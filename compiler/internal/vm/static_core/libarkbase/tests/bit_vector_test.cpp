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

#include "libarkbase/utils/bit_vector.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/arena_containers.h"

#include <gtest/gtest.h>

namespace ark::test {

// NOLINTBEGIN(readability-magic-numbers)

class BitVectorTest : public ::testing::Test {
public:
    NO_MOVE_SEMANTIC(BitVectorTest);
    NO_COPY_SEMANTIC(BitVectorTest);

    BitVectorTest()
    {
        ark::mem::MemConfig::Initialize(0, 64_MB, 256_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~BitVectorTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

TEST_F(BitVectorTest, Basics)
{
    BitVector<> vector;
    const BitVector<> &cvector = vector;
    ASSERT_EQ(vector.capacity(), 0U);

    // Index iterators for empty vector
    for (uint32_t i : cvector.GetSetBitsIndices()) {
        (void)i;
        ADD_FAILURE();
    }
    for (uint32_t i : cvector.GetZeroBitsIndices()) {
        (void)i;
        ADD_FAILURE();
    }

    vector.push_back(true);
    vector.push_back(false);
    ASSERT_NE(vector.capacity(), 0U);

    // Check `GetDataSpan`
    ASSERT_NE(vector.GetDataSpan().size(), 0U);
    uint32_t value = 1;
    ASSERT_EQ(std::memcmp(vector.GetDataSpan().data(), &value, 1U), 0U);

    // Constant operator[]
    ASSERT_EQ(cvector[0U], vector[0U]);

    // Constant versions of begin and end
    ASSERT_EQ(cvector.begin(), vector.begin());
    ASSERT_EQ(cvector.end(), vector.end());

    vector.resize(20U);
    std::fill(vector.begin(), vector.end(), false);
    ASSERT_EQ(vector.PopCount(), 0U);
    std::fill(vector.begin() + 2U, vector.begin() + 15U, true);
    ASSERT_EQ(vector.PopCount(), 13U);
    for (size_t i = 0; i < 15U; i++) {
        if (i > 2U) {
            ASSERT_EQ(vector.PopCount(i), i - 2L);
        } else {
            ASSERT_EQ(vector.PopCount(i), 0U);
        }
    }
    ASSERT_EQ(vector.GetHighestBitSet(), 14U);
    ASSERT_EQ(vector[0U], false);
    ASSERT_EQ(vector[1U], false);
    ASSERT_EQ(vector[2U], true);

    ASSERT_EQ(vector, vector.GetFixed());
    ASSERT_FALSE(vector.GetContainerDataSpan().empty());
}

TEST_F(BitVectorTest, Comparison)
{
    std::vector<bool> values = {false, true, false, true, false, true, false, true, false, true};
    BitVector<> vec1;
    std::copy(values.begin(), values.end(), std::back_inserter(vec1));
    BitVector<ArenaAllocator> vec2(GetAllocator());
    std::copy(values.begin(), values.end(), std::back_inserter(vec2));
    ASSERT_EQ(vec1, vec2);
    vec2[0U] = true;
    ASSERT_NE(vec1, vec2);
}

template <typename T>
void CheckIterator(T &vector)
{
    auto it = vector.begin();
    ASSERT_EQ(*it, false);
    ++it;
    ASSERT_EQ(*it, true);
    auto it1 = it++;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(*it1, true);
    ASSERT_TRUE(it1 < it);
    it += 3U;
    ASSERT_EQ(*it, true);
    it -= 5U;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(it, vector.begin());

    it = it + 6U;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(std::distance(vector.begin(), it), 6U);
    ASSERT_EQ(it[1U], true);
    it = it - 3L;
    ASSERT_EQ(*it, true);
    ASSERT_EQ(std::distance(vector.begin(), it), 3U);
    --it;
    ASSERT_EQ(*it, false);
    it1 = it--;
    ASSERT_EQ(*it, true);
    ASSERT_EQ(*it1, false);
    ASSERT_TRUE(it1 > it);
    it = vector.begin() + 100U;
    ASSERT_EQ(std::distance(vector.begin(), it), 100U);
    ASSERT_TRUE(it + 2U > it);
    ASSERT_TRUE(it + 2U >= it);
    ASSERT_TRUE(it + 0U >= it);
    ASSERT_TRUE(it - 2L < it);
    ASSERT_TRUE(it - 2L <= it);

    auto cit = vector.cbegin();
    ASSERT_EQ(cit, vector.begin());
    ASSERT_EQ(++cit, ++vector.begin());
    ASSERT_EQ(vector.cend(), vector.end());
}

template <typename T>
void TestIteration(T &vector, size_t bits)
{
    int index = 0;

    ASSERT_FALSE(vector.empty());
    ASSERT_EQ(vector.size(), bits);

    std::fill(vector.begin(), vector.end(), true);
    for ([[maybe_unused]] uint32_t i : vector.GetZeroBitsIndices()) {
        ADD_FAILURE();
    }
    index = 0;
    for (uint32_t i : vector.GetSetBitsIndices()) {
        ASSERT_EQ(i, index++);
    }

    std::fill(vector.begin(), vector.end(), false);
    for ([[maybe_unused]] uint32_t i : vector.GetSetBitsIndices()) {
        ADD_FAILURE();
    }
    index = 0;
    for (uint32_t i : vector.GetZeroBitsIndices()) {
        ASSERT_EQ(i, index++);
    }

    index = 0;
    for (auto v : vector) {
        v = (index++ % 2U) != 0;
    }
    index = 0;
    for (auto v : vector) {
        ASSERT_EQ(v, index++ % 2U);
    }
    index = vector.size() - 1U;
    for (auto it = vector.end() - 1U;; --it) {
        ASSERT_EQ(*it, index-- % 2U);
        if (it == vector.begin()) {
            break;
        }
    }
    index = 1;
    for (uint32_t i : vector.GetSetBitsIndices()) {
        ASSERT_EQ(i, index);
        index += 2U;
    }
    index = 0;
    for (uint32_t i : vector.GetZeroBitsIndices()) {
        ASSERT_EQ(i, index);
        index += 2U;
    }

    CheckIterator(vector);
}

TEST_F(BitVectorTest, Iteration)
{
    std::array<uint32_t, 10U> data {};
    size_t bitsNum = data.size() * BitsNumInValue(data[0U]);

    BitVector<> vec1;
    vec1.resize(bitsNum);
    TestIteration(vec1, bitsNum);

    BitVector<ArenaAllocator> vec2(GetAllocator());
    vec2.resize(bitsNum);
    TestIteration(vec2, bitsNum);

    BitVector<ArenaAllocator> vec3(bitsNum, GetAllocator());
    TestIteration(vec3, bitsNum);

    BitVectorSpan vec4(Span<uint32_t>(data.data(), data.size()));
    TestIteration(vec4, bitsNum);

    data.fill(0U);
    BitVectorSpan vec5(data.data(), bitsNum);
    TestIteration(vec5, bitsNum);
}

template <typename T>
void TestModification(T &vector)
{
    std::vector<bool> values = {false, true, false, true, false, true, false, true, false, true};
    ASSERT_TRUE(vector.empty());
    ASSERT_EQ(vector.size(), 0U);
    ASSERT_EQ(vector.PopCount(), 0U);
    ASSERT_EQ(vector.GetHighestBitSet(), -1L);

    vector.push_back(true);
    ASSERT_FALSE(vector.empty());
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(vector.PopCount(), 1U);
    ASSERT_EQ(vector.GetHighestBitSet(), 0U);

    std::copy(values.begin(), values.end(), std::back_inserter(vector));
    ASSERT_EQ(vector.size(), 11U);
    ASSERT_EQ(vector[1U], false);
    ASSERT_EQ(vector.PopCount(), 6U);
    ASSERT_EQ(vector.GetHighestBitSet(), 10U);

    vector[1U] = true;
    ASSERT_EQ(vector[1U], true);

    uint32_t value = 0b10101010111;
    ASSERT_EQ(std::memcmp(vector.data(), &value, vector.GetSizeInBytes()), 0U);

    vector.resize(3U);
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_EQ(vector.PopCount(), 3U);

    vector.resize(10U);
    ASSERT_EQ(vector.PopCount(), 3U);

    vector.clear();
    ASSERT_TRUE(vector.empty());
    ASSERT_EQ(vector.size(), 0U);

    // Push 1000 values with `true` in odd and `false` in even indexes
    for (size_t i = 0; i < 100U; i++) {
        std::copy(values.begin(), values.end(), std::back_inserter(vector));
    }
    ASSERT_EQ(vector.size(), 1000U);
    ASSERT_EQ(vector.PopCount(), 500U);
    for (size_t i = 0; i < 1000U; i++) {
        vector.push_back(false);
    }
    ASSERT_EQ(vector.size(), 2000U);
    ASSERT_EQ(vector.PopCount(), 500U);
    ASSERT_EQ(vector.GetHighestBitSet(), 999U);

    vector.ClearBit(3000U);
    ASSERT_EQ(vector.size(), 3001U);
    ASSERT_EQ(vector.PopCount(), 500U);
    ASSERT_EQ(vector.GetHighestBitSet(), 999U);

    vector.SetBit(4000U);
    ASSERT_EQ(vector.size(), 4001U);
    ASSERT_EQ(vector.PopCount(), 501U);
    ASSERT_EQ(vector.GetHighestBitSet(), 4000U);
}

TEST_F(BitVectorTest, Modification)
{
    BitVector<> vec1;
    TestModification(vec1);
    BitVector<ArenaAllocator> vec2(GetAllocator());
    TestModification(vec2);
}

TEST_F(BitVectorTest, SetClearBit)
{
    BitVector<> vector;

    vector.SetBit(55U);
    ASSERT_EQ(vector.size(), 56U);

    vector.SetBit(45U);
    ASSERT_EQ(vector.size(), 56U);
    ASSERT_EQ(vector.PopCount(), 2U);

    vector.ClearBit(105U);
    ASSERT_EQ(vector.size(), 106U);
    ASSERT_EQ(vector.PopCount(), 2U);
    ASSERT_EQ(vector.GetHighestBitSet(), 55U);

    vector.ClearBit(45U);
    ASSERT_EQ(vector.size(), 106U);
    ASSERT_EQ(vector.PopCount(), 1U);
}

TEST_F(BitVectorTest, TestUnion)
{
    {
        BitVector<> vector0;
        BitVector<> vector1;

        vector0 |= vector1;
        ASSERT_TRUE(vector0.empty());
    }

    {
        BitVector<> vector0;
        vector0.SetBit(13U);
        vector0.SetBit(71U);

        BitVector<> vector1;
        vector1.SetBit(10U);
        vector1.SetBit(13U);
        vector1.SetBit(42U);
        vector1.SetBit(77U);

        vector0 |= vector1;
        ASSERT_EQ(vector0.size(), 78U);
        ASSERT_EQ(vector0.PopCount(), 5U);
        ASSERT_TRUE(vector0.GetBit(10U));
        ASSERT_TRUE(vector0.GetBit(13U));
        ASSERT_TRUE(vector0.GetBit(42U));
        ASSERT_TRUE(vector0.GetBit(71U));
        ASSERT_TRUE(vector0.GetBit(77U));

        ASSERT_EQ(vector1.PopCount(), 4U);
    }

    {
        BitVector<> vector0;
        vector0.SetBit(1U);

        BitVector<> vector1;
        vector1.SetBit(128U);

        vector0 |= vector1;

        ASSERT_EQ(vector0.size(), 129U);
        ASSERT_EQ(vector0.PopCount(), 2U);
        ASSERT_TRUE(vector0.GetBit(1U));
        ASSERT_TRUE(vector0.GetBit(128U));
    }

    {
        BitVector<> vector0;
        vector0.SetBit(128U);

        BitVector<> vector1;
        vector1.SetBit(1U);

        vector0 |= vector1;

        ASSERT_EQ(vector0.size(), 129U);
        ASSERT_EQ(vector0.PopCount(), 2U);
        ASSERT_TRUE(vector0.GetBit(1U));
        ASSERT_TRUE(vector0.GetBit(128U));
    }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
