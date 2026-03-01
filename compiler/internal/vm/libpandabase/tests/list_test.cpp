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

#include "gtest/gtest.h"
#include "utils/list.h"
#include <list>

using namespace panda;

using std::cerr;
using std::endl;

namespace globals {
constexpr const int DEFAULT_LIST_CAPACITY = 1000;
}

struct TestNode : public ListNode {
    TestNode() = default;
    TestNode(int v) : value(v) {}
    int value {0};
};

bool operator==(const TestNode &lhs, const TestNode &rhs)
{
    return lhs.value == rhs.value;
}

class ListTest : public ::testing::Test {
public:
    ListTest()
    {
        nodes_.reserve(globals::DEFAULT_LIST_CAPACITY);
    }
    virtual ~ListTest() = default;

    template <typename... Args>
    TestNode *NewNode(Args &&... args)
    {
        // Vector allocation is unacceptable
        ASSERT(nodes_.size() < nodes_.capacity());

        nodes_.emplace_back(std::forward<Args>(args)...);
        return &nodes_.back();
    }

    bool IsEqual(const List<TestNode> &list1, std::initializer_list<TestNode> list2) const
    {
        if (GetListSize(list1) != list2.size()) {
            return false;
        }
        return std::equal(list1.begin(), list1.end(), list2.begin());
    }

    size_t GetListSize(const List<TestNode> &list) const
    {
        size_t size = 0;
        for (auto it : list)
            size++;
        return size;
    }

private:
    std::vector<TestNode> nodes_;
};

TEST_F(ListTest, Common)
{
    TestNode *node;
    ListIterator<TestNode> it;
    List<TestNode> list;
    List<TestNode> list2;

    ASSERT_TRUE(list.Empty());

    node = NewNode(1);
    list.PushFront(*node);

    ASSERT_FALSE(list.Empty());
    ASSERT_EQ(node, &list.Front());
    ASSERT_EQ(node, &*list.begin());
    ASSERT_EQ(++list.begin(), list.end());

    ASSERT_TRUE(IsEqual(list, {1}));

    list.PushFront(*NewNode(2));

    ASSERT_TRUE(IsEqual(list, {2, 1}));

    list.PopFront();
    ASSERT_TRUE(IsEqual(list, {1}));

    list.InsertAfter(list.begin(), *NewNode(2));
    ASSERT_TRUE(IsEqual(list, {1, 2}));

    list.PushFront(*NewNode(0));
    ASSERT_TRUE(IsEqual(list, {0, 1, 2}));

    list.EraseAfter(list.begin() + 1);
    ASSERT_TRUE(IsEqual(list, {0, 1}));

    it = list.begin() + 1;
    for (int i = 0; i < 8; i++)
        it = list.InsertAfter(it, *NewNode(i + 2));
    ASSERT_TRUE(IsEqual(list, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

    list2.Splice(list2.before_begin(), list);
    ASSERT_TRUE(list.Empty());
    ASSERT_TRUE(IsEqual(list2, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

    list.Splice(list.before_begin(), list2, list2.before_begin() + 5, list2.end());
    ASSERT_TRUE(IsEqual(list, {5, 6, 7, 8, 9}));
    ASSERT_TRUE(IsEqual(list2, {0, 1, 2, 3, 4}));

    list.Splice(list.before_begin(), list2);
    ASSERT_TRUE(list2.Empty());
    ASSERT_TRUE(IsEqual(list, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

    list2.Splice(list2.before_begin(), list, list.begin() + 1, list.begin() + 5);
    ASSERT_TRUE(IsEqual(list, {0, 1, 5, 6, 7, 8, 9}));
    ASSERT_TRUE(IsEqual(list2, {2, 3, 4}));

    list2.Splice(list2.begin(), list, list.before_begin());
    ASSERT_TRUE(IsEqual(list, {1, 5, 6, 7, 8, 9}));
    ASSERT_TRUE(IsEqual(list2, {2, 0, 3, 4}));

    list.Remove(9);
    ASSERT_TRUE(IsEqual(list, {1, 5, 6, 7, 8}));

    list.EraseAfter(list.begin() + 1, list.begin() + 4);
    ASSERT_TRUE(IsEqual(list, {1, 5, 8}));
}

struct DTestNode : public DListNode {
    DTestNode() = default;
    DTestNode(int v) : value(v) {}
    int value {0};
};

class DListTest : public ::testing::Test {
public:
    DListTest()
    {
        nodes_.reserve(globals::DEFAULT_LIST_CAPACITY);
    }
    virtual ~DListTest() = default;

    template <typename... Args>
    DTestNode *NewNode(Args &&... args)
    {
        // Vector allocation is unacceptable
        ASSERT(nodes_.size() < nodes_.capacity());

        nodes_.emplace_back(std::forward<Args>(args)...);
        return &nodes_.back();
    }

    bool IsEqual(const DList &list1, std::list<DTestNode> list2) const
    {
        if (list1.size() != list2.size()) {
            return false;
        }

        auto it1 = list1.begin();
        auto it2 = list2.begin();
        for (; it1 != list1.end(); it1++, it2++) {
            auto *node = reinterpret_cast<const DTestNode *>(&(*it1));
            if (node->value != (*it2).value) {
                return false;
            }
        }
        it1 = list1.begin();
        it2 = list2.begin();
        for (; it2 != list2.end(); ++it1, ++it2) {
            auto *node = reinterpret_cast<const DTestNode *>(&(*it1));
            if (node->value != (*it2).value) {
                return false;
            }
        }

        auto it3 = list1.rbegin();
        auto it4 = list2.rbegin();
        for (; it3 != list1.rend(); it3++, it4++) {
            auto *node = reinterpret_cast<const DTestNode *>(&(*it3));
            if (node->value != (*it4).value) {
                return false;
            }
        }
        it3 = list1.rbegin();
        it4 = list2.rbegin();
        for (; it4 != list2.rend(); ++it3, ++it4) {
            auto *node = reinterpret_cast<const DTestNode *>(&(*it3));
            if (node->value != (*it4).value) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<DTestNode> nodes_;
};

TEST_F(DListTest, Common)
{
    DList list1;
    std::list<DTestNode> list2;
    for (uint32_t i = 0; i < 20; i++) {
        auto *node = NewNode(i);
        list1.push_back(node);
        list2.push_back(i);
    }
    ASSERT_TRUE(IsEqual(list1, list2));

    auto it1 = list1.begin();
    auto it2 = list2.begin();
    for (uint32_t i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            it1 = list1.erase(it1);
            it2 = list2.erase(it2);
        } else {
            it1++;
            it2++;
        }
    }
    ASSERT_TRUE(IsEqual(list1, list2));

    list1.clear();
    list2.clear();
    ASSERT_TRUE(IsEqual(list1, list2));

    for (uint32_t i = 30; i < 50; i++) {
        auto *node = NewNode(i);
        list1.insert(list1.begin(), node);
        list2.insert(list2.begin(), i);
    }
    ASSERT_TRUE(IsEqual(list1, list2));

    list1.remove_if([](DListNode *node) { return reinterpret_cast<const DTestNode *>(node)->value < 41; });
    list2.remove_if([](DTestNode &node) { return node.value < 41; });
    ASSERT_TRUE(IsEqual(list1, list2));
}
