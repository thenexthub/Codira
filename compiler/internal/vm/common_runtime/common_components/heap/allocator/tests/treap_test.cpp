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

#include "common_components/heap/allocator/treap.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/tests/test_helper.h"

using namespace common;

class TreapTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override
    {
        const size_t nUnit = 512;
        const size_t initialSize = 1024;

        const size_t unitInfoSize = sizeof(RegionDesc);
        const size_t heapSize = nUnit * RegionDesc::UNIT_SIZE;

        totalMemoryBuffer_ = reinterpret_cast<char*>(malloc(nUnit * unitInfoSize + heapSize));
        ASSERT_NE(totalMemoryBuffer_, nullptr);
        unitInfoBuffer_ = totalMemoryBuffer_;

        heapStartAddress_ = reinterpret_cast<uintptr_t>(totalMemoryBuffer_ + nUnit * unitInfoSize);

        RegionDesc::Initialize(nUnit, reinterpret_cast<uintptr_t>(unitInfoBuffer_),
                               heapStartAddress_);

        treap_.Init(initialSize);
    }

    void TearDown() override
    {
        treap_.Fini();
        free(totalMemoryBuffer_);
        totalMemoryBuffer_ = nullptr;
        heapStartAddress_ = 0;
        unitInfoBuffer_ = nullptr;
    }

    uintptr_t heapStartAddress_ = 0;
    char* unitInfoBuffer_ = nullptr;
    char* totalMemoryBuffer_ = nullptr;
    Treap treap_;
};


HWTEST_F_L0(TreapTest, InitAndEmpty) {
    EXPECT_TRUE(treap_.Empty());
    EXPECT_EQ(treap_.GetTotalCount(), 0u);
}


HWTEST_F_L0(TreapTest, InsertSingleNode) {
    bool result = treap_.MergeInsert(100, 10, true);
    EXPECT_TRUE(result);
    EXPECT_FALSE(treap_.Empty());
    EXPECT_EQ(treap_.GetTotalCount(), 10u);

    const Treap::TreapNode* root = treap_.RootNode();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetIndex(), 100u);
    EXPECT_EQ(root->GetCount(), 10u);
}


HWTEST_F_L0(TreapTest, MergeInsertAdjacentNodes) {
    treap_.MergeInsert(100, 10, true);

    bool result = treap_.MergeInsert(110, 5, true);
    EXPECT_TRUE(result);
    EXPECT_EQ(treap_.GetTotalCount(), 15u);

    const Treap::TreapNode* root = treap_.RootNode();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetIndex(), 100u);
    EXPECT_EQ(root->GetCount(), 15u);
}
 

HWTEST_F_L0(TreapTest, TakeUnitsFromTree) {
    treap_.MergeInsert(200, 20, true);

    uint32_t idx = 0;
    bool result = treap_.TakeUnits(5, idx, true);
    EXPECT_TRUE(result);
    EXPECT_EQ(idx, 200u);

    const Treap::TreapNode* root = treap_.RootNode();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetIndex(), 205u);
    EXPECT_EQ(root->GetCount(), 15u);
    EXPECT_EQ(treap_.GetTotalCount(), 15u);
}


HWTEST_F_L0(TreapTest, TakeUnitsFailOnSmallNode) {
    treap_.MergeInsert(300, 3, true);

    uint32_t idx = 0;
    bool result = treap_.TakeUnits(5, idx, true);
    EXPECT_FALSE(result);
}


HWTEST_F_L0(TreapTest, EmptyTreeTakeFails) {
    uint32_t idx = 0;
    bool result = treap_.TakeUnits(5, idx, true);
    EXPECT_FALSE(result);
}