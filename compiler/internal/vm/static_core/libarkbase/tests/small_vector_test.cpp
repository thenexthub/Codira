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

#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/small_vector.h"
#include <gtest/gtest.h>

namespace ark::test {

// NOLINTBEGIN(readability-magic-numbers)

class SmallVectorTest : public ::testing::Test {
public:
    NO_MOVE_SEMANTIC(SmallVectorTest);
    NO_COPY_SEMANTIC(SmallVectorTest);

    SmallVectorTest()
    {
        ark::mem::MemConfig::Initialize(0U, 64_MB, 256_MB, 32_MB, 0U, 0U);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }

    ~SmallVectorTest() override
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

template <typename Vector>
void TestVectorGrow(Vector &vector)
{
    std::array values = {10U, 20U, 30U, 40U, 50U, 60U, 70U, 80U, 90U, 100U};
    ASSERT_EQ(vector.size(), 0U);
    ASSERT_EQ(vector.capacity(), 4U);

    vector.push_back(values[0U]);
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(vector.capacity(), 4U);
    ASSERT_TRUE(vector.IsStatic());

    vector.push_back(values[1U]);
    vector.push_back(values[2U]);
    vector.push_back(values[3U]);
    ASSERT_EQ(vector.size(), 4U);
    ASSERT_EQ(vector.capacity(), 4U);
    ASSERT_TRUE(vector.IsStatic());

    vector.push_back(values[4U]);
    ASSERT_EQ(vector.size(), 5U);
    ASSERT_GE(vector.capacity(), 5U);
    ASSERT_FALSE(vector.IsStatic());

    ASSERT_TRUE(std::equal(values.begin(), values.begin() + 5U, vector.begin()));

    std::copy(values.begin() + 5U, values.end(), std::back_inserter(vector));
    ASSERT_EQ(vector.size(), 10U);
    ASSERT_FALSE(vector.IsStatic());
    for (size_t i = 0; i < values.size(); i++) {
        ASSERT_EQ(vector[i], values[i]);
    }
}

TEST_F(SmallVectorTest, Growing)
{
    {
        SmallVector<int, 4U> vector;
        TestVectorGrow(vector);
    }
    {
        SmallVector<int, 4U, ArenaAllocator, true> vector(GetAllocator());
        TestVectorGrow(vector);
    }
}

template <typename Vector>
void CheckIteration(Vector &vector)
{
    auto it = std::find(vector.begin(), vector.end(), 30U);
    ASSERT_NE(it, vector.end());
    ASSERT_EQ(*it, 30U);
    ASSERT_EQ(std::distance(vector.begin(), it), 2U);
    it = std::find(vector.begin(), vector.end(), 50U);
    ASSERT_EQ(it, vector.end());
}

template <typename Vector>
void CheckReverseIteration(Vector &vector)
{
    auto it = std::find(vector.rbegin(), vector.rend(), 30U);
    ASSERT_NE(it, vector.rend());
    ASSERT_EQ(*it, 30U);
    ASSERT_EQ(std::distance(vector.rbegin(), it), 1U);
    it = std::find(vector.rbegin(), vector.rend(), 50U);
    ASSERT_EQ(it, vector.rend());
}

template <typename Vector>
void TestVectorIteration(Vector &vector)
{
    std::array values = {10U, 20U, 30U, 40U, 50U, 60U, 70U, 80U, 90U, 100U};
    ASSERT_EQ(vector.size(), 0U);
    std::copy(values.begin(), values.begin() + 4U, std::back_inserter(vector));
    ASSERT_TRUE(vector.IsStatic());
    ASSERT_EQ(vector.size(), 4U);
    ASSERT_TRUE(std::equal(vector.begin(), vector.end(), values.begin()));

    {
        CheckIteration(vector);
    }

    {
        CheckReverseIteration(vector);
    }

    {
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        const auto constVector = vector;
        ASSERT_TRUE(std::equal(constVector.begin(), constVector.end(), values.begin()));
    }

    std::copy(values.begin() + 4U, values.end(), std::back_inserter(vector));
    ASSERT_EQ(vector.size(), 10U);
    ASSERT_FALSE(vector.IsStatic());
    ASSERT_TRUE(std::equal(vector.begin(), vector.end(), values.begin()));

    {
        auto it = std::find(vector.crbegin(), vector.crend(), 30U);
        ASSERT_NE(it, vector.crend());
        ASSERT_EQ(*it, 30U);
        ASSERT_EQ(std::distance(vector.crbegin(), it), 7U);

        it = std::find(vector.crbegin(), vector.crend(), 190U);
        ASSERT_EQ(it, vector.crend());
    }

    {
        auto it = vector.begin();
        ASSERT_EQ(*(it + 3U), vector[3U]);
        std::advance(it, 8U);
        ASSERT_EQ(*it, vector[8U]);
        it -= 3U;
        ASSERT_EQ(*it, vector[5U]);
        ASSERT_EQ(*(it - 3L), vector[2U]);
        it++;
        ASSERT_EQ(*(it - 3L), vector[3U]);
        --it;
        ASSERT_EQ(*(it - 3L), vector[2U]);
        it--;
        ASSERT_EQ(*(it - 3L), vector[1U]);
    }
}

TEST_F(SmallVectorTest, Iteration)
{
    {
        SmallVector<int, 4U> vector;
        TestVectorIteration(vector);
    }
    {
        SmallVector<int, 4U, ArenaAllocator, true> vector(GetAllocator());
        TestVectorIteration(vector);
    }
}

struct Item {
    Item()
    {
        constructed_++;
    }
    Item(int aa, double bb) : a_(aa), b_(bb)
    {
        constructed_++;
    }
    virtual ~Item()
    {
        destroyed_++;
    }
    DEFAULT_COPY_SEMANTIC(Item);
    DEFAULT_MOVE_SEMANTIC(Item);

