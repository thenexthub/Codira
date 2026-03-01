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

#include "util/obj_pool.h"

#include <gtest/gtest.h>

#include <vector>

namespace ark::verifier::test {

namespace {

struct S {
    int a;
    int b;
};

template <typename I, typename C>
struct Pool : public ObjPool<S, std::vector, I, C> {
    Pool(I i, C c) : ObjPool<S, std::vector, I, C> {i, c} {}
};

}  // namespace

template <typename I, typename C>
static void VerifierTestObjPool1(Pool<I, C> &pool, int &result)
{
    {
        auto q = pool.New();
        auto p = pool.New();
        EXPECT_EQ(pool.Count(), 2U);
        EXPECT_EQ(pool.FreeCount(), 0U);
        EXPECT_EQ(pool.AccCount(), 2U);
        EXPECT_EQ(result, 1U);
    }

    EXPECT_EQ(pool.Count(), 2U);
    EXPECT_EQ(pool.FreeCount(), 2U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);

    {
        auto q = pool.New();
        auto w = pool.New();
        EXPECT_EQ(pool.Count(), 2U);
        EXPECT_EQ(pool.FreeCount(), 0U);
        EXPECT_EQ(pool.AccCount(), 2U);
        EXPECT_EQ(result, 1U);
    }

    {
        auto q = pool.New();
        auto w = pool.New();
        EXPECT_EQ(pool.Count(), 2U);
        EXPECT_EQ(pool.FreeCount(), 0U);
        EXPECT_EQ(pool.AccCount(), 2U);
        EXPECT_EQ(result, 1U);
        {
            auto p = pool.New();
            EXPECT_EQ(pool.Count(), 3U);
            EXPECT_EQ(pool.FreeCount(), 0U);
            EXPECT_EQ(pool.AccCount(), 3U);
            EXPECT_EQ(result, 3U);
        }
        EXPECT_EQ(pool.Count(), 3U);
        EXPECT_EQ(pool.FreeCount(), 1U);
        EXPECT_EQ(pool.AccCount(), 2U);
        EXPECT_EQ(result, 1U);
        {
            auto p = pool.New();
            EXPECT_EQ(pool.Count(), 3U);
            EXPECT_EQ(pool.FreeCount(), 0U);
            EXPECT_EQ(pool.AccCount(), 3U);
            EXPECT_EQ(result, 3U);
        }
    }
}

template <typename I, typename C>
static void VerifierTestObjPool2(Pool<I, C> &pool, int &result)
{
    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 3U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);

    {
        auto q = pool.New();
        auto w = pool.New();
        pool.ShrinkToFit();
        EXPECT_EQ(pool.Count(), 2U);
        EXPECT_EQ(pool.FreeCount(), 0U);
        EXPECT_EQ(pool.AccCount(), 2U);
        EXPECT_EQ(result, 1U);
    }

    EXPECT_EQ(pool.Count(), 2U);
    EXPECT_EQ(pool.FreeCount(), 2U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);
}

template <typename I, typename C>
static void VerifierTestObjPool3(Pool<I, C> &pool, int &result)
{
    auto q = pool.New();
    auto w = pool.New();
    auto p = pool.New();
    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 0U);
    EXPECT_EQ(pool.AccCount(), 3U);
    EXPECT_EQ(result, 3U);

    auto u {p};

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 0U);
    EXPECT_EQ(pool.AccCount(), 4U);
    EXPECT_EQ(result, 3U);

    auto e {std::move(p)};

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 0U);
    EXPECT_EQ(pool.AccCount(), 4U);
    EXPECT_EQ(result, 3U);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_FALSE(p);
    EXPECT_TRUE(u);

    p = e;

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 0U);
    EXPECT_EQ(pool.AccCount(), 5U);
    EXPECT_EQ(result, 3U);
    EXPECT_TRUE(p);

    q = e;

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 1U);
    EXPECT_EQ(pool.AccCount(), 5U);
    EXPECT_EQ(result, 3U);

    w = std::move(e);

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 2U);
    EXPECT_EQ(pool.AccCount(), 4U);
    EXPECT_EQ(result, 2U);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_FALSE(e);

    EXPECT_EQ((*w).a, 2U);
}

TEST(VerifierTest_ObjPool, Basic)
{
    int result = 0;
    auto &&h = [&result](S &s, size_t idx) {
        s.a = idx;
        result += idx;
    };
    Pool pool {h, [&result](S &s) { result -= s.a; }};
    VerifierTestObjPool1(pool, result);
    VerifierTestObjPool2(pool, result);
    VerifierTestObjPool3(pool, result);

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 3U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);

    pool.ShrinkToFit();
    EXPECT_EQ(pool.Count(), 0U);
    EXPECT_EQ(pool.FreeCount(), 0U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);

    {
        auto q = pool.New();
        auto w = pool.New();
        auto p = pool.New();
        q = std::move(w);
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        auto e = p;

        EXPECT_EQ(pool.Count(), 3U);
        EXPECT_EQ(pool.FreeCount(), 1U);
        EXPECT_EQ(pool.AccCount(), 3U);
        EXPECT_EQ(result, 3U);

        int sum = 0;
        int prod = 1;
        int num = 0;

        while (auto acc = pool.AllObjects()()) {
            const S &obj = *(*acc);
            sum += obj.a;
            prod *= obj.a;
            ++num;
        }

        EXPECT_EQ(sum, 3U);
        EXPECT_EQ(prod, 2U);
        EXPECT_EQ(num, 2U);
    }

    EXPECT_EQ(pool.Count(), 3U);
    EXPECT_EQ(pool.FreeCount(), 3U);
    EXPECT_EQ(pool.AccCount(), 0U);
    EXPECT_EQ(result, 0U);
}

}  // namespace ark::verifier::test
