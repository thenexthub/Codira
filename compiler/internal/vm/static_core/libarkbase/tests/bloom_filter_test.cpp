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

#include "libarkbase/utils/bloom_filter.h"

#include "gtest/gtest.h"

namespace ark {

TEST(BloomFilterTest, InvalidConstruction)
{
    // n = 0
    EXPECT_DEATH({ BloomFilter bf(0, 0.01); }, ".*");

    // p <= 0
    EXPECT_DEATH({ BloomFilter bf(1000, 0.0); }, ".*");

    // p >= 1
    EXPECT_DEATH({ BloomFilter bf(1000, 1.0); }, ".*");
}

TEST(BloomFilterTest, InsertAndQuery)
{
    constexpr size_t n = 1000;
    constexpr double p = 0.01;

    BloomFilter bf(n, p);

    std::vector<std::string> keys = {"alpha", "beta", "gamma", "delta", "epsilon"};
    for (auto &key : keys) {
        bf.Add(reinterpret_cast<const uint8_t *>(key.c_str()));
    }

    for (auto &key : keys) {
        bool exist = bf.PossiblyContains(reinterpret_cast<const uint8_t *>(key.c_str()));
        EXPECT_TRUE(exist);
    }
}

static std::vector<std::string> BuildSampleKeys(size_t count, const std::string &prefix)
{
    std::vector<std::string> keys;
    keys.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keys.push_back(prefix + std::to_string(i));
    }

    return keys;
}

TEST(BloomFilterTest, FalsePositiveRate)
{
    constexpr size_t n = 50000;
    constexpr double p = 0.01;

    BloomFilter bf(n, p);

    std::vector<std::string> inserted = BuildSampleKeys(n, "exist_");
    for (auto &key : inserted) {
        bf.Add(reinterpret_cast<const uint8_t *>(key.c_str()));
    }

    std::vector<std::string> nonInserted = BuildSampleKeys(n, "miss_");

    size_t mustNonExist = 0;
    size_t mayExist = 0;

    for (auto &key : nonInserted) {
        bool exist = bf.PossiblyContains(reinterpret_cast<const uint8_t *>(key.c_str()));
        if (exist) {
            mayExist++;
        } else {
            mustNonExist++;
        }
    }

    EXPECT_GT(mustNonExist, 0u);

    constexpr double_t allowErrorPower = 10.0;
    EXPECT_LE(static_cast<double>(mayExist) / n, p * allowErrorPower);
}
}  // namespace ark