    bool operator==(const Item &rhs) const
    {
        return a_ == rhs.a_ && b_ == rhs.b_;
    }
    static void Reset()
    {
        constructed_ = 0;
        destroyed_ = 0;
    }

    static inline size_t constructed_ = 0;
    static inline size_t destroyed_ = 0;

private:
    int a_ {101U};
    double b_ {202U};
};

TEST_F(SmallVectorTest, Emplace)
{
    SmallVector<Item, 1U> vector;
    vector.emplace_back(1U, 1.1L);
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(vector[0U], Item(1U, 1.1L));
    ASSERT_TRUE(vector.IsStatic());
    vector.emplace_back(2U, 2.2L);
    ASSERT_FALSE(vector.IsStatic());
    ASSERT_EQ(vector[1U], Item(2U, 2.2L));
    vector.push_back(Item(3U, 3.3L));
    ASSERT_EQ(vector[2U], Item(3U, 3.3L));
}

TEST_F(SmallVectorTest, ResizeStatic)
{
    SmallVector<Item, 4U> vector;

    vector.push_back(Item(1U, 1.2L));
    ASSERT_EQ(vector[0U], Item(1U, 1.2L));
    Item::Reset();
    vector.resize(3U);
    ASSERT_EQ(Item::constructed_, 2U);
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_TRUE(vector.IsStatic());
    ASSERT_EQ(vector[0U], Item(1U, 1.2L));
    ASSERT_EQ(vector[1U], Item());
    ASSERT_EQ(vector[2U], Item());

    Item::Reset();
    vector.resize(1U);
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(Item::destroyed_, 2U);

    Item::Reset();
    vector.clear();
    ASSERT_EQ(Item::destroyed_, 1U);
    ASSERT_EQ(vector.size(), 0U);
}

TEST_F(SmallVectorTest, ResizeDynamic)
{
    std::array values = {Item(1U, 1.2L), Item(2U, 2.3L), Item(3U, 3.4L)};
    SmallVector<Item, 2U> vector;

    Item::Reset();
    vector.resize(6U);
    ASSERT_EQ(Item::constructed_, 6U);
    ASSERT_FALSE(vector.IsStatic());
    ASSERT_EQ(vector.size(), 6U);
    ASSERT_TRUE(std::all_of(vector.begin(), vector.end(), [](const auto &v) { return v == Item(); }));

    Item::Reset();
    vector.resize(3U);
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_EQ(Item::destroyed_, 3U);
    ASSERT_FALSE(vector.IsStatic());

    Item::Reset();
    vector.clear();
    ASSERT_EQ(Item::destroyed_, 3U);
    ASSERT_EQ(vector.size(), 0U);
    ASSERT_FALSE(vector.IsStatic());
}

TEST_F(SmallVectorTest, ResizeStaticWithValue)
{
    SmallVector<Item, 4U> vector;

    vector.push_back(Item(1U, 1.2L));
    ASSERT_EQ(vector[0U], Item(1U, 1.2L));
    Item::Reset();
    vector.resize(3U, Item(3U, 3.3L));
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_TRUE(vector.IsStatic());
    ASSERT_EQ(vector[0U], Item(1U, 1.2L));
    ASSERT_EQ(vector[1U], Item(3U, 3.3L));
    ASSERT_EQ(vector[2U], Item(3U, 3.3L));

    Item item(3U, 3.3L);
    Item::Reset();
    vector.resize(1U, item);
    ASSERT_EQ(vector.size(), 1U);
    ASSERT_EQ(Item::destroyed_, 2U);

    Item::Reset();
    vector.clear();
    ASSERT_EQ(Item::destroyed_, 1U);
    ASSERT_EQ(vector.size(), 0U);
}

TEST_F(SmallVectorTest, ResizeDynamicWithValue)
{
    std::array values = {Item(1U, 1.2L), Item(2U, 2.3L), Item(3U, 3.4L)};
    SmallVector<Item, 2U> vector;

    Item::Reset();
    vector.resize(6U, Item(3U, 3.3L));
    ASSERT_FALSE(vector.IsStatic());
    ASSERT_EQ(vector.size(), 6U);
    ASSERT_TRUE(std::all_of(vector.begin(), vector.end(), [](const auto &v) { return v == Item(3U, 3.3L); }));

    Item item(3U, 3.3L);
    Item::Reset();
    vector.resize(3U, item);
    ASSERT_EQ(vector.size(), 3U);
    ASSERT_EQ(Item::destroyed_, 3U);
    ASSERT_FALSE(vector.IsStatic());

    Item::Reset();
    vector.clear();
    ASSERT_EQ(Item::destroyed_, 3U);
    ASSERT_EQ(vector.size(), 0U);
    ASSERT_FALSE(vector.IsStatic());
}

TEST_F(SmallVectorTest, ConstructingStaticToDynamic)
{
    std::array values = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};

