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

#include "util/set_operations.h"
#include "util/tests/environment.h"
#include "util/range.h"

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <initializer_list>
#include <utility>
#include <string>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark::verifier;

namespace ark::verifier::test {

namespace {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-err58-cpp)
const EnvOptions OPTIONS {"VERIFIER_TEST"};

using Interval = Range<size_t>;
using Intervals = std::initializer_list<Interval>;

void ClassifySize(size_t size, const Intervals &intervals)
{
    for (const auto &i : intervals) {
        RC_CLASSIFY(i.Contains(size), std::to_string(i));
    }
}

}  // namespace

using Set = std::set<int>;

// 1 set
RC_GTEST_PROP(OperationsOverSets0, ConversionToSet, (std::vector<int> && vec))
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    Intervals intervals = {{0, 10}, {11, 30}, {31, 10000}};

    auto stat = [&intervals](const std::initializer_list<size_t> &sizes) {
        if (OPTIONS.Get<bool>("verbose", false)) {
            for (size_t size : sizes) {
                ClassifySize(size, intervals);
            }
        }
    };
    stat({vec.size()});
    Set result = ToSet<Set>(vec);
    for (const auto &elt : vec) {
        RC_ASSERT(result.count(elt) > 0U);
    }
}

// 2 sets
RC_GTEST_PROP(OperationsOverSets1, Intersection, (Set && set1, Set &&set2))
{
    Set result;

    result = SetIntersection(set1, set2);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U && set2.count(elt) > 0U);
    }
    for (int elt : set1) {
        RC_ASSERT(result.count(elt) == 0U || set2.count(elt) > 0U);
    }
    for (int elt : set2) {
        RC_ASSERT(result.count(elt) == 0U || set1.count(elt) > 0U);
    }
}

RC_GTEST_PROP(OperationsOverSets1, Union, (Set && set1, Set &&set2))
{
    Set result;
    result = SetUnion(set1, set2);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U || set2.count(elt) > 0U);
    }
    for (int elt : set1) {
        RC_ASSERT(result.count(elt) > 0U);
    }
    for (int elt : set2) {
        RC_ASSERT(result.count(elt) > 0U);
    }
}
RC_GTEST_PROP(OperationsOverSets1, Difference, (Set && set1, Set &&set2))
{
    Set result;
    result = SetDifference(set1, set2);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U && set2.count(elt) == 0U);
    }
}

// 3 sets
RC_GTEST_PROP(OperationsOverSets2, Intersection, (Set && set1, Set &&set2, Set &&set3))
{
    Set result;

    result = SetIntersection(set1, set2, set3);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U && set2.count(elt) > 0U && set3.count(elt) > 0U);
    }
    for (int elt : set1) {
        RC_ASSERT(result.count(elt) == 0U || (set2.count(elt) > 0U && set3.count(elt) > 0U));
    }
    for (int elt : set2) {
        RC_ASSERT(result.count(elt) == 0U || (set1.count(elt) > 0U && set3.count(elt) > 0U));
    }
    for (int elt : set3) {
        RC_ASSERT(result.count(elt) == 0U || (set2.count(elt) > 0U && set1.count(elt) > 0U));
    }
}

RC_GTEST_PROP(OperationsOverSets2, Union, (Set && set1, Set &&set2, Set &&set3))
{
    Set result;

    result = SetUnion(set1, set2, set3);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U || set2.count(elt) > 0U || set3.count(elt) > 0U);
    }
    for (int elt : set1) {
        RC_ASSERT(result.count(elt) > 0U);
    }
    for (int elt : set2) {
        RC_ASSERT(result.count(elt) > 0U);
    }
    for (int elt : set3) {
        RC_ASSERT(result.count(elt) > 0U);
    }
}
RC_GTEST_PROP(OperationsOverSets2, Difference, (Set && set1, Set &&set2, Set &&set3))
{
    Set result;
    result = SetDifference(set1, set2, set3);
    for (int elt : result) {
        RC_ASSERT(set1.count(elt) > 0U && set2.count(elt) == 0U && set3.count(elt) == 0U);
    }
}

}  // namespace ark::verifier::test
