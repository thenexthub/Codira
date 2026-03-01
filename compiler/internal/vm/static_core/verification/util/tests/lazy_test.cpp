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

#include "util/lazy.h"

#include <vector>
#include <unordered_set>

#include <gtest/gtest.h>

namespace ark::verifier::test {

TEST(Verifier, Lazy)
{
    std::vector<int> testData {1, 2, 3, -1, -2, -3, 5};

    auto stream1 = LazyFetch(testData);
    auto calcFunc = [](int acc, int val) { return acc + val; };
    auto result1 = FoldLeft(stream1, -4, calcFunc);

    EXPECT_EQ(result1, 1);

    EXPECT_EQ(FoldLeft(ConstLazyFetch(testData), -3, calcFunc), 2);

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto result2 = FoldLeft(Transform(ConstLazyFetch(testData), [](int val) { return val * 10; }), -49, calcFunc);
    EXPECT_EQ(result2, 1);

    auto result3 = FoldLeft(Filter(ConstLazyFetch(testData), [](int val) { return val > 0; }), -1, calcFunc);
    EXPECT_EQ(result3, 10);

    auto result4 =
        FoldLeft(Enumerate(ConstLazyFetch(testData)), 0, [](int acc, auto val) { return acc + std::get<0>(val); });
    EXPECT_EQ(result4, 21);

    auto result5 = FoldLeft(IndicesOf(testData), 0, calcFunc);
    EXPECT_EQ(result5, 21);

    int sum = 0;
    ForEach(ConstLazyFetch(testData), [&sum](int val) { sum += val; });
    EXPECT_EQ(sum, 5);

    int num = 0;
    for (const auto val : Iterable(Filter(ConstLazyFetch(testData), [](int val) { return val > 0; }))) {
        EXPECT_TRUE(val > 0);
        ++num;
    }
    EXPECT_EQ(num, 4);

    auto result6 = FoldLeft(ConstLazyFetch(testData) + ConstLazyFetch(testData), 0, calcFunc);
    EXPECT_EQ(result6, 10);

    auto result7 = ContainerOf<std::set<int>>(ConstLazyFetch(testData));
    EXPECT_EQ(result7, (std::set<int> {1, 2, 3, -1, -2, -3, 5}));

    auto result8 = ContainerOf<std::vector<int>>(ConstLazyFetch(testData));
    EXPECT_EQ(result8, (std::vector<int> {1, 2, 3, -1, -2, -3, 5}));
}

}  // namespace ark::verifier::test
