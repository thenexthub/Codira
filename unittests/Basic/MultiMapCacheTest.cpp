//===--- MultiMapCacheTest.cpp --------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/MultiMapCache.h"
#include "language/Basic/Range.h"
#include "gtest/gtest.h"
#include <random>

using namespace language;

TEST(MultiMapCache, powersTest) {
  std::function<bool(unsigned, std::vector<unsigned> &)> cacheCompute =
      [&](unsigned key, std::vector<unsigned> &outArray) {
        outArray.push_back(key);
        outArray.push_back(key * key);
        outArray.push_back(key * key * key);
        return true;
      };
  MultiMapCache<unsigned, unsigned> cache(cacheCompute);

  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.size(), 0u);
  for (unsigned index : range(1, 256)) {
    auto array = *cache.get(index);
    for (unsigned power : array) {
      EXPECT_EQ(power % index, 0u);
    }
  }
  EXPECT_FALSE(cache.empty());
  EXPECT_EQ(cache.size(), 255u);
  for (unsigned index : range(1, 256)) {
    auto array = *cache.get(index);
    for (unsigned power : array) {
      EXPECT_EQ(power % index, 0u);
    }
  }
  EXPECT_FALSE(cache.empty());
  EXPECT_EQ(cache.size(), 255u);

  cache.clear();
  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.size(), 0u);
}

TEST(MultiMapCache, smallTest) {
  std::function<bool(unsigned, SmallVectorImpl<unsigned> &)> cacheCompute =
      [&](unsigned key, SmallVectorImpl<unsigned> &outArray) {
        outArray.push_back(key);
        outArray.push_back(key * key);
        outArray.push_back(key * key * key);
        return true;
      };
  SmallMultiMapCache<unsigned, unsigned> cache(cacheCompute);

  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.size(), 0u);
  for (unsigned index : range(1, 256)) {
    auto array = *cache.get(index);
    for (unsigned power : array) {
      EXPECT_EQ(power % index, 0u);
    }
  }
  EXPECT_FALSE(cache.empty());
  EXPECT_EQ(cache.size(), 255u);
  for (unsigned index : range(1, 256)) {
    auto array = *cache.get(index);
    for (unsigned power : array) {
      EXPECT_EQ(power % index, 0u);
    }
  }
  EXPECT_FALSE(cache.empty());
  EXPECT_EQ(cache.size(), 255u);

  cache.clear();
  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.size(), 0u);
}