    // Assign from static vector to dynamic
    {
        SmallVector<int, 2U> vector1;
        SmallVector<int, 2U> vector2;
        std::copy(values.begin(), values.end(), std::back_inserter(vector1));
        vector2.push_back(values[0U]);
        vector2.push_back(values[1U]);

        vector1 = vector2;
        ASSERT_EQ(vector1.size(), 2U);
        ASSERT_TRUE(vector1.IsStatic());
        ASSERT_TRUE(std::equal(vector1.begin(), vector1.end(), vector2.begin()));
        vector1.push_back(values[2U]);
        ASSERT_FALSE(vector1.IsStatic());
    }

    // Move assign from static vector to dynamic
    {
        SmallVector<int, 2U> vector1;
        SmallVector<int, 2U> vector2;
        std::copy(values.begin(), values.end(), std::back_inserter(vector1));
        vector2.push_back(values[0U]);
        vector2.push_back(values[1U]);

        vector1 = std::move(vector2);
        ASSERT_EQ(vector1.size(), 2U);
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        ASSERT_EQ(vector2.size(), 0U);
        ASSERT_TRUE(vector2.IsStatic());
        ASSERT_TRUE(vector1.IsStatic());
        ASSERT_TRUE(std::equal(vector1.begin(), vector1.end(), values.begin()));
    }

