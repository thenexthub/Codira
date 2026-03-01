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

#include "utils/bit_vector.h"
#include "mem/pool_manager.h"
#include "utils/arena_containers.h"

#include <gtest/gtest.h>

namespace panda::test {

class BitVectorTest : public ::testing::Test {
public:
    BitVectorTest()
    {
        panda::mem::MemConfig::Initialize(0, 64_MB, 256_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    virtual ~BitVectorTest()
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
};

HWTEST_F(BitVectorTest, Basics, testing::ext::TestSize.Level0)
{
    BitVector<> vector;
    const BitVector<> &cvector = vector;
    ASSERT_EQ(vector.capacity(), 0U);

    // Index iterators for empty vector
    for (uint32_t i : cvector.GetSetBitsIndices()) {
        ADD_FAILURE() << i;
    }
    for (uint32_t i : cvector.GetZeroBitsIndices()) {
        ADD_FAILURE() << i;
    }

    vector.push_back(true);
    vector.push_back(false);
    ASSERT_NE(vector.capacity(), 0U);

    // Check `GetDataSpan`
    ASSERT_NE(vector.GetDataSpan().size(), 0U);
    uint32_t value = 1;
    ASSERT_EQ(std::memcmp(vector.GetDataSpan().data(), &value, 1), 0);

    // Constant operator[]
    ASSERT_EQ(cvector[0], vector[0]);

    // Constant versions of begin and end
    ASSERT_EQ(cvector.begin(), vector.begin());
    ASSERT_EQ(cvector.end(), vector.end());

    vector.resize(20);
    std::fill(vector.begin(), vector.end(), false);
    ASSERT_EQ(vector.PopCount(), 0U);
    std::fill(vector.begin() + 2, vector.begin() + 15, true);
    ASSERT_EQ(vector.PopCount(), 13U);
    for (size_t i = 0; i < 15; i++) {
        if (i > 2) {
            ASSERT_EQ(vector.PopCount(i), i - 2);
        } else {
            ASSERT_EQ(vector.PopCount(i), 0U);
        }
    }
    ASSERT_EQ(vector.GetHighestBitSet(), 14);
    ASSERT_EQ(vector[0], false);
    ASSERT_EQ(vector[1], false);
    ASSERT_EQ(vector[2], true);

    ASSERT_EQ(vector, vector.GetFixed());
    ASSERT_FALSE(vector.GetContainerDataSpan().empty());
}

HWTEST_F(BitVectorTest, Comparison, testing::ext::TestSize.Level0)
{
    std::vector<bool> values = {false, true, false, true, false, true, false, true, false, true};
    BitVector<> vec1;
    std::copy(values.begin(), values.end(), std::back_inserter(vec1));
    BitVector<ArenaAllocator> vec2(GetAllocator());
    std::copy(values.begin(), values.end(), std::back_inserter(vec2));
    ASSERT_EQ(vec1, vec2);
    vec2[0] = true;
    ASSERT_NE(vec1, vec2);
}

template <typename T>
void TestIteration1(T &vector, size_t bits)
{
    uint32_t index = 0;

    ASSERT_FALSE(vector.empty());
    ASSERT_EQ(vector.size(), bits);

    std::fill(vector.begin(), vector.end(), true);
    for (uint32_t i : vector.GetZeroBitsIndices()) {
        ADD_FAILURE() << i;
    }
    index = 0;
    for (uint32_t i : vector.GetSetBitsIndices()) {
        ASSERT_EQ(i, index++);
    }

    std::fill(vector.begin(), vector.end(), false);
    for (uint32_t i : vector.GetSetBitsIndices()) {
        ADD_FAILURE() << i;
    }
    index = 0;
    for (uint32_t i : vector.GetZeroBitsIndices()) {
        ASSERT_EQ(i, index++);
    }

    index = 0;
    for (auto v : vector) {
        v = (index++ % 2) != 0;
    }
    index = 0;
    for (auto v : vector) {
        ASSERT_EQ(v, index++ % 2);
    }
    index = vector.size() - 1;
    for (auto it = vector.end() - 1;; --it) {
        ASSERT_EQ(*it, index-- % 2);
        if (it == vector.begin()) {
            break;
        }
    }
    index = 1;
    for (uint32_t i : vector.GetSetBitsIndices()) {
        ASSERT_EQ(i, index);
        index += 2;
    }
    index = 0;
    for (uint32_t i : vector.GetZeroBitsIndices()) {
        ASSERT_EQ(i, index);
        index += 2;
    }
}

template <typename T>
void TestIteration2(T &vector, size_t bits)
{
    auto it = vector.begin();
    ASSERT_EQ(*it, false);
    ++it;
    ASSERT_EQ(*it, true);
    auto it1 = it++;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(*it1, true);
    ASSERT_TRUE(it1 < it);
    it += 3;
    ASSERT_EQ(*it, true);
    it -= 5;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(it, vector.begin());

    it = it + 6;
    ASSERT_EQ(*it, false);
    ASSERT_EQ(std::distance(vector.begin(), it), 6);
    ASSERT_EQ(it[1], true);
    it = it - 3;
    ASSERT_EQ(*it, true);
    ASSERT_EQ(std::distance(vector.begin(), it), 3);
    --it;
    ASSERT_EQ(*it, false);
    it1 = it--;
    ASSERT_EQ(*it, true);
    ASSERT_EQ(*it1, false);
    ASSERT_TRUE(it1 > it);
    it = vector.begin() + 100;
    ASSERT_EQ(std::distance(vector.begin(), it), 100);
    ASSERT_TRUE(it + 2 > it);
    ASSERT_TRUE(it + 2 >= it);
    ASSERT_TRUE(it + 0 >= it);
    ASSERT_TRUE(it - 2 < it);
    ASSERT_TRUE(it - 2 <= it);

    auto cit = vector.cbegin();
    ASSERT_EQ(cit, vector.begin());
    ASSERT_EQ(++cit, ++vector.begin());
    ASSERT_EQ(vector.cend(), vector.end());
}

HWTEST_F(BitVectorTest, Iteration, testing::ext::TestSize.Level0)
{
    std::array<uint32_t, 10> data {};
    size_t bits_num = data.size() * BitsNumInValue(data[0]);

    BitVector<> vec1;
    vec1.resize(bits_num);
    TestIteration1(vec1, bits_num);
    TestIteration2(vec1, bits_num);

    BitVector<ArenaAllocator> vec2(GetAllocator());
    vec2.resize(bits_num);
    TestIteration1(vec2, bits_num);
    TestIteration2(vec2, bits_num);

    BitVector<ArenaAllocator> vec3(bits_num, GetAllocator());
    TestIteration1(vec3, bits_num);
    TestIteration2(vec2, bits_num);

    BitVectorSpan vec4(Span<uint32_t>(data.data(), data.size()));
    TestIteration1(vec4, bits_num);
    TestIteration2(vec4, bits_num);

    data.fill(0);
    BitVectorSpan vec5(data.data(), bits_num);
    TestIteration1(vec5, bits_num);
    TestIteration2(vec5, bits_num);
}

template <typename T>
void TestModification(T &vector)
{
    std::vector<bool> values = {false, true, false, true, false, true, false, true, false, true};
    ASSERT_TRUE(vector.empty());
    ASSERT_EQ(vector.size(), 0U);
    ASSERT_EQ(vector.PopCount(), 0U);
    ASSERT_EQ(vector.GetHighestBitSet(), -1);

    vector.push_back(true);
    ASSERT_FALSE(vector.empty());
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(vector.PopCount(), 1U);
    ASSERT_EQ(vector.GetHighestBitSet(), 0);

    std::copy(values.begin(), values.end(), std::back_inserter(vector));
    ASSERT_EQ(vector.size(), 11U);
    ASSERT_EQ(vector[1], false);
    ASSERT_EQ(vector.PopCount(), 6U);
    ASSERT_EQ(vector.GetHighestBitSet(), 10);

    vector[1] = true;
    ASSERT_EQ(vector[1], true);

    uint32_t value = 0b10101010111;
    ASSERT_EQ(std::memcmp(vector.data(), &value, vector.GetSizeInBytes()), 0);

    vector.resize(3);
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_EQ(vector.PopCount(), 3U);

    vector.resize(10);
    ASSERT_EQ(vector.PopCount(), 3U);

    vector.clear();
    ASSERT_TRUE(vector.empty());
    ASSERT_EQ(vector.size(), 0U);

    // Push 1000 values with `true` in odd and `false` in even indexes
    for (int i = 0; i < 100; i++) {
        std::copy(values.begin(), values.end(), std::back_inserter(vector));
    }
    ASSERT_EQ(vector.size(), 1000U);
    ASSERT_EQ(vector.PopCount(), 500U);
    for (int i = 0; i < 1000; i++) {
        vector.push_back(false);
    }
    ASSERT_EQ(vector.size(), 2000U);
    ASSERT_EQ(vector.PopCount(), 500U);
    ASSERT_EQ(vector.GetHighestBitSet(), 999);

    vector.ClearBit(3000);
    ASSERT_EQ(vector.size(), 3001U);
    ASSERT_EQ(vector.PopCount(), 500U);
    ASSERT_EQ(vector.GetHighestBitSet(), 999);

    vector.SetBit(4000);
    ASSERT_EQ(vector.size(), 4001U);
    ASSERT_EQ(vector.PopCount(), 501U);
    ASSERT_EQ(vector.GetHighestBitSet(), 4000);
}

HWTEST_F(BitVectorTest, Modification, testing::ext::TestSize.Level0)
{
    BitVector<> vec1;
    TestModification(vec1);
    BitVector<ArenaAllocator> vec2(GetAllocator());
    TestModification(vec2);
}

HWTEST_F(BitVectorTest, SetClearBit, testing::ext::TestSize.Level0)
{
    BitVector<> vector;

    vector.SetBit(55);
    ASSERT_EQ(vector.size(), 56U);

    vector.SetBit(45);
    ASSERT_EQ(vector.size(), 56U);
    ASSERT_EQ(vector.PopCount(), 2U);

    vector.ClearBit(105);
    ASSERT_EQ(vector.size(), 106U);
    ASSERT_EQ(vector.PopCount(), 2U);
    ASSERT_EQ(vector.GetHighestBitSet(), 55);

    vector.ClearBit(45);
    ASSERT_EQ(vector.size(), 106U);
    ASSERT_EQ(vector.PopCount(), 1U);
}

}  // namespace panda::test