    // Copy constructor from static
    {
        SmallVector<int, 2U> vector1;
        std::copy(values.begin(), values.begin() + 2U, std::back_inserter(vector1));
        ASSERT_TRUE(vector1.IsStatic());
        SmallVector<int, 2U> vector2(vector1);
        ASSERT_EQ(vector1.size(), 2U);
        ASSERT_EQ(vector2.size(), 2U);
        ASSERT_TRUE(std::equal(vector2.begin(), vector2.end(), vector1.begin()));
    }

    // Move constructor from static
    {
        SmallVector<int, 2U> vector1;
        std::copy(values.begin(), values.begin() + 2U, std::back_inserter(vector1));
        ASSERT_TRUE(vector1.IsStatic());
        SmallVector<int, 2U> vector2(std::move(vector1));
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        ASSERT_EQ(vector1.size(), 0U);
        ASSERT_EQ(vector2.size(), 2U);
        ASSERT_TRUE(std::equal(vector2.begin(), vector2.end(), values.begin()));
    }
}

// NOLINTEND(readability-magic-numbers)

TEST_F(SmallVectorTest, ConstructingDynamicToStatic)
{
    std::array values = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};

    // Assign from dynamic vector to static
    {
        SmallVector<int, 2U> vector1;
        SmallVector<int, 2U> vector2;
        std::copy(values.begin(), values.end(), std::back_inserter(vector2));
        vector1.push_back(values[0U]);
        vector1.push_back(values[1U]);

        vector1 = vector2;
        ASSERT_EQ(vector1.size(), values.size());
        ASSERT_FALSE(vector1.IsStatic());
        ASSERT_TRUE(std::equal(vector1.begin(), vector1.end(), vector2.begin()));
    }

    // Move assign from dynamic vector to static
    {
        SmallVector<int, 2U> vector1;
        SmallVector<int, 2U> vector2;
        std::copy(values.begin(), values.end(), std::back_inserter(vector2));
        vector1.push_back(values[0U]);
        vector1.push_back(values[1U]);

        vector1 = std::move(vector2);
        ASSERT_EQ(vector1.size(), values.size());
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        ASSERT_EQ(vector2.size(), 0U);
        ASSERT_TRUE(vector2.IsStatic());
        ASSERT_FALSE(vector1.IsStatic());
        ASSERT_TRUE(std::equal(vector1.begin(), vector1.end(), values.begin()));
    }

    // Copy constructor from dynamic
    {
        SmallVector<int, 2U> vector1;
        std::copy(values.begin(), values.end(), std::back_inserter(vector1));
        ASSERT_FALSE(vector1.IsStatic());
        ASSERT_EQ(vector1.size(), values.size());
        SmallVector<int, 2U> vector2(vector1);
        ASSERT_EQ(vector1.size(), values.size());
        ASSERT_EQ(vector2.size(), values.size());
        ASSERT_TRUE(std::equal(vector2.begin(), vector2.end(), vector1.begin()));
    }

    // Move constructor from dynamic
    {
        SmallVector<int, 2U> vector1;
        std::copy(values.begin(), values.end(), std::back_inserter(vector1));
        ASSERT_FALSE(vector1.IsStatic());
        ASSERT_EQ(vector1.size(), values.size());
        SmallVector<int, 2U> vector2(std::move(vector1));
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        ASSERT_EQ(vector1.size(), 0U);
        ASSERT_EQ(vector2.size(), values.size());
        ASSERT_TRUE(std::equal(vector2.begin(), vector2.end(), values.begin()));
    }
}

}  // namespace ark::test
